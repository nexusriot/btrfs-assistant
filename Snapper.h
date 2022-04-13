#ifndef SNAPPER_H
#define SNAPPER_H

#include <QObject>

#include "Btrfs.h"
#include "System.h"

#define DEFAULT_SNAP_PATH "/.snapshots"

struct SnapperResult {
    int exitCode;
    QStringList outputList;
};

struct SnapperSnapshots {
    int number;
    QDateTime time;
    QString desc;
    QString type;
};

struct SnapperSubvolume {
    QString subvol;
    int subvolid;
    int snapshotNum;
    QDateTime time;
    QString desc;
    QString uuid;
    QString type;
};

/**
 * @brief The Snapper service class that handles all the interaction with the snapper application.
 */
class Snapper : public QObject {
    Q_OBJECT
  public:
    Snapper(Btrfs *btrfs, QString snapperCommand, QObject *parent = nullptr);

    /**
     * @brief Gets the list of configuration settings for a given config
     * @param name - A QString that is the Snapper config name
     * @return A QMap of name, value pairs for each setting
     */
    const QMap<QString, QString> config(const QString &name);

    /**
     * @brief Finds all available Snapper configs
     *
     * @return A QStringList of config names
     *
     */
    const QStringList configs() { return m_configs.keys(); }

    /**
     * @brief Creates a new Snapper config
     * @param name - The name of the new config
     * @param path - The absolute path to the mountpoint of the subvolume that will be snapshotted by the config
     */
    SnapperResult createConfig(const QString &name, const QString &path) const { return runSnapper("create-config " + path, name); }

    /**
     * @brief Creates a new manual snapshot with the given description
     * @param name - The name of the Snapper config
     * @param description - A string holding the description to be saved
     */
    SnapperResult createSnapshot(const QString &name, const QString &desc) const { return runSnapper("create -d '" + desc + "'", name); }

    /**
     * @brief Reads the list of subvols to create mapping between the snapshot subvolume and the source subvolume
     */
    void createSubvolMap();

    /**
     * @brief Deletes the given snapper config
     * @param name - The name of the Snapper config to delete
     */
    SnapperResult deleteConfig(const QString &name) const { return runSnapper("delete-config", name); }

    /**
     * @brief Deletes a given Snapper snapshot
     * @param name - The name of the config that contains the snapshot to delete
     * @param num - The number of the snapshot to delete
     */
    SnapperResult deleteSnapshot(const QString &name, const int num) const { return runSnapper("delete " + QString::number(num), name); }

    /**
     * @brief Finds the subvolume that is used by snapper to hold the snapshots for @p subvol
     * @param subvol - A Qstring containg the path of the subvolume relative to the filesystem root
     * @return A QString containing the path to the snapshot subvolume relative to the root or an empty string
     */
    static const QString findSnapshotSubvolume(const QString &subvol);

    /**
     * @brief Finds the original path where a file in a snapshot should be restored to
     * @param snapshotPath - The absolute path to the root of the snapshot
     * @param filePath - The absolute path to the file in the snapshot
     * @param uuid - The UUID of the filesystem holding the snapshot
     * @return The absolute path to the file that is the target of the restore
     */
    const QString findTargetPath(const QString &snapshotPath, const QString &filePath, const QString &uuid);

    /**
     * @brief Finds where the snapshots of @p snapshotSubvol should be restored to
     * @param snapshotSubvol - The path to the snapshot subvolume relative to the root of the filesystem
     * @param uuid - The UUID of the btrfs filesystem
     * @return A QString that is the path to the target subvolume relative to the root of the filesystem
     */
    const QString findTargetSubvol(const QString &snapshotSubvol, const QString &uuid) const;

    /**
     * @brief Loads all the Snapper meta data from disk
     *
     * Populates m_configs and m_snapshots from the results of the snapper command
     *
     */
    void load();

    /**
     * @brief loads the data for a single Snapper config
     * @param name - A QString that holds the name of the config to load
     */
    void loadConfig(const QString &name);

    /**
     * @brief loads the Btrfs subvolumes that are Snapper snapshots
     */
    void loadSubvols();

    /**
     * @brief Reads the contents of a snapper metafile for a snapshot
     * @param filename - The absolute path to the meta file to read
     * @return A SnapperSnapshots struct with the values from the file
     */
    static SnapperSnapshots readSnapperMeta(const QString &filename);

    /**
     * @brief Restores a single file identified by @p filePath to it's original location
     * @param snapshotPath - An absolute to where the snapshot subvolume is currently mounted
     * @param filePath - An absolute path to the file to restore
     * @param uuid - The UUID of the filesystem the restore applies to
     * @return Return true on success and false otherwise
     */
    bool restoreFile(const QString &sourcePath, const QString &destPath) const;

    /**
     * @brief Restores the source subvolume over the target
     * @param uuid - A QString that holds the UUID of the filesystem you want to perform the restore in
     * @param sourceId - An int that is the subvolid of the source subvolume
     * @param targetId - An int that is the subvolid of the target subvolume
     * @return A RestoreResult struct that contains the results of the operation
     */
    const RestoreResult restoreSubvol(const QString &uuid, const int sourceId, const int targetId) const;

    /**
     * @brief Updates the settings for a given Snapper config described by @p name
     * @param name - The name of the Snapper config to be updated
     * @param configMap - A QMap of name/value pairs that holds the settings to update
     */
    SnapperResult setConfig(const QString &name, const QMap<QString, QString> configMap);

    /**
     * @brief Returns a list of metadata for each snapshot in @p config
     * @param config - The name of the Snapper config to list
     * @return A QVector of SnapperShots for each snapshot
     */
    const QVector<SnapperSnapshots> snapshots(const QString &config);

    /**
     * @brief Gets the list of targets where a Snapper snapshot can be restored to
     * @return A QStringList that is a list of paths relative to the root of the Btrfs filesystem
     */
    const QStringList subvolKeys() { return m_subvols.keys(); }

    /**
     * @brief Returns a list of metadata for each subvol associated with @p config
     * @param config - The name of the Snapper config to list
     * @return A QVector of SnapperSubvolumes for each subvol
     */
    const QVector<SnapperSubvolume> subvols(const QString &config);

  private:
    Btrfs *m_btrfs;
    // The outer map is keyed with the config name, the inner map is the name, value pairs of the configuration settings
    QMap<QString, QMap<QString, QString>> m_configs;

    // The absolute path to the snapper command
    QString m_snapperCommand;

    // A map of snapper snapshots.  The key is the snapper config name
    QMap<QString, QVector<SnapperSnapshots>> m_snapshots;

    // A map of btrfs subvolumes that hold snapper snapshots.  The key is the target subvol.
    QMap<QString, QVector<SnapperSubvolume>> m_subvols;

    // Maps the subvolumes to their snapshot directories.  key is the snapshot subvol path
    QMap<QString, QString> *m_subvolMap;

    const SnapperResult runSnapper(const QString &command, const QString &name = "") const;

  signals:
};

#endif // SNAPPER_H
