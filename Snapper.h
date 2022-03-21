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
    QString subvolid;
    QString time;
    QString desc;
    QString uuid;
};


class Snapper : public QObject
{
    Q_OBJECT
public:
    explicit Snapper(Btrfs *btrfs, QObject *parent = nullptr);

    const QString config(const QString &mountpoint) { return m_configs.key(mountpoint); }

    /**
     * @brief Finds all available Snapper configs
     *
     * @return A QStringList of config names
     *
     */
    const QStringList configs() { return m_configs.keys(); }

    /**
     * @brief Loads all the Snapper meta data from disk
     *
     * Populates m_configs and m_snapshots from the results of the snapper command
     *
     */
    void reload();

    void reloadSubvols();
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
    QMap<QString, QString> m_configs;
    QMap<QString, QVector<SnapperSnapshots>> m_snapshots;
    QMap<QString, QVector<SnapperSubvolume>> m_subvols;


signals:

};

#endif // SNAPPER_H
