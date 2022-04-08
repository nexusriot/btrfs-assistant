#ifndef SUBVOLMODEL_H
#define SUBVOLMODEL_H

#include <QAbstractTableModel>
#include <QMutex>

struct Subvolume {
    int parentId = 0;
    int subvolId;
    QString subvolName;
    QString uuid;
    long size = 0;
    long exclusive = 0;
};
struct BtrfsMeta;

enum SubvolHeader { subvolId, parentId, subvolName, uuid, size, exclusive };

class SubvolModel : public QAbstractTableModel {
    Q_OBJECT

  public:
    explicit SubvolModel(QObject *parent = nullptr) : QAbstractTableModel(parent) {}

    // Basic model functions
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    /**
     * @brief Populates the model using @p subvolData and @p subvolSize
     * @param subvolData - A map of Subvolumes with subvolId as the key
     * @param m_subvolSize - A map of QVectors where subvolId is the key and size is at index 0 and exclusize size at index 1
     */
    void loadModel(const QMap<QString, BtrfsMeta> &volumeData, const QMap<QString, QMap<int, QVector<long>>> &subvolSize);

    /**
     * @brief Sets the boolean used to determine whether to include snapshots in the subvolume model.
     * @param includeSnapshots Bool value to set.
     */
    void setIncludeSnapshots(bool includeSnapshots) { m_includeSnapshots = includeSnapshots; }

  private:
    int m_columns = 6;
    // Holds the data for the model
    QVector<Subvolume> m_data;
    bool m_includeSnapshots = false;
    // Used to ensure only one model update runs at a time
    QMutex m_updateMutex;
};

#endif // SUBVOLMODEL_H
