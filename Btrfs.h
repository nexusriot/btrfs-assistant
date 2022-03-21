#ifndef BTRFS_H
#define BTRFS_H

#include <QMap>
#include <QObject>

struct Subvolume {
    int parentId = 0;
    QString subvolName;
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

class Btrfs : public QObject {
    Q_OBJECT
  public:
    explicit Btrfs(QObject *parent = nullptr);

    /** @brief Returns the data for the Btrfs volume identified by @p UUID
     *
     * Returns a BtrfsMeta struct that represents the btrfs filesystem identified by @p UUID.
     * If no data is found for the given UUID, it returns a default contructed value
     *
     */
    const BtrfsMeta btrfsVolume(const QString &uuid);

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

    /** @brief Returns true if @p subvolume is a timeshift snapshot
     *
     */
    static bool isTimeshift(const QString &subvolume) { return subvolume.contains("timeshift-btrfs"); }

    /** @brief Returns true if @p subvolume is a snapper snapshot
     *
     */
    static bool isSnapper(const QString &subvolume) { return subvolume.contains(".snapshots") && !subvolume.endsWith(".snapshots"); }

    /** @brief Returns the name of the subvol mounted at /
     *
     * Returns QString containing the name of the subvol.  If there is not Btrfs subvol mounted at /,
     * a default constructed QString is returned
     *
     */
    static const QString findRootSubvol();

    /** @brief Returns true if the subvol represented by @p subvolid is mounted for @p uuid
     */
    static bool isMounted(const QString &uuid, const int subvolid);

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
    const QMap<int, Subvolume> listSubvolumes(const QString &uuid);

    /** @brief Mounts the root of a given Btrfs volume
     *
     *  Finds the mountpoint of a btrfs volume specified by @p uuid.  If it isn't mounted, it will first mount it.
     *  returns the mountpoint or a default constructed string if it fails
     *
     */
    static const QString mountRoot(const QString &uuid);

    /** @brief Reloads the btrfs subvolume list for a given volume
     *
     *  Updates the subvol list in m_btrfsVolumes for @p uuid
     *
     */
    void reloadSubvols(const QString &uuid);

    /** @brief Reloads the btrfs metadata
     *
     *  Populates m_btrfsVolumes with data from all the btrfs filesystems
     *
     */
    void reloadVolumes();

    /** @brief Renames a btrfs subvolume from @p source to @p target
     *
     *  Returns true on success, false otherwise
     */
    static bool renameSubvolume(const QString &source, const QString &target);

    /** @brief Returns the subvolid for a given subvol
     *
     *  Returns the subvolid of the subvol named by @p subvol on for @p uuid.  If @p subvol is not found,
     *  it returns 0
     */
    const int subvolId(const QString &uuid, const QString &subvolName);

    const int subvolTopParent(const QString &uuid, const int subvolId);

  private:
    // A map BtrfsMeta.  The key is UUID
    QMap<QString, BtrfsMeta> m_volumes;

  signals:
};

#endif // BTRFS_H
