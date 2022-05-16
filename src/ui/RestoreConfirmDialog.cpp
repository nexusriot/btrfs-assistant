#include "RestoreConfirmDialog.h"
#include "ui_RestoreConfirmDialog.h"

#include <QDir>

RestoreConfirmDialog::RestoreConfirmDialog(const QString &title, const QString &label, QWidget *parent) :
    QDialog(parent), m_ui(new Ui::RestoreConfirmDialog)
{
    m_ui->setupUi(this);
    m_ui->questionLabel->setText(label);
    this->setWindowTitle(title);
}

RestoreConfirmDialog::~RestoreConfirmDialog()
{
    delete m_ui;
}

void RestoreConfirmDialog::on_pushButton_yes_clicked()
{
    m_backupName = m_ui->backupNameLineEdit->displayText().simplified();
    m_backupName.remove(QDir::separator());
    accept();
}

void RestoreConfirmDialog::on_pushButton_no_clicked()
{
    reject();
}

QString RestoreConfirmDialog::backupName()
{
    return m_backupName;
}
