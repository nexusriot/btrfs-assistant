#include "SubvolModel.h"
#include "Btrfs.h"
#include "System.h"

QVariant SubvolModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole) {
        return QAbstractTableModel::headerData(section, orientation, role);
    }

    if (orientation == Qt::Vertical) {
        return section;
    }

    switch (section) {
    case SubvolHeader::parentId:
        return tr("Parent ID");
    case SubvolHeader::subvolId:
        return tr("Subvol ID");
    case SubvolHeader::subvolName:
        return tr("Subvolume");
    case SubvolHeader::size:
        return tr("Size");
    case SubvolHeader::uuid:
        return tr("UUID");
    case SubvolHeader::exclusive:
        return tr("Exclusive");
    }

    return QString();
}

int SubvolModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid())
        return 0;

    return m_data.count();
}

int SubvolModel::columnCount(const QModelIndex &parent) const {
    if (parent.isValid())
        return 0;

    return m_columns;
}

QVariant SubvolModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    const QVector<Subvolume> *data;

    // Ensure the data is in range
    if (index.column() >= m_columns || index.row() >= m_data.count()) {
        return QVariant();
    }

    switch (index.column()) {
    case SubvolHeader::parentId:
        return m_data.at(index.row()).parentId;
    case SubvolHeader::subvolId:
        return m_data.at(index.row()).subvolId;
    case SubvolHeader::subvolName:
        return m_data.at(index.row()).subvolName;
    case SubvolHeader::size:
        return System::toHumanReadable(m_data.at(index.row()).size);
    case SubvolHeader::exclusive:
        return System::toHumanReadable(m_data.at(index.row()).exclusive);
    case SubvolHeader::uuid:
        return m_data.at(index.row()).uuid;
    }

    return QVariant();
}

void SubvolModel::loadModel(const QMap<int, Subvolume> &subvolData, const QMap<int, QVector<long>> &subvolSize) {
    // Ensure that multiple threads don't try to update the model at the same time
    QMutexLocker lock(&m_updateMutex);

    beginResetModel();
    m_data.clear();
    const QList<int> keys = subvolData.keys();

    for (const int key : keys) {
        if (m_includeSnapshots || !(Btrfs::isSnapper(subvolData[key].subvolName) || Btrfs::isTimeshift(subvolData[key].subvolName))) {
            Subvolume subvol = subvolData[key];
            if (subvolSize.contains(key) && subvolSize[key].count() == 2) {
                subvol.size = subvolSize[key][0];
                subvol.exclusive = subvolSize[key][1];
            }
            m_data.append(subvol);
        }
    }

    std::sort(m_data.begin(), m_data.end(), [](const Subvolume &a, const Subvolume &b) -> bool { return a.subvolName < b.subvolName; });
    endResetModel();
}
