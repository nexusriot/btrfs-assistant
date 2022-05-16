#ifndef RESTORECONFIRMDIALOG_H
#define RESTORECONFIRMDIALOG_H

#include <QDialog>

namespace Ui {
class RestoreConfirmDialog;
}

class RestoreConfirmDialog : public QDialog
{
    Q_OBJECT

public:
    RestoreConfirmDialog(const QString &title, const QString &label, QWidget *parent = nullptr);
    ~RestoreConfirmDialog();
    QString backupName();

private:
    Ui::RestoreConfirmDialog *m_ui = nullptr;
    QString m_backupName;

private slots:
    void on_pushButton_yes_clicked();
    void on_pushButton_no_clicked();

};

#endif // SNAPSHOTSUBVOLUMEDIALOG_H
