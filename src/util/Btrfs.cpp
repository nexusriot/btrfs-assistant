#include "util/Btrfs.h"
#include "util/Settings.h"
#include "util/System.h"
#include <sys/mount.h>

#include <QDebug>
#include <QDir>
#include <QRegularExpression>
#include <QTemporaryDir>
#include <btrfsutil.h>

namespace {

QString uuidToString(const uint8_t uuid[16])
{
    QString ret;
    bool allZeros = true;
    for (int i = 0; i < 16; ++i) {
        ret.append(QString::number(uuid[i], 16));
        if ((i + 1) % 2 == 0 && (i > 1 && i < 10)) {
            ret.append('-');
        }
        allZeros &= (uuid[i] == 0);
    }
    return allZeros ? "" : ret;
}

Subvolume infoToSubvolume(const QString &fileSystemUuid, const QString &name, const struct btrfs_util_subvolume_info &subvolInfo)
{
    Subvolume ret;
    ret.subvolName = name;
    ret.parentId = subvolInfo.parent_id;
    ret.id = subvolInfo.id;
    ret.uuid = uuidToString(subvolInfo.uuid);
    ret.parentUuid = uuidToString(subvolInfo.parent_uuid);
    ret.receivedUuid = uuidToString(subvolInfo.received_uuid);
    ret.generation = subvolInfo.generation;
    ret.flags = subvolInfo.flags;
    ret.createdAt = QDateTime::fromSecsSinceEpoch(subvolInfo.otime.tv_sec);
    ret.filesystemUuid = fileSystemUuid;
    return ret;
}

} // namespace

Btrfs::Btrfs(QObject *parent) : QObject{parent} { loadVolumes(); }

Btrfs::~Btrfs() { unmountFilesystems(); }

QString Btrfs::balanceStatus(const QString &mountpoint) const
{
    return System::runCmd("btrfs", {"balance", "status", mountpoint}, false).output;
}

BtrfsFilesystem Btrfs::filesystem(const QString &uuid) const
{
    // If the uuid isn't found return a default constructed btrfsMeta
    if (!m_filesystems.contains(uuid)) {
        return BtrfsFilesystem();
    }

    return m_filesystems[uuid];
}

QStringList Btrfs::children(const uint64_t subvolId, const QString &uuid) const
{
    const QString mountpoint = findAnyMountpoint(uuid);
    btrfs_util_subvolume_iterator *iter;

    btrfs_util_error returnCode = btrfs_util_create_subvolume_iterator(mountpoint.toLocal8Bit(), BTRFS_ROOT_ID, 0, &iter);
    if (returnCode != BTRFS_UTIL_OK) {
        return QStringList();
    }

    QStringList children;

    while (returnCode != BTRFS_UTIL_ERROR_STOP_ITERATION) {
        char *path = nullptr;
        struct btrfs_util_subvolume_info subvolInfo;
        returnCode = btrfs_util_subvolume_iterator_next_info(iter, &path, &subvolInfo);
        if (returnCode == BTRFS_UTIL_OK && subvolInfo.parent_id == subvolId) {
            children.append(QString::fromLocal8Bit(path));
            free(path);
        }
    }

    btrfs_util_destroy_subvolume_iterator(iter);
    return children;
}

bool Btrfs::createSnapshot(const QString &source, const QString &dest, bool readOnly)
{
    return btrfs_util_create_snapshot(source.toLocal8Bit(), dest.toLocal8Bit(), (readOnly ? BTRFS_UTIL_CREATE_SNAPSHOT_READ_ONLY : 0),
                                      nullptr, nullptr) == BTRFS_UTIL_OK;
}

std::optional<Subvolume> Btrfs::createSnapshot(const QString &uuid, uint64_t subvolId, const QString &dest, bool readOnly)
{
    std::optional<Subvolume> ret;
    if (m_filesystems.contains(uuid) && m_filesystems.value(uuid).subvolumes.contains(subvolId)) {
        const QString mountpoint = mountRoot(uuid);
        const QString subvolPath = QDir::cleanPath(mountpoint + QDir::separator() + subvolumeName(uuid, subvolId).name);

        if (createSnapshot(subvolPath, dest, readOnly)) {
            struct btrfs_util_subvolume_info subvolInfo;
            btrfs_util_error returnCode = btrfs_util_subvolume_info(dest.toLocal8Bit(), 0, &subvolInfo);
            if (returnCode == BTRFS_UTIL_OK) {
                ret = infoToSubvolume(uuid, subvolumeName(dest).name, subvolInfo);
                m_filesystems[uuid].subvolumes[ret->id] = *ret;
            }
        }
    }
    return ret;
}

bool Btrfs::deleteSubvol(const QString &uuid, const uint64_t subvolid)
{
    Subvolume subvol;
    if (m_filesystems.contains(uuid)) {
        subvol = m_filesystems[uuid].subvolumes.value(subvolid);
        if (subvol.parentId != 0) {
            QString mountpoint = mountRoot(uuid);

            // Everything checks out, lets delete the subvol
            const QString subvolPath = QDir::cleanPath(mountpoint + QDir::separator() + subvol.subvolName);
            btrfs_util_error returnCode = btrfs_util_delete_subvolume(subvolPath.toLocal8Bit(), 0);
            if (returnCode == BTRFS_UTIL_OK) {
                return true;
            }
        }
    }

    // If we get to here, it failed
    return false;
}

QString Btrfs::findAnyMountpoint(const QString &uuid)
{
    const QStringList outputList = System::runCmd("findmnt", {"-t", "btrfs", "-lno", "uuid,target"}, false).output.split("\n");

    for (const QString &line : outputList) {
        if (line.section(" ", 0, 0).trimmed() == uuid) {
            return line.section(" ", 1).trimmed();
        }
    }

    return QString();
}

bool Btrfs::isSnapper(const QString &subvolume)
{
    static QRegularExpression re("\\/[0-9]*\\/snapshot$");
    return re.match(subvolume).hasMatch();
}

bool Btrfs::isSubvolumeBackup(QString subvolPath)
{
    static QRegularExpression re("_backup_[0-9]{17}");
    const QStringList nameParts = subvolPath.split(re);

    return nameParts.count() == 2;
}

bool Btrfs::isMounted(const QString &uuid, const uint64_t subvolid)
{
    const QStringList outputList =
        System::runCmd("findmnt -nO subvolid=" + QString::number(subvolid) + " -o uuid", false).output.trimmed().split("\n");
    return uuid == outputList.at(0).trimmed();
}

bool Btrfs::isQuotaEnabled(const QString &mountpoint)
{
    return !System::runCmd("btrfs", {QStringLiteral("qgroup"), QStringLiteral("show"), mountpoint}, false).output.isEmpty();
}

QStringList Btrfs::listFilesystems()
{
    const QStringList outputList = System::runCmd("btrfs filesystem show -m", false).output.split('\n');
    QStringList uuids;
    for (const QString &line : outputList) {
        if (line.contains("uuid:")) {
            uuids.append(line.split("uuid:").at(1).trimmed());
        }
    }
    return uuids;
}

QStringList Btrfs::listMountpoints()
{
    QStringList mountpoints;

    // Find all btrfs mountpoints
    const QStringList output = System::runCmd("findmnt --real -t btrfs -lno target", false).output.trimmed().split('\n');
    for (const QString &line : output) {
        const QString mountpoint = line.trimmed();
        if (!mountpoint.isEmpty()) {
            mountpoints.append(mountpoint);
        }
    }

    mountpoints.sort();

    return mountpoints;
}

SubvolumeMap Btrfs::listSubvolumes(const QString &uuid) const { return m_filesystems.value(uuid).subvolumes; }

void Btrfs::loadQgroups(const QString &uuid)
{
    if (!isUuidLoaded(uuid)) {
        return;
    }

    const QString mountpoint = findAnyMountpoint(uuid);
    if (mountpoint.isEmpty()) {
        return;
    }

    if (!isQuotaEnabled(mountpoint)) {
        // If qgroups aren't enabled we need to abort
        return;
    }

    QStringList outputList = System::runCmd("btrfs", {"qgroup", "show", "--raw", "--sync", mountpoint}, false).output.split("\n");

    // The header takes the first two lines, make sure it is more than two lines and then consume them
    if (outputList.count() <= 2) {
        return;
    }
    outputList.takeFirst();
    outputList.takeFirst();

    // Load the data

    for (const QString &line : qAsConst(outputList)) {
        const QStringList qgroupList = line.split(" ", Qt::SkipEmptyParts);
        uint64_t subvolId;
        if (!qgroupList.at(0).contains("/")) {
            continue;
        }

        subvolId = qgroupList.at(0).split("/").at(1).toUInt();

        if (m_filesystems[uuid].subvolumes.contains(subvolId)) {
            m_filesystems[uuid].subvolumes[subvolId].size = qgroupList.at(1).toULong();
            m_filesystems[uuid].subvolumes[subvolId].exclusive = qgroupList.at(2).toULong();
        }
    }
}

void Btrfs::loadSubvols(const QString &uuid)
{
    if (isUuidLoaded(uuid)) {
        m_filesystems[uuid].subvolumes.clear();

        const QString mountpoint = findAnyMountpoint(uuid);
        btrfs_util_subvolume_iterator *iter;

        btrfs_util_error returnCode = btrfs_util_create_subvolume_iterator(mountpoint.toLocal8Bit(), BTRFS_ROOT_ID, 0, &iter);
        if (returnCode != BTRFS_UTIL_OK) {
            return;
        }

        SubvolumeMap subvols;

        while (returnCode != BTRFS_UTIL_ERROR_STOP_ITERATION) {
            char *path = nullptr;
            struct btrfs_util_subvolume_info subvolInfo;
            returnCode = btrfs_util_subvolume_iterator_next_info(iter, &path, &subvolInfo);
            if (returnCode == BTRFS_UTIL_OK) {
                subvols[subvolInfo.id] = infoToSubvolume(uuid, QString::fromLocal8Bit(path), subvolInfo);
                free(path);
            }
        }
        btrfs_util_destroy_subvolume_iterator(iter);

        // We need to add the root at subvolid 5
        struct btrfs_util_subvolume_info subvolInfo;
        returnCode = btrfs_util_subvolume_info(mountpoint.toLocal8Bit(), BTRFS_ROOT_ID, &subvolInfo);
        if (returnCode == BTRFS_UTIL_OK) {
            subvols[subvolInfo.id] = infoToSubvolume(uuid, QString(), subvolInfo);
        }

        m_filesystems[uuid].subvolumes = subvols;
        loadQgroups(uuid);
    }
}

void Btrfs::loadVolumes()
{
    QStringList uuidList = listFilesystems();

    // Loop through btrfs devices and retrieve filesystem usage
    for (const QString &uuid : qAsConst(uuidList)) {
        QString mountpoint = findAnyMountpoint(uuid);
        if (!mountpoint.isEmpty()) {
            BtrfsFilesystem btrfs;
            btrfs.isPopulated = true;
            QStringList usageLines = System::runCmd("LANG=C ; btrfs fi usage -b \"" + mountpoint + "\"", false).output.split('\n');
            for (const QString &line : qAsConst(usageLines)) {
                const QStringList &cols = line.split(':');
                QString type = cols.at(0).trimmed();
                if (type == "Device size") {
                    btrfs.totalSize = cols.at(1).trimmed().toULong();
                } else if (type == "Device allocated") {
                    btrfs.allocatedSize = cols.at(1).trimmed().toULong();
                } else if (type == "Used") {
                    btrfs.usedSize = cols.at(1).trimmed().toULong();
                } else if (type == "Free (estimated)") {
                    btrfs.freeSize = cols.at(1).split(QRegExp("\\s+"), Qt::SkipEmptyParts).at(0).trimmed().toULong();
                } else if (type.startsWith("Data,")) {
                    btrfs.dataSize = cols.at(2).split(',').at(0).trimmed().toULong();
                    btrfs.dataUsed = cols.at(3).split(' ').at(0).trimmed().toULong();
                } else if (type.startsWith("Metadata,")) {
                    btrfs.metaSize = cols.at(2).split(',').at(0).trimmed().toULong();
                    btrfs.metaUsed = cols.at(3).split(' ').at(0).trimmed().toULong();
                } else if (type.startsWith("System,")) {
                    btrfs.sysSize = cols.at(2).split(',').at(0).trimmed().toULong();
                    btrfs.sysUsed = cols.at(3).split(' ').at(0).trimmed().toULong();
                }
            }
            m_filesystems[uuid] = btrfs;
            loadSubvols(uuid);
        }
    }
}

QString Btrfs::mountRoot(const QString &uuid)
{
    // Check to see if it is already mounted
    QStringList findmntOutput =
        System::runCmd("findmnt", {"-nO", "subvolid=" + QString::number(BTRFS_ROOT_ID), "-o", "uuid,target"}, false).output.split('\n');
    QString mountpoint;
    for (const QString &line : qAsConst(findmntOutput)) {
        if (!line.isEmpty() && line.split(' ').at(0).trimmed() == uuid)
            mountpoint = line.section(' ', 1).trimmed();
    }

    // If it isn't mounted we need to mount it
    if (mountpoint.isEmpty()) {
        mountpoint = QDir::cleanPath(System::mountPathRoot() + QDir::separator() + uuid);

        // Add this mountpoint to a list so it can be unmounted later
        m_tempMountpoints.append(mountpoint);

        // Create the mountpoint and mount the volume if successful
        QDir tempMount;
        const QString device = QDir::cleanPath(QStringLiteral("/dev/disk/by-uuid/") + uuid);
        const QString options = "subvolid=" + QString::number(BTRFS_ROOT_ID);
        if (!(tempMount.mkpath(mountpoint) &&
              mount(device.toLocal8Bit(), mountpoint.toLocal8Bit(), "btrfs", 0, options.toLocal8Bit()) == 0)) {
            return QString();
        }
    }

    return mountpoint;
}

bool Btrfs::renameSubvolume(const QString &source, const QString &target)
{
    QDir dir;
    // If there is an empty dir at target, remove it
    if (dir.exists(target)) {
        dir.rmdir(target);
    }
    return dir.rename(source, target);
}

RestoreResult Btrfs::restoreSubvol(const QString &uuid, const uint64_t sourceId, const uint64_t targetId, const QString &customName)
{
    RestoreResult restoreResult;

    if (targetId == 5) {
        restoreResult.failureMessage = tr("You cannot restore to the root of the partition");
        restoreResult.isSuccess = false;
        return restoreResult;
    }

    // Get the subvol names associated with the IDs
    const QString sourceName = subvolumeName(uuid, sourceId).name;
    const QString targetName = subvolumeName(uuid, targetId).name;

    // Ensure the root of the partition is mounted and get the mountpoint
    QString mountpoint = mountRoot(uuid);

    // We are out of excuses, time to do the restore....carefully
    QString targetBackup = targetName + "_backup_" + QDateTime::currentDateTime().toString("yyyyddMMHHmmsszzz");

    if (!customName.trimmed().isEmpty()) {
        targetBackup += "_" + customName.trimmed();
    }

    restoreResult.backupSubvolName = targetBackup;

    // Find the children before we start
    const QStringList children = this->children(targetId, uuid);

    // Rename the target
    if (!Btrfs::renameSubvolume(QDir::cleanPath(mountpoint + QDir::separator() + targetName),
                                QDir::cleanPath(mountpoint + QDir::separator() + targetBackup))) {
        restoreResult.failureMessage = tr("Failed to make a backup of target subvolume");
        return restoreResult;
    }

    // If the source is nested inside the target, set the path to match the renamed target
    QString newSubvolume;
    if (sourceName.startsWith(QDir::cleanPath(targetName) + QDir::separator())) {
        newSubvolume = targetBackup + sourceName.right(sourceName.length() - targetName.length());
    } else {
        newSubvolume = sourceName;
    }

    // Place a snapshot of the source where the target was
    bool snapshotSuccess = Btrfs::createSnapshot(QDir::cleanPath(mountpoint + QDir::separator() + newSubvolume).toUtf8(),
                                                 QDir::cleanPath(mountpoint + QDir::separator() + targetName).toUtf8(), false);

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
    restoreResult.isSuccess = true;
    return restoreResult;
}

QString Btrfs::scrubStatus(const QString &mountpoint) const
{
    return System::runCmd("btrfs", {"scrub", "status", mountpoint}, false).output;
}

void Btrfs::setQgroupEnabled(const QString &mountpoint, bool enable)
{
    if (enable) {
        System::runCmd(QStringLiteral("btrfs"), {QStringLiteral("quota"), QStringLiteral("enable"), mountpoint}, false);
    } else {
        System::runCmd(QStringLiteral("btrfs"), {QStringLiteral("quota"), QStringLiteral("disable"), mountpoint}, false);
    }
}

bool Btrfs::isSubvolume(const QString &path) { return btrfs_util_is_subvolume(path.toLocal8Bit()); }

uint64_t Btrfs::subvolId(const QString &uuid, const QString &subvolName)
{
    const QString mountpoint = mountRoot(uuid);
    if (mountpoint.isEmpty()) {
        return 0;
    }
    const QString subvolPath = QDir::cleanPath(mountpoint + QDir::separator() + subvolName);
    uint64_t id;
    btrfs_util_error returnCode = btrfs_util_subvolume_id(subvolPath.toLocal8Bit(), &id);
    if (returnCode == BTRFS_UTIL_OK) {
        return id;
    } else {
        return 0;
    }
}

SubvolResult Btrfs::subvolumeName(const QString &uuid, const uint64_t subvolId) const
{
    if (m_filesystems.contains(uuid) && m_filesystems[uuid].subvolumes.contains(subvolId)) {
        return {m_filesystems[uuid].subvolumes[subvolId].subvolName, true};
    } else {
        return {QString(), false};
    }
}

SubvolResult Btrfs::subvolumeName(const QString &path)
{
    SubvolResult ret;
    char *subvolName = nullptr;
    btrfs_util_error returnCode = btrfs_util_subvolume_path(path.toLocal8Bit(), 0, &subvolName);
    if (returnCode == BTRFS_UTIL_OK) {
        ret = {QString::fromLocal8Bit(subvolName), true};
        free(subvolName);
    }
    return ret;
}

uint64_t Btrfs::subvolParent(const QString &uuid, const uint64_t subvolId) const
{
    if (m_filesystems.contains(uuid) && m_filesystems[uuid].subvolumes.contains(subvolId)) {
        return m_filesystems[uuid].subvolumes[subvolId].parentId;
    } else {
        return 0;
    }
}

uint64_t Btrfs::subvolParent(const QString &path) const
{
    struct btrfs_util_subvolume_info subvolInfo;
    btrfs_util_error returnCode = btrfs_util_subvolume_info(path.toLocal8Bit(), 0, &subvolInfo);
    if (returnCode != BTRFS_UTIL_OK) {
        return 0;
    }

    return subvolInfo.parent_id;
}

bool Btrfs::setSubvolumeReadOnly(const QString &path, bool readOnly)
{
    return btrfs_util_set_subvolume_read_only(path.toLocal8Bit(), readOnly) == BTRFS_UTIL_OK;
}

bool Btrfs::setSubvolumeReadOnly(const QString &uuid, uint64_t subvolId, bool readOnly)
{
    bool ret = false;
    if (m_filesystems.contains(uuid) && m_filesystems.value(uuid).subvolumes.contains(subvolId)) {
        Subvolume &subvol = m_filesystems[uuid].subvolumes[subvolId];
        const QString mountpoint = mountRoot(uuid);
        const QString subvolPath = QDir::cleanPath(mountpoint + QDir::separator() + subvol.subvolName);

        ret = setSubvolumeReadOnly(subvolPath, readOnly);
        if (ret) {
            subvol.flags = readOnly ? 0x1u : 0;
        }
    }
    return ret;
}

bool Btrfs::setSubvolumeReadOnly(const Subvolume &subvol, bool readOnly)
{
    return setSubvolumeReadOnly(subvol.filesystemUuid, subvol.id, readOnly);
}

bool Btrfs::isSubvolumeReadOnly(const QString &path)
{
    bool ret = false;
    if (btrfs_util_get_subvolume_read_only(path.toLocal8Bit(), &ret) != BTRFS_UTIL_OK) {
        ret = false;
    }
    return ret;
}

bool Btrfs::isUuidLoaded(const QString &uuid)
{
    // First make sure the data we are trying to access exists
    if (!m_filesystems.contains(uuid) || !m_filesystems[uuid].isPopulated) {
        loadVolumes();
    }

    // If it still doesn't exist, we need to bail
    if (!m_filesystems.contains(uuid) || !m_filesystems[uuid].isPopulated) {
        qWarning() << tr("UUID " + uuid.toLocal8Bit() + " not found!");
        return false;
    }

    return true;
}

void Btrfs::startBalanceRoot(const QString &uuid)
{
    if (isUuidLoaded(uuid)) {
        QString mountpoint = findAnyMountpoint(uuid);

        // Run full balance command against UUID top level subvolume.
        System::runCmd("btrfs", {"balance", "start", mountpoint, "--full-balance", "--bg"}, false);
    }
}

void Btrfs::startScrubRoot(const QString &uuid)
{
    if (isUuidLoaded(uuid)) {
        QString mountpoint = findAnyMountpoint(uuid);

        System::runCmd("btrfs", {"scrub", "start", mountpoint}, false);
    }
}

void Btrfs::stopBalanceRoot(const QString &uuid)
{
    if (isUuidLoaded(uuid)) {
        QString mountpoint = findAnyMountpoint(uuid);

        System::runCmd("btrfs", {"balance", "cancel", mountpoint}, false);
    }
}

void Btrfs::stopScrubRoot(const QString &uuid)
{
    if (isUuidLoaded(uuid)) {
        QString mountpoint = findAnyMountpoint(uuid);

        System::runCmd("btrfs", {"scrub", "cancel", mountpoint}, false);
    }
}

void Btrfs::unmountFilesystems()
{
    for (const QString &mountpoint : qAsConst(m_tempMountpoints)) {
        umount2(mountpoint.toLocal8Bit(), MNT_DETACH);
    }
}

bool Subvolume::isEmpty() const { return id == 0; }

bool Subvolume::isReadOnly() const { return flags & 0x1u; }

bool Subvolume::isSnapshot() const { return !parentUuid.isEmpty(); }

bool Subvolume::isReceived() const { return !receivedUuid.isEmpty(); }
