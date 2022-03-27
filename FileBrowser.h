#ifndef FILEBROWSER_H
#define FILEBROWSER_H

#include "Snapper.h"

#include <QDialog>
#include <QFileSystemModel>
#include <QTreeView>

namespace Ui {
class FileBrowser;
}

class FileBrowser : public QDialog {
    Q_OBJECT

  public:
    explicit FileBrowser(Snapper *snapper, const QString &rootPath, const QString &uuid, QWidget *parent = nullptr);
    ~FileBrowser();

  private:
    Ui::FileBrowser *ui;
    QFileSystemModel *m_fileModel;
    QTreeView *m_treeView;
    QString m_rootPath;
    QString m_uuid;
    Snapper *m_snapper;

  private slots:
    void on_pushButton_close_clicked();
    void on_pushButton_restore_clicked();
};

#endif // FILEBROWSER_H
