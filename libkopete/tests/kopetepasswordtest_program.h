#ifndef KOPETEPASSWORDTEST_H
#define KOPETEPASSWORDTEST_H

#include <qobject.h>

class PasswordRetriever : public QObject
{
    Q_OBJECT
public:
    QString password;
public Q_SLOTS:
    void timer();
    void gotPassword(const QString &);
};

#endif
