#ifndef FILTERLINEEDIT_H
#define FILTERLINEEDIT_H
#include <QLineEdit>

class QToolButton;

class FilterLineEdit : public QLineEdit {
    Q_OBJECT
  public:
    FilterLineEdit(QWidget *parent = nullptr);

  protected:
    void resizeEvent(QResizeEvent *) override;

  private slots:
    void updateButton(const QString &text);

  private:
    QToolButton *m_clearButton;
};

#endif // FILTERLINEEDIT_H
