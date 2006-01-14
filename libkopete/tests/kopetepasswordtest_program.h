#ifndef KOPETEPASSWORDTEST_H
#define KOPETEPASSWORDTEST_H

#include <qobject.h>

class PasswordRetriever : public QObject
{
	Q_OBJECT
public:
	QString password;
public slots:
	void timer();
	void gotPassword( const QString & );
};

#endif
