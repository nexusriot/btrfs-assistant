#ifndef BTRFS_H
#define BTRFS_H

#include <QDateTime>
#include <QMap>
#include <QObject>

#include <optional>

constexpr uint64_t BTRFS_ROOT_ID = 5;

struct RestoreResult {
    bool isSuccess = false;
    QString failureMessage;
    QString backupSubvolName;
};

struct SubvolResult {
    QString name;
    bool success = false;
};

struct Subvolume {
    uint64_t id = 0;
    uint64_t parentId = 0;
    QString subvolName;
    QString uuid;
    QString parentUuid;
    QString receivedUuid;
    uint64_t generation = 0;
    QString filesystemUuid;
    uint64_t size = 0;
    uint64_t exclusive = 0;
    uint64_t flags = 0;
    QDateTime createdAt;

    /** @brief Returns true if this instance doesn't represent any subvolume */
    bool isEmpty() const;

    /** @brief Returns if the subvolume is read-only. */
    bool isReadOnly() const;

    /** @brief Returns if the subvolume is a snapshot. */
    bool isSnapshot() const;

    /** @brief Returns if the subvolume was received. */
    bool isReceived() const;
};

using SubvolumeMap = QMap<uint64_t, Subvolume>;

struct BtrfsFilesystem {
    bool isPopulated = false;
    uint64_t totalSize = 0;
    uint64_t allocatedSize = 0;
    uint64_t usedSize = 0;
    uint64_t freeSize = 0;
    uint64_t dataSize = 0;
    uint64_t dataUsed = 0;
    uint64_t metaSize = 0;
    uint64_t metaUsed = 0;
    uint64_t sysSize = 0;
    uint64_t sysUsed = 0;
    SubvolumeMap subvolumes;
};

/**
 * @brief The Btrfs service class handles all btrfs device functionality.
 */
class Btrfs : public QObject {
    Q_OBJECT

  public:
    explicit Btrfs(QObject *parent = nullptr);

    ~Btrfs();

    /**
     * @brief Checks the balance status of a given subvolume.
     * @param mountpoint - A Qstring that represents the mountpoint to check for a btrfs balance on
     * @return Qstring that contains the output from the btrfs balance command.
     */
    QString balanceStatus(const QString &mountpoint) const;

    /** @brief Returns the data for the Btrfs volume identified by @p UUID
     *
     * Returns a BtrfsFilesystem struct that represents the btrfs filesystem identified by @p UUID.
     * If no data is found for the given UUID, it returns a default contructed value
     *
     */
    BtrfsFilesystem filesystem(const QString &uuid) const;

    /** @brief Returns the direct children for a given subvolume
     *
     *  Finds all children which are a single generation below the parent subvol identified by
     *  @p subvolid on volume @p uuid.
     *
     *  Returns a QStringList containing the subvol names/paths of all the children subvols
     *
     */
    QStringList children(const uint64_t subvolid, const QString &uuid) const;

    /**
     * @brief Creates a btrfs snapshot
     * @param source - The absolute path to the source subvolume
     * @param dest - The absolute path where the snapshot will be created
     * @param readOnly - Whether the snapshot should be read-only
     * @return True on success, false otherwise
     */
    static bool createSnapshot(const QString &source, const QString &dest, bool readOnly);

    /**
     * @brief Creates a btrfs snapshot
     * @param fileSystemUuid The uuid of the filesystem containing the subvolume
     * @param sourceSubvolId - The id of the subvolume to snapshot
     * @param dest - The absolute path where the snapshot will be created
     * @param readOnly - Whether the snapshot should be read-only
     * @return Subvolume on success, or empty instance on failure.
     */
    std::optional<Subvolume> createSnapshot(const QString &fileSystemUuid, uint64_t sourceSubvolId, const QString &dest, bool readOnly);

    /** @brief Deletes a given subvolume
     *
     *  Deletes the subvol represented by @p subvolid on @p uuid.
     *
     *  Returns true if successful and false if it fails for any reason.
     *
     */
    bool deleteSubvol(const QString &uuid, const uint64_t subvolid);

    /**
     * @brief Finds a single mountpoint for a btrfs filesystem
     * @param uuid The uuid of the filesystem to find the mountpoint for
     * @return The absolute path to the mountpoint. If no mountpoint is found, a default constructed value
     */
    static QString findAnyMountpoint(const QString &uuid);

    /** @brief Returns true if the subvol represented by @p subvolid is mounted for @p uuid
     */
    static bool isMounted(const QString &uuid, const uint64_t subvolid);

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

    /**
     * @brief Returns boolean if subvolume is a Btrfs Assistant backup.
     * @param subvolPath - The QString path to the subvolume
     * @return true if subvolume is a Btrfs Assistant backup subvolume based on naming.
     */
    bool isSubvolumeBackup(QString subvolPath);

    /** @brief Returns true if @p subvolume is a timeshift snapshot
     *
     */
    static bool isTimeshift(const QString &subvolume) { return subvolume.contains("timeshift-btrfs"); }

    /** @brief Returns true if @p subvolume is a docker or podman subvolume
     *
     */
    static bool isContainer(const QString &subvolume) { return subvolume.contains("/btrfs/subvolumes"); }

    /** @brief Returns a QStringList of UUIDs containing Btrfs filesystems
     */
    static QStringList listFilesystems();

    /** @brief Returns a mountpoints for each Btrfs subvolume
     *
     *  Finds all mountpoints for each Btrfs subvolume and returns a sorted QStringList
     *  containing all the subvol names
     */
    static QStringList listMountpoints();

    /** @brief Returns the btrfs subvolume list for a given volume
     *
     *  Returns a QMap where the key is subvolid and the data is subvolume name for @p uuid.  If no list is found,
     *  returns an empty list
     *
     */
    SubvolumeMap listSubvolumes(const QString &uuid) const;

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
    QString mountRoot(const QString &uuid);

    /** @brief Renames a btrfs subvolume from @p source to @p target
     *
     *  Returns true on success, false otherwise
     */
    static bool renameSubvolume(const QString &source, const QString &target);

    /**
     * @brief Restores the source subvolume over the target
     * @param uuid - A QString that holds the UUID of the filesystem you want to perform the restore in
     * @param sourceId - An uint64_t that is the subvolid of the source subvolume
     * @param targetId - An uint64_t that is the subvolid of the target subvolume
     * @return A RestoreResult struct that contains the results of the operation
     */
    RestoreResult restoreSubvol(const QString &uuid, const uint64_t sourceId, const uint64_t targetId,
                                const QString &customName = QString());

    /**
     * @brief Checks the scrub status of a given subvolume.
     * @param mountpoint - A Qstring that represents the mountpoint to check for a btrfs scrub on
     * @return Qstring that contains the output from the btrfs scrub command.
     */
    QString scrubStatus(const QString &mountpoint) const;

    /**
     * @brief Enables or disables btrfs qgroup support on @p mountpoint
     * @param mountpoint - An absolute path to the mountpoint that qgroups will be enabled on
     * @param enable - A boolean that enables qgroups when true and disables them when false
     */
    static void setQgroupEnabled(const QString &mountpoint, bool enable);

    /**
     * @brief Return whether a given path is a subvolume.
     * @param path - An absolute path to a subvolume
     * @return True if path is a subvolume.
     */
    static bool isSubvolume(const QString &path);

    /** @brief Returns the subvolid for a given subvol
     *
     *  Returns the subvolid of the subvol named by @p subvol on for @p uuid.  If @p subvol is not found,
     *  it returns 0
     */
    uint64_t subvolId(const QString &uuid, const QString &subvolName);

    /**
     * @brief Returns the name of the subvol with id @p subvolId
     * @param uuid - A QString that represents the UUID of the filesystem to match @p subvolId to
     * @param subvolId - An uint64_t with the ID of the subvolume to find the name for
     * @return A struct containing the path of the subvolume relative to the root of the filesystem and a success flag
     */
    SubvolResult subvolumeName(const QString &uuid, const uint64_t subvolId) const;

    /**
     * @brief Returns the name of the subvol at @p path
     * @param path - An absolute path to a subvolume
     * @return A struct containing the path of the subvolume relative to the root of the filesystem and a success flag
     */
    static SubvolResult subvolumeName(const QString &path);

    /**
     * @brief Finds the ID of the subvolume that is the parent of @p subvolId
     * @param uuid - A QString that represents the UUID of the filesystem to match @p subvolId to
     * @param subvolId - An uint64_t with the ID of the subvolume to find the parent of
     * @return An uint64_t with parent ID or 0 if the subvolId is not found
     */
    uint64_t subvolParent(const QString &uuid, const uint64_t subvolId) const;

    /**
     * @brief Finds the ID of the subvolume that is the parent of the subvol at @p path
     * @param path - An absolute path to a subvolume
     * @return An uint64_t with parent ID or 0 if the subvolId is not found
     */
    uint64_t subvolParent(const QString &path) const;

    /**
     * @brief Set whether a subvolume is read-only
     * @param path - An absolute path to a subvolume
     * @param readOnly - New value of read-only flag
     * @return Returns true if successful and false if it fails for any reason.
     */
    static bool setSubvolumeReadOnly(const QString &path, bool readOnly);

    /**
     * @brief Set whether a subvolume is read-only
     * @param uuid - An uuid of the filesystem containing the subvolume
     * @param subvolId - An if of the subvolume to modify
     * @param readOnly - New value of read-only flag
     * @return Returns true if successful and false if it fails for any reason.
     */
    bool setSubvolumeReadOnly(const QString &uuid, uint64_t subvolId, bool readOnly);

    /**
     * @brief Set whether a subvolume is read-only
     * @param subvol - A subvolume to modify
     * @param readOnly - New value of read-only flag
     * @return Returns true if successful and false if it fails for any reason.
     */
    bool setSubvolumeReadOnly(const Subvolume &subvol, bool readOnly);

    /**
     * @brief Get whether a subvolume is read-only
     * @param readOnly - New value of read-only flag
     * @return Returns true if the subvolume is read-only or false if not read-only or an error occured.
     */
    static bool isSubvolumeReadOnly(const QString &path);

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

    /**
     * @brief Provides access to the full metadata for all btrfs volumes
     * @return A const reference to the volume data
     */
    const QMap<QString, BtrfsFilesystem> &filesystems() { return m_filesystems; }

  private:
    // A map of BtrfsFilesystem.  The key is UUID
    QMap<QString, BtrfsFilesystem> m_filesystems;
    QVector<QString> m_tempMountpoints;

    /**
     * @brief Validates the UUID passed in actually exists and is accessible still.
     * @param uuid - The UUID of the filesystem to validate
     * @return bool - True if the UUID is a mounted Btrfs filesystem
     */
    bool isUuidLoaded(const QString &uuid);

    /**
     * @brief Unmounts any filesystems that were mounted by the application
     */
    void unmountFilesystems();
};

#endif // BTRFS_H
