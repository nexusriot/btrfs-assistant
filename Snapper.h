#ifndef SNAPPER_H
#define SNAPPER_H

#include <QDir>
#include <QFile>
#include <QObject>

#include "Btrfs.h"
#include "System.h"

struct SnapperSnapshots {
    int number;
    QString time;
    QString desc;
};

struct SnapperSubvolume {
    QString subvol;
    int subvolid;
    int snapshotNum;
    QString time;
    QString desc;
    QString uuid;
};

class Snapper : public QObject {
    Q_OBJECT
  public:
    explicit Snapper(Btrfs *btrfs, QString snapperCommand, QObject *parent = nullptr);

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

    void deleteConfig(const QString &name) const { runSnapper("delete-config", name); }

    void deleteSnapshot(const QString &name, const int num) const { runSnapper("delete " + QString::number(num), name); }

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
    QString m_snapperCommand;
    QMap<QString, QVector<SnapperSnapshots>> m_snapshots;
    QMap<QString, QVector<SnapperSubvolume>> m_subvols;

    const QStringList runSnapper(const QString &command, const QString &name = "") const;

  signals:
};

#endif // SNAPPER_H
