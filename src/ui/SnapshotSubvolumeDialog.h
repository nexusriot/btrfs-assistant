#pragma once

#include <QDialog>

namespace Ui {
class SnapshotSubvolumeDialog;
}

class SnapshotSubvolumeDialog : public QDialog {
    Q_OBJECT

  public:
    explicit SnapshotSubvolumeDialog(QWidget *parent = nullptr);
    ~SnapshotSubvolumeDialog();

    QString destination() const;
    void setDestination(const QString &text);

    bool isReadOnly() const;
    void setReadOnly(bool value);

    void selectAllTextAndSetFocus();

  private:
    Ui::SnapshotSubvolumeDialog *ui;
};
