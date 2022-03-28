#include "FileBrowser.h"
#include "ui_FileBrowser.h"

#include <QDir>
#include <QMessageBox>

FileBrowser::FileBrowser(Snapper *snapper, const QString &rootPath, const QString &uuid, QWidget *parent) : QDialog(parent), ui(new Ui::FileBrowser) {
    ui->setupUi(this);

    m_rootPath = rootPath;
    m_uuid = uuid;
    m_treeView = ui->treeView_file;
    m_snapper = snapper;

    // Setup the file browser tree view
    m_fileModel = new QFileSystemModel;
    m_fileModel->setRootPath(rootPath);
    m_treeView->setModel(m_fileModel);
    m_treeView->hideColumn(1);
    m_treeView->hideColumn(2);
    m_treeView->setRootIndex(m_fileModel->index(rootPath));
    m_treeView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);

    this->setWindowTitle(tr("Snapshot File Viewer"));
}

FileBrowser::~FileBrowser() { delete ui; }

void FileBrowser::on_pushButton_close_clicked() { this->close(); }

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

    if (QMessageBox::question(0, tr("Confirm"), tr("Are you sure you want to restore this the file over the current localtion?")) !=
        QMessageBox::Yes) {
        return;
    }

    // Restore the file
    const QString filePath = m_fileModel->filePath(indexes.at(0));

    if(!m_snapper->restoreFile(m_rootPath, filePath, m_uuid)) {
        QMessageBox::warning(this, tr("Restore Failed"), tr("The file failed to restore"));
        return;
    }

    QMessageBox::information(this, tr("Restore File"), tr("The file was succesfully restored"));
}
