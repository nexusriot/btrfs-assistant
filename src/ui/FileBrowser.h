#ifndef FILEBROWSER_H
#define FILEBROWSER_H

#include "util/Snapper.h"

#include <QDialog>
#include <QFileSystemModel>
#include <QTreeView>

namespace Ui {
class FileBrowser;
}

/**
 * @brief The FileBrowser class that handles the snapshot file browsing dialog
 */
class FileBrowser : public QDialog {
    Q_OBJECT

  public:
    FileBrowser(Snapper *snapper, const QString &rootPath, const QString &uuid, QWidget *parent = nullptr);
    FileBrowser(const QString &rootPath, const QString &uuid, QWidget *parent = nullptr);
    ~FileBrowser();

  private:
    Ui::FileBrowser *m_ui = nullptr;
    QString m_rootPath;
    QString m_uuid;
    Snapper *m_snapper = nullptr;
    QTreeView *m_treeView = nullptr;
    QFileSystemModel *m_fileModel = nullptr;
    void intializeFileBrowser(const QString &rootPath);

  private slots:
    void on_pushButton_close_clicked();
    void on_pushButton_diff_clicked();
    void on_pushButton_restore_clicked();
};

#endif // FILEBROWSER_H
