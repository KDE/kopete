// taken from iris

#ifndef GW_TASK_H
#define GW_TASK_H

#include <qobject.h>

#include "transfer.h"

class Client;
class Request; 

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
	/** 
	 * Allows a task to examine an incoming Transfer and decide whether to 'take' it
	 * for further processing.
	 */
	virtual bool take( Transfer* transfer );
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
	/**
	 * Used in take() to check if the offered transfer is for this Task
	 * @return true if this Task should take the Transfer.  Default impl always returns false.
	 */
	virtual bool forMe( const Transfer * transfer ) const;
	/**
	 * Sets the transfer the task is about to send or receive
	 */
	virtual void setTransfer( Transfer * transfer );
	
private slots:
	void clientDisconnected();
	void done();

private:
	void init();

	class TaskPrivate;
	TaskPrivate *d;
};

#endif
