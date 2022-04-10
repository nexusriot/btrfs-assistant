#ifndef SUBVOLMODEL_H
#define SUBVOLMODEL_H

#include <QAbstractTableModel>
#include <QMutex>
#include <QSortFilterProxyModel>

struct Subvolume {
    int parentId = 0;
    int subvolId = 0;
    QString subvolName;
    QString uuid;
    long size = 0;
    long exclusive = 0;
};
struct BtrfsMeta;

class SubvolumeModel : public QAbstractTableModel {
    Q_OBJECT

  public:
    enum Column { Id, ParentId, Name, Uuid, Size, ExclusiveSize, ColumnCount };

    enum Role { Sort = Qt::UserRole };

    explicit SubvolumeModel(QObject *parent = nullptr) : QAbstractTableModel(parent) {}

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

  private:
    // Holds the data for the model
    QVector<Subvolume> m_data;
    // Used to ensure only one model update runs at a time
    QMutex m_updateMutex;
};

class SubvolumeFilterModel : public QSortFilterProxyModel {
    Q_OBJECT
  public:
    SubvolumeFilterModel(QObject *parent = nullptr);

    bool includeSnapshots() const;
    bool includeDocker() const;

  public slots:
    void setIncludeSnapshots(bool includeSnapshots);
    void setIncludeDocker(bool includeDocker);

  protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

  private:
    bool m_includeSnapshots = false;
    bool m_includeDocker = false;
};

#endif // SUBVOLMODEL_H
