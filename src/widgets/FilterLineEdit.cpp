#include "FilterLineEdit.h"
#include <QStyle>
#include <QToolButton>

FilterLineEdit::FilterLineEdit(QWidget *parent) : QLineEdit(parent)
{

    m_clearButton = new QToolButton(this);
    QIcon icon = QIcon::fromTheme("edit-clear", QIcon(":/icons/edit-clear.svg"));
    m_clearButton->setIcon(icon);
    m_clearButton->setCursor(Qt::ArrowCursor);
    m_clearButton->setStyleSheet("QToolButton { border: none; padding: 0px; }");
    m_clearButton->hide();

    connect(m_clearButton, &QToolButton::clicked, this, &FilterLineEdit::clear);
    connect(this, &FilterLineEdit::textChanged, this, &FilterLineEdit::updateButton);

    int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    setStyleSheet(QString("QLineEdit { padding-right: %1px; } ").arg(m_clearButton->sizeHint().width() + frameWidth + 1));
    QSize msz = minimumSizeHint();
    setMinimumSize(qMax(msz.width(), m_clearButton->sizeHint().height() + frameWidth * 2 + 2),
                   qMax(msz.height(), m_clearButton->sizeHint().height() + frameWidth * 2 + 2));
}

void FilterLineEdit::resizeEvent(QResizeEvent *)
{
    QSize sz = m_clearButton->sizeHint();
    int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    m_clearButton->move(rect().right() - frameWidth - sz.width(), (rect().bottom() + 1 - sz.height()) / 2);
}

void FilterLineEdit::updateButton(const QString &text) { m_clearButton->setVisible(!text.isEmpty()); }
