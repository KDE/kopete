// taken from iris

#ifndef GW_TASK_H
#define GW_TASK_H

#include <qobject.h>

#include "transfer.h"

class Client;

class Task : public QObject
{
	Q_OBJECT
public:
	enum { ErrDisc };
	Task(Task *parent);
	Task( Client *, bool isRoot );
	virtual ~Task();

	Task *parent() const;
	Client *client() const;
	Transfer *transfer() const;
	
	QString id() const;

	bool success() const;
	int statusCode() const;
	const QString & statusString() const;

	void go( bool autoDelete=false  );
	virtual bool take( const Transfer* transfer );
	void safeDelete();

signals:
	void finished();

protected:
	virtual void onGo();
	virtual void onDisconnect();
	void send( Request * request );
	void setSuccess( int code=0, const QString &str="" );
	void setError( int code=0, const QString &str="" );
// 	void debug( const char *, ... );
	void debug( const QString & );

private slots:
	void clientDisconnected();
	void done();

private:
	void init();

	class TaskPrivate;
	TaskPrivate *d;
};

#endif
