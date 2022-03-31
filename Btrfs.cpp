#include "Btrfs.h"
#include "System.h"

#include <QDebug>
#include <QDir>
#include <QUuid>

Btrfs::Btrfs(QObject *parent) : QObject{parent} { reloadVolumes(); }

const BtrfsMeta Btrfs::btrfsVolume(const QString &uuid) const {
    // If the uuid isn't found return a default constructed btrfsMeta
    if (!m_volumes.contains(uuid)) {
        return BtrfsMeta();
    }

    return m_volumes[uuid];
}

const QStringList Btrfs::children(const int subvolId, const QString &uuid) const {
    QStringList children;
    if (m_volumes.contains(uuid) && m_volumes[uuid].subvolumes.contains(subvolId)) {
        const QList<int> keys = m_volumes[uuid].subvolumes.keys();
        for (const int &key : keys) {
            if (m_volumes[uuid].subvolumes[key].parentId == subvolId) {
                children.append(m_volumes[uuid].subvolumes[key].subvolName);
            }
        }
    }

    return children;
}

const bool Btrfs::deleteSubvol(const QString &uuid, const int subvolid) {
    Subvolume subvol;
    if (m_volumes.contains(uuid)) {
        subvol = m_volumes[uuid].subvolumes.value(subvolid);
        if (subvol.parentId != 0) {
            QString mountpoint = mountRoot(uuid);

            // Everything checks out, lets delete the subvol
            if (mountpoint.right(1) != "/")
                mountpoint += "/";
            Result result = System::runCmd("btrfs subvolume delete " + mountpoint + subvol.subvolName, true);
            if (result.exitCode == 0) {
                return true;
            }
        }
    }

    // If we get to here, it failed
    return false;
}

bool Btrfs::isSnapper(const QString &subvolume) {
    static QRegularExpression re("\\/[0-9]*\\/snapshot$");
    return re.match(subvolume).hasMatch();
}

bool Btrfs::isMounted(const QString &uuid, const int subvolid) {
    const QStringList outputList =
        System::runCmd("findmnt -nO subvolid=" + QString::number(subvolid) + " -o uuid", false).output.trimmed().split("\n");
    return uuid == outputList.at(0).trimmed();
}

const QString Btrfs::findRootSubvol() {
    const Result findmntResult = System::runCmd("LANG=C findmnt -no uuid,options /", false);
    if (findmntResult.exitCode != 0 || findmntResult.output.isEmpty())
        return QString();

    const QString uuid = findmntResult.output.split(' ').at(0).trimmed();
    const QString options = findmntResult.output.right(findmntResult.output.length() - uuid.length()).trimmed();
    if (options.isEmpty() || uuid.isEmpty())
        return QString();

    QString subvol;
    const QStringList optionsList = options.split(',');
    for (const QString &option : optionsList) {
        if (option.startsWith("subvol="))
            subvol = option.split("subvol=").at(1);
    }

    // Make sure subvolume doesn't have a leading slash
    if (subvol.startsWith("/"))
        subvol = subvol.right(subvol.length() - 1);

    // At this point subvol will either contain nothing or the name of the subvol
    return subvol;
}

const QStringList Btrfs::listFilesystems() {
    const QStringList outputList = System::runCmd("btrfs filesystem show -m", false).output.split('\n');
    QStringList uuids;
    for (const QString &line : outputList) {
        if (line.contains("uuid:")) {
            uuids.append(line.split("uuid:").at(1).trimmed());
        }
    }
    return uuids;
}

const QStringList Btrfs::listMountpoints() {
    QStringList mountpoints;

    // loop through mountpoints and only include those with btrfs fileystem
    const QStringList output = System::runCmd("findmnt --real -lno fstype,target", false).output.trimmed().split('\n');
    for (const QString &line : output) {
        if (line.startsWith("btrfs")) {
            QString mountpoint = line.simplified().split(' ').at(1).trimmed();
            QStringList crap = line.split(' ');
            qDebug() << crap;
            if (!mountpoint.isEmpty()) {
                mountpoints.append(mountpoint);
            }
        }
    }

    mountpoints.sort();

    return mountpoints;
}

const QMap<int, Subvolume> Btrfs::listSubvolumes(const QString &uuid) const {
    // If the uuid isn't found return a default constructed QMap
    if (!m_volumes.contains(uuid)) {
        return QMap<int, Subvolume>();
    }

    return m_volumes[uuid].subvolumes;
}

const QString Btrfs::mountRoot(const QString &uuid) {
    // Check to see if it is already mounted
    QStringList findmntOutput = System::runCmd("findmnt -nO subvolid=5 -o uuid,target", false).output.split('\n');
    QString mountpoint;
    for (const QString &line : qAsConst(findmntOutput)) {
        if (!line.isEmpty() && line.split(' ').at(0).trimmed() == uuid)
            mountpoint = line.split(' ').at(1).trimmed();
    }

    // If it isn't mounted we need to mount it
    if (mountpoint.isEmpty()) {
        // Format a temp mountpoint using a GUID
        mountpoint = QDir::cleanPath(QDir::tempPath() + QDir::separator() + QUuid::createUuid().toString());

        // Create the mountpoint and mount the volume if successful
        QDir tempMount;
        if (tempMount.mkpath(mountpoint))
            System::runCmd("mount -t btrfs -o subvolid=5 UUID=" + uuid + " " + mountpoint, false);
        else
            return QString();
    }

    return mountpoint;
}

void Btrfs::reloadSubvols(const QString &uuid) {
    if (UuidIsValid(uuid)) {
        m_volumes[uuid].subvolumes.clear();

        QString mountpoint = mountRoot(uuid);

        QStringList output = System::runCmd("btrfs subvolume list " + mountpoint, false).output.split('\n');
        QMap<int, Subvolume> subvols;
        for (const QString &line : qAsConst(output)) {
            if (!line.isEmpty()) {
                Subvolume subvol;
                int subvolId = line.split(' ').at(1).toInt();
                subvol.subvolName = line.split(' ').at(8).trimmed();
                subvol.parentId = line.split(' ').at(6).toInt();
                subvols[subvolId] = subvol;
            }
        }

        m_volumes[uuid].subvolumes = subvols;
    }
}

void Btrfs::reloadVolumes() {
    m_volumes.clear();

    QStringList uuidList = listFilesystems();

    // Loop through btrfs devices and retrieve filesystem usage
    for (const QString &uuid : qAsConst(uuidList)) {
        QString mountpoint = mountRoot(uuid);
        if (!mountpoint.isEmpty()) {
            BtrfsMeta btrfs;
            btrfs.populated = true;
            btrfs.mountPoint = mountpoint;
            QStringList usageLines = System::runCmd("LANG=C ; btrfs fi usage -b " + mountpoint, false).output.split('\n');
            for (const QString &line : qAsConst(usageLines)) {
                QString type = line.split(':').at(0).trimmed();
                if (type == "Device size") {
                    btrfs.totalSize = line.split(':').at(1).trimmed().toLong();
                } else if (type == "Device allocated") {
                    btrfs.allocatedSize = line.split(':').at(1).trimmed().toLong();
                } else if (type == "Used") {
                    btrfs.usedSize = line.split(':').at(1).trimmed().toLong();
                } else if (type == "Free (estimated)") {
                    btrfs.freeSize = line.split(':').at(1).split(QRegExp("\\s+"), Qt::SkipEmptyParts).at(0).trimmed().toLong();
                } else if (type.startsWith("Data,")) {
                    btrfs.dataSize = line.split(':').at(2).split(',').at(0).trimmed().toLong();
                    btrfs.dataUsed = line.split(':').at(3).split(' ').at(0).trimmed().toLong();
                } else if (type.startsWith("Metadata,")) {
                    btrfs.metaSize = line.split(':').at(2).split(',').at(0).trimmed().toLong();
                    btrfs.metaUsed = line.split(':').at(3).split(' ').at(0).trimmed().toLong();
                } else if (type.startsWith("System,")) {
                    btrfs.sysSize = line.split(':').at(2).split(',').at(0).trimmed().toLong();
                    btrfs.sysUsed = line.split(':').at(3).split(' ').at(0).trimmed().toLong();
                }
            }
            m_volumes[uuid] = btrfs;
            reloadSubvols(uuid);
        }
    }
}

bool Btrfs::renameSubvolume(const QString &source, const QString &target) {
    QDir dir;
    // If there is an empty dir at target, remove it
    if (dir.exists(target)) {
        dir.rmdir(target);
    }
    return dir.rename(source, target);
}

const int Btrfs::subvolId(const QString &uuid, const QString &subvolName) const {
    if (!m_volumes.contains(uuid) || !m_volumes[uuid].populated) {
        return 0;
    }

    const QMap<int, Subvolume> subvols = m_volumes[uuid].subvolumes;
    int subvolId = 0;
    QList<int> subvolIds = subvols.keys();
    for (const int &subvol : subvolIds) {
        if (subvols[subvol].subvolName.trimmed() == subvolName.trimmed()) {
            subvolId = subvol;
        }
    }
    return subvolId;
}

const QString Btrfs::subvolName(const QString &uuid, const int subvolId) const {
    if (m_volumes.contains(uuid) && m_volumes[uuid].subvolumes.contains(subvolId)) {
        return m_volumes[uuid].subvolumes[subvolId].subvolName;
    } else {
        return QString();
    }
}

const int Btrfs::subvolTopParent(const QString &uuid, const int subvolId) const {
    int parentId = subvolId;
    if (m_volumes.contains(uuid) && m_volumes[uuid].subvolumes.contains(subvolId)) {
        int levels = 0;
        while (levels < 2 && m_volumes[uuid].subvolumes[parentId].parentId != 5) {
            parentId = m_volumes[uuid].subvolumes[parentId].parentId;
            levels++;
        }
    } else {
        return 0;
    }

    return parentId;
}

bool Btrfs::UuidIsValid(const QString &uuid) {
    // First make sure the data we are trying to access exists
    if (!m_volumes.contains(uuid) || !m_volumes[uuid].populated) {
        reloadVolumes();
    }

    // If it still doesn't exist, we need to bail
    if (!m_volumes.contains(uuid) || !m_volumes[uuid].populated) {
        qWarning() << tr("UUID " + uuid.toUtf8() + " not found!");
        return false;
    }

    return true;
}

void Btrfs::startBalanceRoot(const QString &uuid) {
    if (UuidIsValid(uuid)) {
        QString mountpoint = mountRoot(uuid);

        // Run full balance command against UUID top level subvolume.
        System::runCmd("btrfs balance start " + mountpoint + " --full-balance --bg", false);
    }
}

void Btrfs::stopBalanceRoot(const QString &uuid) {
    if (UuidIsValid(uuid)) {
        QString mountpoint = mountRoot(uuid);

        System::runCmd("btrfs balance cancel " + mountpoint, false);
    }
}

void Btrfs::startScrubRoot(const QString &uuid) {
    if (UuidIsValid(uuid)) {
        QString mountpoint = mountRoot(uuid);

        System::runCmd("btrfs scrub start " + mountpoint, false);
    }
}

void Btrfs::stopScrubRoot(const QString &uuid) {
    if (UuidIsValid(uuid)) {
        QString mountpoint = mountRoot(uuid);

        System::runCmd("btrfs scrub cancel " + mountpoint, false);
    }
}

void Btrfs::startDefragRoot(const QString &uuid) {
    if (UuidIsValid(uuid)) {
        QString mountpoint = mountRoot(uuid);

        System::runCmd("btrfs filesystem defragment start " + mountpoint + " -r", false);
    }
}

const QString Btrfs::checkBalanceStatus(const QString &mountpoint) const {
    return System::runCmd("btrfs balance status " + mountpoint, false).output;
}

const QString Btrfs::checkScrubStatus(const QString &mountpoint) const {
    return System::runCmd("btrfs scrub status " + mountpoint, false).output;
}

const QString Btrfs::checkDefragStatus(const QString &mountpoint) const {
    return System::runCmd("btrfs defrag status " + mountpoint, false).output;
}
