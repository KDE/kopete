/*
    task.h - Kopete Groupwise Protocol
      
    Copyright (c) 2004      SUSE Linux AG	 	 http://www.suse.com
    
    Based on Iris, Copyright (C) 2003  Justin Karneges <justin@affinix.com>
    
    Kopete (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>
 
    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/
  
#ifndef YAHOO_TASK_H
#define YAHOO_TASK_H

#include <qobject.h>

class QString;

namespace KYahoo {
	class Client;
}

class Transfer;

class Task : public QObject
{
	Q_OBJECT
public:
	enum { ErrDisc };
	Task(Task *parent);
	Task( KYahoo::Client *, bool isRoot );
	virtual ~Task();

	Task *parent() const;
	KYahoo::Client *client() const;
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
	void send( Transfer * request );
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
	 * Creates a transfer with the given command and field list
	 */
	//void createTransfer( const QString & command, const Field::FieldList fields );
	/**
	 * Direct setter for Tasks which don't have any fields
	 */
	void setTransfer( Transfer * transfer );
private slots:
	void clientDisconnected();
	void done();

private:
	void init();

	class TaskPrivate;
	TaskPrivate *d;
};

#endif
