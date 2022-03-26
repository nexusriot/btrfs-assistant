#ifndef SNAPPER_H
#define SNAPPER_H

#include <QDir>
#include <QFile>
#include <QObject>
#include <QRegularExpression>

#include "Btrfs.h"
#include "System.h"

struct SnapperSnapshots {
    int number;
    QString time;
    QString desc;
    QString type;
};

struct SnapperSubvolume {
    QString subvol;
    int subvolid;
    int snapshotNum;
    QString time;
    QString desc;
    QString uuid;
    QString type;
};

class Snapper : public QObject {
    Q_OBJECT
  public:
    explicit Snapper(Btrfs *btrfs, QString snapperCommand, const QMap<QString, QString> &subvolMap, QObject *parent = nullptr);

    const QMap<QString, QString> config(const QString &name);

    /**
     * @brief Finds all available Snapper configs
     *
     * @return A QStringList of config names
     *
     */
    const QStringList configs() { return m_configs.keys(); }

    void createConfig(const QString &name, const QString &path) const { runSnapper("create-config " + path, name); }

    void createSnapshot(const QString &name) const { runSnapper("create -d 'Manual Snapshot'", name); }

    /**
     * @brief Reads the list of subvols to create mapping between the snapshot subvolume and the source subvolume
     */
    void createSubvolMap();

    void deleteConfig(const QString &name) const { runSnapper("delete-config", name); }

    void deleteSnapshot(const QString &name, const int num) const { runSnapper("delete " + QString::number(num), name); }

    /**
     * @brief Finds the subvolume that is used by snapper to hold the snapshots for @p subvol
     * @param subvol - A Qstring containg the path of the subvolume relative to the filesystem root
     * @return A QString containing the path to the snapshot subvolume relative to the root or an empty string
     */
    static const QString findSnapshotSubvolume(const QString &subvol);

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

    void setConfig(const QString &name, const QMap<QString, QString> configMap);

    /**
     * @brief Returns a list of metadata for each snapshot in @p config
     * @param config - The name of the Snapper config to list
     * @return A QVector of SnapperShots for each snapshot
     */
    const QVector<SnapperSnapshots> snapshots(const QString &config);

    const QStringList subvolKeys() { return m_subvols.keys(); }

    /**
     * @brief Returns a list of metadata for each subvol associated with @p config
     * @param config - The name of the Snapper config to list
     * @return A QVector of SnapperSubvolumes for each subvol
     */
    const QVector<SnapperSubvolume> subvols(const QString &config);

  private:
    Btrfs *m_btrfs;
    // The outer map is keyed with the config name, the inner map is the name, value pairs in the named config
    QMap<QString, QMap<QString, QString>> m_configs;

    // The absolute path to the snapper command
    QString m_snapperCommand;

    // A map of snapper snapshots.  The key is the snapper config name
    QMap<QString, QVector<SnapperSnapshots>> m_snapshots;

    // A map of btrfs subvolumes that hold snapper snapshots.  The key is the source subvol.
    QMap<QString, QVector<SnapperSubvolume>> m_subvols;

    // Maps the subvolumes to their snapshot directories.  key is the snapshot subvol path
    QMap<QString, QString> m_subvolMap;

    const QStringList runSnapper(const QString &command, const QString &name = "") const;

  signals:
};

#endif // SNAPPER_H
