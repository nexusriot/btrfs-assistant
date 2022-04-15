#include "Snapper.h"
#include "Settings.h"
#include "System.h"

#include <unistd.h>

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QRegularExpression>

Snapper::Snapper(Btrfs *btrfs, QString snapperCommand, QObject *parent)
    : QObject{parent}, m_btrfs(btrfs), m_snapperCommand(snapperCommand) {
    m_subvolMap = Settings::getInstance().subvolMap();
    load();
}

const QMap<QString, QString> Snapper::config(const QString &name) {
    if (m_configs.contains(name)) {
        return m_configs[name];
    } else {
        return QMap<QString, QString>();
    }
}

void Snapper::createSubvolMap() {
    for (const QVector<SnapperSubvolume> &subvol : qAsConst(m_subvols)) {
        const QString snapshotSubvol = findSnapshotSubvolume(subvol.at(0).subvol);
        const QString uuid = subvol.at(0).uuid;
        if (!m_subvolMap->value(snapshotSubvol, "").endsWith(uuid)) {
            const uint64_t snapSubvolId = m_btrfs->subvolId(uuid, snapshotSubvol);
            const uint64_t targetId = m_btrfs->subvolParent(uuid, snapSubvolId);
            QString targetSubvol = m_btrfs->subvolumeName(uuid, targetId);

            // Get the subvolid of the target and do some additional error checking
            if (targetId == 0 || targetSubvol.isEmpty()) {
                continue;
            }

            // Handle a special case where the snapshot is of the root of the Btrfs partition
            if (targetSubvol == ".snapshots") {
                targetSubvol = "";
            }

            m_subvolMap->insert(snapshotSubvol, targetSubvol + "," + uuid);
        }
    }
}

const QString Snapper::findSnapshotSubvolume(const QString &subvol) {
    static QRegularExpression re("\\/[0-9]*\\/snapshot$");
    QStringList subvolSplit = subvol.split(re);

    // If count > 1 than the split worked, otherwise there was no match
    if (subvolSplit.count() > 1) {
        return subvolSplit.at(0);
    } else {
        return QString();
    }
}

const QString Snapper::findTargetPath(const QString &snapshotPath, const QString &filePath, const QString &uuid) {
    // Make sure it is Snapper snapshot
    if (!Btrfs::isSnapper(snapshotPath)) {
        return QString();
    }

    QString snapshotSubvol = findSnapshotSubvolume(snapshotPath);

    // Ensure the root of the partition is mounted and get the mountpoint
    QString mountpoint = Btrfs::mountRoot(uuid);

    QDir mp(mountpoint);

    const QString relSnapshotSubvol = mp.relativeFilePath(snapshotSubvol);
    QString targetSubvol = findTargetSubvol(relSnapshotSubvol, uuid);

    QDir snapshotDir(snapshotPath);

    QString relpath = snapshotDir.relativeFilePath(filePath);

    if (snapshotSubvol.isEmpty() || targetSubvol.isEmpty() || mountpoint.isEmpty() || relpath.isEmpty()) {
        return QString();
    }

    return QDir::cleanPath(mountpoint + QDir::separator() + targetSubvol + QDir::separator() + relpath);
}

const QString Snapper::findTargetSubvol(const QString &snapshotSubvol, const QString &uuid) const {
    if (m_subvolMap->value(snapshotSubvol, "").endsWith(uuid)) {
        return m_subvolMap->value(snapshotSubvol, "").split(",").at(0);
    } else {
        return QString();
    }
}

void Snapper::load() {

    // Load the list of valid configs
    m_configs.clear();
    m_snapshots.clear();
    const SnapperResult result = runSnapper("list-configs --columns config");

    if (result.exitCode != 0 || result.outputList.isEmpty()) {
        return;
    }

    for (const QString &line : qAsConst(result.outputList)) {
        // for each config, add to the map and add it's snapshots to the vector
        SnapperResult listResult;
        QString name = line.trimmed();

        loadConfig(name);

        // The root needs special handling because we may be booted off a snapshot
        if (name == "root") {
            listResult = runSnapper("list --columns number,date,description,type");

            if (listResult.exitCode != 0) {
                continue;
            }

            if (listResult.outputList.isEmpty()) {
                // This means that either there are no snapshots or the root is mounted on non-btrfs filesystem like an overlayfs
                // Let's check the latter case first
                QString subvolName = m_btrfs->subvolumeName(DEFAULT_SNAP_PATH);
                if (subvolName.isEmpty()) {
                    // This probably means there are just no snapshots or we are using a nested subvol in another place
                    continue;
                }

                // Now we need to find out where the snapshots are actually stored
                const uint64_t parentId = m_btrfs->subvolParent(DEFAULT_SNAP_PATH);

                // It shouldn't be possible for the parent to not exist but we check anyway
                if (parentId == 0) {
                    continue;
                }

                const QString uuid = System::runCmd("findmnt", {"-no", "uuid", DEFAULT_SNAP_PATH}, false).output;

                // Make sure the root of the partition is mounted
                QString mountpoint = Btrfs::mountRoot(uuid);
                if (mountpoint.isEmpty()) {
                    continue;
                }

                const QString parentName = m_btrfs->subvolumeName(uuid, parentId);

                listResult = runSnapper("--no-dbus -r " + QDir::cleanPath(mountpoint + QDir::separator() + parentName) +
                                        " list --columns number,date,description,type");
                if (listResult.exitCode != 0 || listResult.outputList.isEmpty()) {
                    // If this is still empty, give up
                    continue;
                }
            }
        } else {
            listResult = runSnapper("list --columns number,date,description,type", name);
            if (listResult.exitCode != 0 || listResult.outputList.isEmpty()) {
                continue;
            }
        }

        for (const QString &snap : qAsConst(listResult.outputList)) {
            m_snapshots[name].append({snap.split(',').at(0).trimmed().toInt(),
                                      QDateTime::fromString(snap.split(',').at(1).trimmed(), Qt::ISODate), snap.split(',').at(2).trimmed(),
                                      snap.split(',').at(3).trimmed()});
        }
    }
    loadSubvols();
}

void Snapper::loadConfig(const QString &name) {
    // If the config is already loaded, remove the old data
    if (m_configs.contains(name)) {
        m_configs.remove(name);
    }

    // Call Snapper to get the config data
    const SnapperResult result = runSnapper("get-config", name);

    if (result.exitCode != 0) {
        return;
    }

    // Iterate over the data adding the name/value pairs to the map
    QMap<QString, QString> config;
    for (const QString &line : result.outputList) {
        if (line.trimmed().isEmpty()) {
            continue;
        }
        QString key = line.split(',').at(0).trimmed();
        QString value = line.split(',').at(1).trimmed();
        config.insert(key, value);
    }

    // Add the map to m_configs
    if (!config.isEmpty()) {
        m_configs[name] = config;
    }
}

void Snapper::loadSubvols() {
    // Clear the existing info
    m_subvols.clear();

    // Get a list of the btrfs filesystems and loop over them
    const QStringList btrfsFilesystems = Btrfs::listFilesystems();
    for (const QString &uuid : btrfsFilesystems) {
        // We need to ensure the root is mounted and get the mountpoint
        QString mountpoint = Btrfs::mountRoot(uuid);
        if (mountpoint.isEmpty()) {
            continue;
        }

        const SubvolumeMap &subvols = m_btrfs->listSubvolumes(uuid);

        for (const auto &subvol : subvols) {

            // Check if it is snapper snapshot
            if (!Btrfs::isSnapper(subvol.subvolName)) {
                continue;
            }

            SnapperSubvolume snapperSubvol;

            snapperSubvol.uuid = uuid;
            snapperSubvol.subvolid = subvol.subvolId;
            snapperSubvol.subvol = subvol.subvolName;

            // It is a snapshot so now we parse it and read the snapper XML
            const QString end = "snapshot";
            QString filename = snapperSubvol.subvol.left(snapperSubvol.subvol.length() - end.length()) + "info.xml";

            filename = QDir::cleanPath(mountpoint + QDir::separator() + filename);

            SnapperSnapshots snap = readSnapperMeta(filename);

            if (snap.number == 0) {
                continue;
            }

            snapperSubvol.desc = snap.desc;
            snapperSubvol.time = snap.time;
            snapperSubvol.snapshotNum = snap.number;
            snapperSubvol.type = snap.type;

            const QString snapshotSubvol = findSnapshotSubvolume(snapperSubvol.subvol);
            if (snapshotSubvol.isEmpty()) {
                continue;
            }

            // Check the map for the target subvolume
            QString targetSubvol = findTargetSubvol(snapshotSubvol, uuid);

            // If it is empty, it may mean the the map isn't loaded yet for the nested subvolumes
            if (targetSubvol.isEmpty()) {
                if (snapshotSubvol.endsWith(DEFAULT_SNAP_PATH)) {
                    const uint64_t targetSubvolId = m_btrfs->subvolId(uuid, snapshotSubvol);
                    const uint64_t parentId = m_btrfs->subvolParent(uuid, targetSubvolId);
                    targetSubvol = m_btrfs->subvolumeName(uuid, parentId);
                } else {
                    continue;
                }
            }

            m_subvols[targetSubvol].append(snapperSubvol);
        }
    }
    createSubvolMap();
}

SnapperSnapshots Snapper::readSnapperMeta(const QString &filename) {
    SnapperSnapshots snap;
    snap.number = 0;
    QFile metaFile(filename);
    if (!metaFile.open(QIODevice::ReadOnly | QIODevice::Text))
        return snap;

    while (!metaFile.atEnd()) {
        QString line = metaFile.readLine();
        if (line.trimmed().startsWith("<num>")) {
            snap.number = line.trimmed().split("<num>").at(1).split("</num>").at(0).trimmed().toInt();
        } else if (line.trimmed().startsWith("<date>")) {
            snap.time = QDateTime::fromString(line.trimmed().split("<date>").at(1).split("</date>").at(0).trimmed(), Qt::ISODate);
            snap.time = snap.time.addSecs(snap.time.offsetFromUtc());
        } else if (line.trimmed().startsWith("<description>")) {
            snap.desc = line.trimmed().split("<description>").at(1).split("</description>").at(0).trimmed();
        } else if (line.trimmed().startsWith("<type>")) {
            snap.type = line.trimmed().split("<type>").at(1).split("</type>").at(0).trimmed();
        }
    }

    return snap;
}

bool Snapper::restoreFile(const QString &sourcePath, const QString &destPath) const {

    // QFile won't overwrite an existing file so we need to remove it first
    if (QFile::exists(destPath)) {
        QFile::remove(destPath);
    }
    if (!QFile::copy(sourcePath, destPath)) {
        return false;
    }

    // Now we need to restore the permissions or it will be owned by root
    QFileInfo qfi(sourcePath);
    if (chown(destPath.toUtf8(), qfi.ownerId(), qfi.groupId()) != 0) {
        qWarning() << tr("Failed to reset ownership of restored file") << Qt::endl;
    }
    QFile::setPermissions(destPath, QFile::permissions(sourcePath));

    return true;
}

const RestoreResult Snapper::restoreSubvol(const QString &uuid, const uint64_t sourceId, const uint64_t targetId) const {
    RestoreResult restoreResult;

    // Get the subvol names associated with the IDs
    const QString sourceName = m_btrfs->subvolumeName(uuid, sourceId);
    const QString targetName = m_btrfs->subvolumeName(uuid, targetId);

    // Ensure the root of the partition is mounted and get the mountpoint
    QString mountpoint = Btrfs::mountRoot(uuid);

    QString snapshotSubvol = findSnapshotSubvolume(sourceName);

    // We are out of excuses, time to do the restore....carefully
    QString targetBackup = targetName + "_backup_" + QDateTime::currentDateTime().toString("yyyyddMMHHmmsszzz");
    restoreResult.backupSubvolName = targetBackup;

    // Find the children before we start
    const QStringList children = m_btrfs->children(targetId, uuid);

    // Rename the target
    if (!Btrfs::renameSubvolume(QDir::cleanPath(mountpoint + QDir::separator() + targetName),
                                QDir::cleanPath(mountpoint + QDir::separator() + targetBackup))) {
        restoreResult.failureMessage = tr("Failed to make a backup of target subvolume");
        return restoreResult;
    }

    // If the snapshot subvolume is nested, we need to set the new subvol name to be inside the backup we created
    QString newSubvolume;
    if (targetName + DEFAULT_SNAP_PATH == snapshotSubvol) {
        newSubvolume = targetBackup + sourceName.right(sourceName.length() - targetName.length());
    } else {
        newSubvolume = sourceName;
    }

    // Place a snapshot of the source where the target was
    bool snapshotSuccess = Btrfs::createSnapshot(QDir::cleanPath(mountpoint + QDir::separator() + newSubvolume).toUtf8(),
                                                 QDir::cleanPath(mountpoint + QDir::separator() + targetName).toUtf8());

    if (!snapshotSuccess) {
        // That failed, try to put the old one back
        Btrfs::renameSubvolume(QDir::cleanPath(mountpoint + QDir::separator() + targetBackup),
                               QDir::cleanPath(mountpoint + QDir::separator() + targetName));
        restoreResult.failureMessage = tr("Failed to restore subvolume!") + "\n\n" +
                                       tr("Snapshot restore failed.  Please verify the status of your system before rebooting");
        return restoreResult;
    }

    // The restore was successful, now we need to move any child subvolumes into the target
    QString childSubvolPath;
    for (const QString &childSubvol : children) {
        childSubvolPath = childSubvol.right(childSubvol.length() - (targetName.length() + 1));

        // rename snapshot
        QString sourcePath = QDir::cleanPath(mountpoint + QDir::separator() + targetBackup + QDir::separator() + childSubvolPath);
        QString destinationPath = QDir::cleanPath(mountpoint + QDir::separator() + childSubvol);
        if (!Btrfs::renameSubvolume(sourcePath, destinationPath)) {
            // If this fails, not much can be done except let the user know
            restoreResult.failureMessage = tr("The restore was successful but the migration of the nested subvolumes failed") + "\n\n" +
                                           tr("Please migrate the those subvolumes manually");
            return restoreResult;
        }
    }

    // If we get to here, it worked!
    restoreResult.success = true;
    return restoreResult;
}

SnapperResult Snapper::setConfig(const QString &name, const QMap<QString, QString> configMap) {
    SnapperResult result;

    const QStringList keys = configMap.keys();

    QString command;
    for (const QString &key : keys) {
        if (configMap[key].isEmpty()) {
            continue;
        }

        command += " " + key + "=" + configMap[key];
    }

    if (command.isEmpty()) {
        result.exitCode = -1;
        result.outputList = QStringList() << tr("Failed to set config");
    } else {
        result = runSnapper("set-config" + command, name);
    }

    loadConfig(name);

    return result;
}

const QVector<SnapperSnapshots> Snapper::snapshots(const QString &config) {
    if (m_snapshots.contains(config)) {
        return m_snapshots[config];
    } else {
        return QVector<SnapperSnapshots>();
    }
}

const QVector<SnapperSubvolume> Snapper::subvols(const QString &config) {
    if (m_subvols.contains(config)) {
        return m_subvols[config];
    } else {
        return QVector<SnapperSubvolume>();
    }
}

/*
 *
 *  Private functions
 *
 */

const SnapperResult Snapper::runSnapper(const QString &command, const QString &name) const {
    Result result;
    SnapperResult snapperResult;

    if (name.isEmpty()) {
        result = System::runCmd(m_snapperCommand + " --machine-readable csv -q " + command, true);
    } else {
        result = System::runCmd(m_snapperCommand + " -c " + name + " --machine-readable csv -q " + command, true);
    }

    snapperResult.exitCode = result.exitCode;

    if (result.exitCode != 0 || result.output.isEmpty()) {
        snapperResult.outputList = QStringList() << result.output;
    } else {
        QStringList outputList = result.output.split('\n');

        // Remove the header
        outputList.removeFirst();

        snapperResult.outputList = outputList;
    }

    return snapperResult;
}
