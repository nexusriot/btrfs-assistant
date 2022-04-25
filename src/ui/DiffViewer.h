#ifndef DIFFVIEWER_H
#define DIFFVIEWER_H

#include "util/Snapper.h"
#include "ui_DiffViewer.h"

enum DiffColumn { num, dateTime, rootPath, filePath };

namespace Ui {
class DiffViewer;
}

/**
 * @brief The DiffViewer class handles the Diff Viewer dialog
 */
class DiffViewer : public QDialog {
    Q_OBJECT

  public:
    DiffViewer(Snapper *snapper, const QString &rootPath, const QString &filePath, const QString &uuid, QWidget *parent = nullptr);

    ~DiffViewer();

  private slots:
    void on_pushButton_close_clicked() { this->close(); }
    void on_pushButton_restore_clicked();

    /**
     * @brief When a selection changes in the table, update the diff textEdit widget
     */
    void on_tableWidget_snapshotList_itemSelectionChanged();

  private:
    Ui::DiffViewer *m_ui;
    Snapper *m_snapper;
    // The path to the file on the current system that will be restored to and diffed against
    QString m_targetPath;
    // This a convenience pointer to m_ui->tableWidget_snapshotList to improve readability
    QTableWidget *m_twSnapshot;
    QString m_uuid;

    /**
     * @brief Finds all the snapshots that contain the file and populated the grid
     * @param rootPath - The absolute path to the snapshot
     * @param filePath - The absolute path to the file selected within the snapshot
     */
    void LoadSnapshots(const QString &rootPath, const QString &filePath);
};

#endif // DIFFVIEWER_H
