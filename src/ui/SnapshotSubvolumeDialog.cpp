#include "SnapshotSubvolumeDialog.h"
#include "ui_SnapshotSubvolumeDialog.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>

SnapshotSubvolumeDialog::SnapshotSubvolumeDialog(QWidget *parent) : QDialog(parent), ui(new Ui::SnapshotSubvolumeDialog)
{
    ui->setupUi(this);

    disconnect(ui->buttonBox, &QDialogButtonBox::accepted, nullptr, nullptr);

    connect(ui->browseButton, &QPushButton::clicked, this, [this]() {
        QString dir = QFileDialog::getExistingDirectory(this, tr("Select a parent directory"));
        if (!dir.isEmpty()) {
            ui->destinationLineEdit->setText(dir);
        }
    });

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, [this]() {
        QString dest = ui->destinationLineEdit->text().trimmed();
        if (dest.isEmpty()) {
            QMessageBox::warning(this, tr("Btrfs Assistant"), tr("The destination path cannot be empty"));
            ui->destinationLineEdit->setFocus();
        } else {
            if (!dest.startsWith(QChar('/'))) {
                dest = QFileInfo(dest).absoluteFilePath();
                if (QMessageBox::question(
                        this, tr("Btrfs Assistant"),
                        tr("You entered a relative path. Do you want to continue with the resulting absolute path: %1?").arg(dest)) ==
                    QMessageBox::No) {
                    ui->destinationLineEdit->setFocus();
                    return;
                }
            }

            accept();
        }
    });
}

SnapshotSubvolumeDialog::~SnapshotSubvolumeDialog() { delete ui; }

QString SnapshotSubvolumeDialog::destination() const { return ui->destinationLineEdit->text().trimmed(); }

void SnapshotSubvolumeDialog::setDestination(const QString &text) { ui->destinationLineEdit->setText(text); }

bool SnapshotSubvolumeDialog::isReadOnly() const { return ui->readOnlyCheckBox->isChecked(); }

void SnapshotSubvolumeDialog::setReadOnly(bool value) { ui->readOnlyCheckBox->setChecked(value); }

void SnapshotSubvolumeDialog::selectAllTextAndSetFocus()
{
    ui->destinationLineEdit->selectAll();
    ui->destinationLineEdit->setFocus();
}
