#include "Btrfs.h"
#include "System.h"

#include <QDebug>
#include <QDir>
#include <QUuid>

// Returns one of the mountpoints for a given UUID
static const QString findMountpoint(const QString &uuid) {
    return System::runCmd("findmnt --real -rno target,uuid | grep " + uuid + " | head -n 1 | awk '{print $1}'", false).output;
}


Btrfs::Btrfs(QObject *parent) : QObject{parent} { reloadVolumes(); }

const BtrfsMeta Btrfs::btrfsVolume(const QString &uuid) {
    // If the uuid isn't found return a default constructed btrfsMeta
    if (!m_btrfsVolumes.contains(uuid)) {
        return BtrfsMeta();
    }

    return m_btrfsVolumes[uuid];
}

const QStringList Btrfs::children(const int subvolid, const QString &uuid) {
    QString mountpoint = findMountpoint(uuid);
    if (mountpoint.isEmpty()) {
        qWarning() << tr("Failed to find mountpoint for UUID " + uuid.toUtf8());
        return QStringList();
    }
    QString output = System::runCmd("sudo btrfs subvolume list " + mountpoint, false).output;
    if (output.isEmpty()) {
        return QStringList();
    }

    QStringList subvols;
    const QStringList outputList = output.split('\n');
    for (const QString &subvolEntry : outputList) {
        if (subvolEntry.isEmpty())
            continue;

        if (subvolEntry.split(' ').at(6).toInt() == subvolid)
            subvols.append(subvolEntry.split(' ').at(8).trimmed());
    }

    return subvols;
}

const bool Btrfs::deleteSubvol(const QString &uuid, const int subvolid) {
    QString subvol;
    if (m_btrfsVolumes.contains(uuid)) {
        subvol = m_btrfsVolumes[uuid].subVolumes.value(QString::number(subvolid));
        if (subvol != QString()) {
            QString mountpoint = mountRoot(uuid);

            // Everything checks out, lets delete the subvol
            if (mountpoint.right(1) != "/")
                mountpoint += "/";
            Result result = System::runCmd("btrfs subvolume delete " + mountpoint + subvol, true);
            if (result.exitCode == 0) {
                return true;
            }
        }
    }

    // If we get to here, it failed
    return false;
}

bool Btrfs::isMounted(const QString &uuid, const int subvolid) {
    return uuid == System::runCmd("findmnt -nO subvolid=" + QString::number(subvolid) + " -o uuid | head -n 1", false).output.trimmed();
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
    return System::runCmd("btrfs filesystem show -m | grep uuid | awk -F':' '{gsub(/ /,\"\");print $3}'", false).output.split('\n');
}

const QStringList Btrfs::listMountpoints() {
    QStringList mountpoints;

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

const QMap<QString, QString> Btrfs::listSubvolumes(const QString &uuid) {
    // If the uuid isn't found return a default constructed QMap
    if (!m_btrfsVolumes.contains(uuid)) {
        return QMap<QString, QString>();
    }

    return m_btrfsVolumes[uuid].subVolumes;
}

const QString Btrfs::mountRoot(const QString &uuid) {
    // Check to see if it is already mounted
    QStringList findmntOutput = System::runCmd("findmnt -nO subvolid=5 -o uuid,target | head -n 1", false).output.split('\n');
    QString mountpoint;
    for (const QString &line : qAsConst(findmntOutput)) {
        if (line.split(' ').at(0).trimmed() == uuid)
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
    // First make sure the data we are trying to reload exists
    if (!m_btrfsVolumes.contains(uuid) || !m_btrfsVolumes[uuid].populated) {
        reloadVolumes();
    }

    // If it still doesn't exist, we need to bail
    if (!m_btrfsVolumes.contains(uuid) || !m_btrfsVolumes[uuid].populated) {
        qWarning() << tr("UUID " + uuid.toUtf8() + " not found!");
        return;
    }

    m_btrfsVolumes[uuid].subVolumes.clear();

    QString mountpoint = findMountpoint(uuid);

    QStringList output = System::runCmd("btrfs subvolume list " + mountpoint, false).output.split('\n');
    QMap<QString, QString> subvols;
    for (const QString &line : qAsConst(output)) {
        if (!line.isEmpty())
            subvols[line.split(' ').at(1)] = line.split(' ').at(8);
    }

    m_btrfsVolumes[uuid].subVolumes = subvols;
}

void Btrfs::reloadVolumes() {
    m_btrfsVolumes.clear();

    QStringList uuidList = listFilesystems();

    for (const QString &uuid : qAsConst(uuidList)) {
        QString mountpoint = findMountpoint(uuid);
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
            m_btrfsVolumes[uuid] = btrfs;
            reloadSubvols(uuid);
        }
    }
}

bool Btrfs::renameSubvolume(const QString &source, const QString &target) {
    QDir dir;
    return dir.rename(source, target);
}

const int Btrfs::subvolId(const QString &uuid, const QString &subvol) {
    if (!m_btrfsVolumes.contains(uuid) || !m_btrfsVolumes[uuid].populated) {
        return 0;
    }

    return m_btrfsVolumes[uuid].subVolumes.key(subvol).toInt();
}
