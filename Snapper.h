#ifndef SNAPPER_H
#define SNAPPER_H

#include <QObject>

class Snapper : public QObject
{
    Q_OBJECT
public:
    explicit Snapper(QObject *parent = nullptr);

signals:

};

#endif // SNAPPER_H
