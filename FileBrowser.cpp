#include "FileBrowser.h"
#include "DiffViewer.h"
#include "ui_FileBrowser.h"

#include <QDir>
#include <QMessageBox>

FileBrowser::FileBrowser(Snapper *snapper, const QString &rootPath, const QString &uuid, QWidget *parent)
    : QDialog(parent), m_ui(new Ui::FileBrowser), m_uuid(uuid), m_rootPath(rootPath), m_snapper(snapper) {
    m_ui->setupUi(this);

    m_treeView = m_ui->treeView_file;

    // Setup the file browser tree view
    m_fileModel = new QFileSystemModel;
    m_fileModel->setRootPath(rootPath);
    m_fileModel->setFilter(QDir::Hidden | QDir::AllEntries | QDir::NoDotAndDotDot);
    m_treeView->setModel(m_fileModel);
    m_treeView->hideColumn(1);
    m_treeView->hideColumn(2);
    m_treeView->setRootIndex(m_fileModel->index(rootPath));
    m_treeView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);

    this->setWindowTitle(tr("Snapshot File Viewer"));
}

FileBrowser::~FileBrowser() { delete m_ui; }

void FileBrowser::on_pushButton_close_clicked() { this->close(); }

void FileBrowser::on_pushButton_diff_clicked() {
    // Get the selected row and ensure it isn't empty
    QModelIndexList indexes = m_treeView->selectionModel()->selectedIndexes();
    if (indexes.count() == 0) {
        return;
    }

    // Check to be sure it isn't a directory
    if (m_fileModel->isDir(indexes.at(0))) {
        QMessageBox::information(this, tr("Diff File"),
                                 m_fileModel->fileName(indexes.at(0)) + tr(" is a directory, only files can be diffed"));
        return;
    }

    // Grad the path of the selected file
    const QString filePath = m_fileModel->filePath(indexes.at(0));

    // Create DiffViewer dialog
    DiffViewer df(m_snapper, m_rootPath, filePath, m_uuid);
    df.exec();
}

void FileBrowser::on_pushButton_restore_clicked() {
    // Get the selected row and ensure it isn't empty
    QModelIndexList indexes = m_treeView->selectionModel()->selectedIndexes();
    if (indexes.count() == 0) {
        return;
    }

    // Check to be sure it isn't a directory
    if (m_fileModel->isDir(indexes.at(0))) {
        QMessageBox::information(this, tr("Restore File"),
                                 m_fileModel->fileName(indexes.at(0)) + tr(" is a directory, only files can be restored"));
        return;
    }

    if (QMessageBox::question(0, tr("Confirm"), tr("Are you sure you want to restore this the file over the current file?")) !=
        QMessageBox::Yes) {
        return;
    }

    // Restore the file
    const QString filePath = m_fileModel->filePath(indexes.at(0));

    const QString targetPath = m_snapper->findTargetPath(m_rootPath, filePath, m_uuid);

    if (targetPath.isEmpty() || !m_snapper->restoreFile(filePath, targetPath)) {
        QMessageBox::warning(this, tr("Restore Failed"), tr("The file failed to restore"));
        return;
    }

    QMessageBox::information(this, tr("Restore File"), tr("The file was succesfully restored"));
}
