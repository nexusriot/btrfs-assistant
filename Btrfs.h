#ifndef BTRFS_H
#define BTRFS_H

#include "SubvolModel.h"

#include <QDir>
#include <QMap>
#include <QObject>
#include <QRegularExpression>
#include <QTime>

struct RestoreResult {
    bool success = false;
    QString failureMessage;
    QString backupSubvolName;
};

struct BtrfsMeta {
    bool populated = false;
    QString mountPoint;
    long totalSize;
    long allocatedSize;
    long usedSize;
    long freeSize;
    long dataSize;
    long dataUsed;
    long metaSize;
    long metaUsed;
    long sysSize;
    long sysUsed;
    QMap<int, Subvolume> subvolumes;
};

/**
 * @brief The Btrfs service class handles all btrfs device functionality.
 */
class Btrfs : public QObject {
    Q_OBJECT

  public:
    explicit Btrfs(QObject *parent = nullptr);

    /**
     * @brief Checks the balance status of a given subvolume.
     * @param mountpoint - A Qstring that represents the mountpoint to check for a btrfs balance on
     * @return Qstring that contains the output from the btrfs balance command.
     */
    const QString balanceStatus(const QString &mountpoint) const;

    /** @brief Returns the data for the Btrfs volume identified by @p UUID
     *
     * Returns a BtrfsMeta struct that represents the btrfs filesystem identified by @p UUID.
     * If no data is found for the given UUID, it returns a default contructed value
     *
     */
    const BtrfsMeta btrfsVolume(const QString &uuid) const;

    /** @brief Returns the direct children for a given subvolume
     *
     *  Finds all children which are a single generation below the parent subvol identified by
     *  @p subvolid on volume @p uuid.
     *
     *  Returns a QStringList containing the subvol names/paths of all the children subvols
     *
     */
    const QStringList children(const int subvolid, const QString &uuid) const;

    /** @brief Deletes a given subvolume
     *
     *  Deletes the subvol represented by @p subvolid on @p uuid.
     *
     *  Returns true if successful and false if it fails for any reason.
     *
     */
    const bool deleteSubvol(const QString &uuid, const int subvolid);

    /** @brief Returns true if the subvol represented by @p subvolid is mounted for @p uuid
     */
    static bool isMounted(const QString &uuid, const int subvolid);

    /**
     * @brief Checks if quotas are enables at @p mountpoint
     * @param mountpoint - The absolute path to a mountpoint to check for quota enablement on
     * @return true is quotas are enabled, false otherwise
     */
    static bool isQuotaEnabled(const QString &mountpoint);

    /** @brief Returns true if @p subvolume is a snapper snapshot
     *
     */
    static bool isSnapper(const QString &subvolume);

    /** @brief Returns true if @p subvolume is a timeshift snapshot
     *
     */
    static bool isTimeshift(const QString &subvolume) { return subvolume.contains("timeshift-btrfs"); }

    /** @brief Returns true if @p subvolume is a docker subvolume
     *
     */
    static bool isDocker(const QString &subvolume) { return subvolume.contains("docker/btrfs/subvolumes"); }

    /** @brief Returns a QStringList of UUIDs containing Btrfs filesystems
     */
    static const QStringList listFilesystems();

    /** @brief Returns a mountpoints for each Btrfs subvolume
     *
     *  Finds all mountpoints for each Btrfs subvolume and returns a sorted QStringList
     *  containing all the subvol names
     */
    static const QStringList listMountpoints();

    /** @brief Returns the btrfs subvolume list for a given volume
     *
     *  Returns a QMap where the key is subvolid and the data is subvolume name for @p uuid.  If no list is found,
     *  returns an empty list
     *
     */
    const QMap<int, Subvolume> listSubvolumes(const QString &uuid) const;

    /**
     * @brief Reads the qgroup data to populate subvol sizes
     * @param uuid - The UUID to read the qgroup data from
     */
    void loadQgroups(const QString &uuid);

    /** @brief Reloads the btrfs subvolume list for a given volume
     *
     *  Updates the subvol list in m_btrfsVolumes for @p uuid
     *
     */
    void loadSubvols(const QString &uuid);

    /** @brief Reloads the btrfs metadata
     *
     *  Populates m_btrfsVolumes with data from all the btrfs filesystems
     *
     */
    void loadVolumes();

    /** @brief Mounts the root of a given Btrfs volume
     *
     *  Finds the mountpoint of a btrfs volume specified by @p uuid.  If it isn't mounted, it will first mount it.
     *  returns the mountpoint or a default constructed string if it fails
     *
     */
    static const QString mountRoot(const QString &uuid);

    /** @brief Renames a btrfs subvolume from @p source to @p target
     *
     *  Returns true on success, false otherwise
     */
    static bool renameSubvolume(const QString &source, const QString &target);

    /**
     * @brief Checks the scrub status of a given subvolume.
     * @param mountpoint - A Qstring that represents the mountpoint to check for a btrfs scrub on
     * @return Qstring that contains the output from the btrfs scrub command.
     */
    const QString scrubStatus(const QString &mountpoint) const;

    /**
     * @brief Enables or disables btrfs qgroup support on @p mountpoint
     * @param mountpoint - An absolute path to the mountpoint that qgroups will be enabled on
     * @param enable - A boolean that enables qgroups when true and disables them when false
     */
    static void setQgroupEnabled(const QString &mountpoint, bool enable);

    /** @brief Returns the subvolid for a given subvol
     *
     *  Returns the subvolid of the subvol named by @p subvol on for @p uuid.  If @p subvol is not found,
     *  it returns 0
     */
    const int subvolId(const QString &uuid, const QString &subvolName) const;

    /**
     * @brief Returns a pointer to the subvol model
     */
    SubvolModel *subvolModel() { return &m_subvolModel; }

    /**
     * @brief Returns the name of the subvol with id @p subvolId
     * @param uuid - A QString that represents the UUID of the filesystem to match @p subvolId to
     * @param subvolId - An int with the ID of the subvolume to find the name for
     * @return The path of the subvolume relative to the root of the filesystem or a default constructed QString if not found
     */
    const QString subvolName(const QString &uuid, const int subvolId) const;

    /**
     * @brief Finds the ID of the subvolume that is the parent of @p subvolId
     * @param uuid - A QString that represents the UUID of the filesystem to match @p subvolId to
     * @param subvolId - An int with the ID of the subvolume to find the parent of
     * @return An int with parent ID or 0 if the subvolId is not found
     */
    const int subvolParent(const QString &uuid, const int subvolId) const;

    /**
     * @brief Performs a balance operation on top level subvolume for device.
     * @param uuid - A QString that represents the UUID of the filesystem to identify top level mountpoint
     */
    void startBalanceRoot(const QString &uuid);

    /**
     * @brief Performs a scrub operation on root subvolume for device.
     * @param uuid - A QString that represents the UUID of the filesystem to identify top level mountpoint
     */
    void startScrubRoot(const QString &uuid);

    /**
     * @brief Stops a balance operation on root subvolume for device.
     * @param uuid - A QString that represents the UUID of the filesystem to identify top level mountpoint
     */
    void stopBalanceRoot(const QString &uuid);

    /**
     * @brief Stops a scrub operation on root subvolume for device.
     * @param uuid - A QString that represents the UUID of the filesystem to identify top level mountpoint
     */
    void stopScrubRoot(const QString &uuid);

  private:
    SubvolModel m_subvolModel;
    // Holds the subvol sizes, the outer key is UUID, the inner key is subvolId, element 0 is size and element 1 is exclusive
    QMap<QString, QMap<int, QVector<long>>> m_subvolSize;
    // A map of BtrfsMeta.  The key is UUID
    QMap<QString, BtrfsMeta> m_volumes;

    /**
     * @brief Validates the UUID passed in actually exists and is accessible still.
     * @param uuid - The UUID of the filesystem to validate
     * @return bool - True if the UUID is a mounted Btrfs filesystem
     */
    bool isUuidLoaded(const QString &uuid);

  signals:
};

#endif // BTRFS_H
