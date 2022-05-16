#include "model/SubvolModel.h"
#include "util/System.h"

QVariant SubvolumeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) {
        return QAbstractTableModel::headerData(section, orientation, role);
    }

    if (orientation == Qt::Vertical) {
        return section;
    }

    switch (section) {
    case Column::ParentId:
        return tr("Parent ID");
    case Column::Id:
        return tr("Subvol ID");
    case Column::Name:
        return tr("Subvolume");
    case Column::Uuid:
        return tr("UUID");
    case Column::ParentUuid:
        return tr("Parent UUID");
    case Column::ReceivedUuid:
        return tr("Received UUID");
    case Column::CreatedAt:
        return tr("Created");
    case Column::Generation:
        return tr("Generation");
    case Column::ReadOnly:
        return tr("Read-only");
    case Column::Size:
        return tr("Size");
    case Column::FilesystemUuid:
        return tr("Filesystem");
    case Column::ExclusiveSize:
        return tr("Exclusive");
    }

    return QString();
}

int SubvolumeModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return m_data.count();
}

int SubvolumeModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return ColumnCount;
}

QVariant SubvolumeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.column() >= ColumnCount || index.row() >= m_data.count()) {
        return {};
    }

    if (role == Qt::TextAlignmentRole) {
        switch (static_cast<Column>(index.column())) {
        case Column::Id:
        case Column::ParentId:
        case Column::Name:
        case Column::Uuid:
        case Column::ParentUuid:
        case Column::ReceivedUuid:
        case Column::FilesystemUuid:
            return {};
        case Column::Generation:
        case Column::Size:
        case Column::ExclusiveSize:
            return static_cast<int>(Qt::AlignRight | Qt::AlignVCenter);
        case Column::CreatedAt:
        case Column::ReadOnly:
            return static_cast<int>(Qt::AlignCenter);
        case Column::ColumnCount:
            return {};
        }
    }

    if (role != Qt::DisplayRole && role != Role::Sort && role != Qt::TextAlignmentRole) {
        return {};
    }

    const Subvolume &subvol = m_data[index.row()];
    switch (index.column()) {
    case Column::ParentId:
        return QVariant::fromValue(subvol.parentId);
    case Column::Id:
        return QVariant::fromValue(subvol.id);
    case Column::Name:
        return subvol.subvolName;
    case Column::Uuid:
        return subvol.uuid;
    case Column::ParentUuid:
        return subvol.parentUuid;
    case Column::ReceivedUuid:
        return subvol.receivedUuid;
    case Column::CreatedAt:
        return subvol.createdAt;
    case Column::Generation:
        return QVariant::fromValue<qulonglong>(subvol.generation);
    case Column::ReadOnly:
        // We want an empty item instead of 'false' value
        return subvol.isReadOnly() ? QVariant(true) : QVariant();
    case Column::FilesystemUuid:
        return subvol.filesystemUuid;
    case Column::Size:
        if (role == Qt::DisplayRole) {
            return System::toHumanReadable(subvol.size);
        } else {
            return QVariant::fromValue<qulonglong>(subvol.size);
        }
    case Column::ExclusiveSize:
        if (role == Qt::DisplayRole) {
            return System::toHumanReadable(subvol.exclusive);
        } else {
            return QVariant::fromValue<qulonglong>(subvol.exclusive);
        }
    }

    return QVariant();
}

const Subvolume &SubvolumeModel::subvolume(int row) const { return m_data[row]; }

void SubvolumeModel::load(const QMap<QString, BtrfsFilesystem> &filesystems)
{
    // Ensure that multiple threads don't try to update the model at the same time
    QMutexLocker lock(&m_updateMutex);

    beginResetModel();
    m_data.clear();

    const QList<QString> filesystemUuids = filesystems.keys();

    // Extract all the subvolumes
    for (const QString &uuid : filesystemUuids) {
        for (const Subvolume &subvol : filesystems.value(uuid).subvolumes) {
            if (subvol.id != BTRFS_ROOT_ID && subvol.id != 0) {
                m_data.append(subvol);
            }
        }
    }

    endResetModel();
}

void SubvolumeModel::addSubvolume(const Subvolume &subvol)
{
    beginInsertRows(QModelIndex(), m_data.size(), m_data.size());
    m_data.append(subvol);
    endInsertRows();
}

void SubvolumeModel::updateSubvolume(const Subvolume &subvol)
{
    for (int i = 0; i < m_data.size(); ++i) {
        Subvolume &existing = m_data[i];
        if (subvol.id == existing.id && subvol.filesystemUuid == existing.filesystemUuid) {
            existing = subvol;
            QModelIndex leftIdx = index(i, 0);
            QModelIndex rightIdx = index(i, ColumnCount - 1);
            emit dataChanged(leftIdx, rightIdx);
            break;
        }
    }
}

SubvolumeFilterModel::SubvolumeFilterModel(QObject *parent) : QSortFilterProxyModel(parent)
{
    setSortRole(static_cast<int>(SubvolumeModel::Role::Sort));
    setFilterCaseSensitivity(Qt::CaseInsensitive);
    setFilterKeyColumn(SubvolumeModel::Column::Name);
}

bool SubvolumeFilterModel::includeSnapshots() const { return m_includeSnapshots; }

bool SubvolumeFilterModel::includeContainer() const { return m_includeContainer; }

void SubvolumeFilterModel::setIncludeSnapshots(bool includeSnapshots)
{
    if (m_includeSnapshots != includeSnapshots) {
        m_includeSnapshots = includeSnapshots;
        invalidateFilter();
    }
}

void SubvolumeFilterModel::setIncludeContainer(bool includeContainer)
{
    if (m_includeContainer != includeContainer) {
        m_includeContainer = includeContainer;
        invalidateFilter();
    }
}

bool SubvolumeFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex nameIdx = sourceModel()->index(sourceRow, static_cast<int>(SubvolumeModel::Column::Name), sourceParent);
    const QString &name = sourceModel()->data(nameIdx).toString();

    if (!m_includeSnapshots && (Btrfs::isSnapper(name) || Btrfs::isTimeshift(name))) {
        return false;
    }
    if (!m_includeContainer && Btrfs::isContainer(name)) {
        return false;
    }

    return QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);
}
