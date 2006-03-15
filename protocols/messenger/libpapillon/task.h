/*
   task.h - Papillon Task base class.

   Copyright (c) 2006 by Michaël Larouche <michael.larouche@kdemail.net>

   Based on code Copyright (c) 2004 Matt Rogers <mattr@kde.org>
   Based on code copyright (c) 2004 SuSE Linux AG <http://www.suse.com>
   Based on Iris, Copyright (C) 2003 Justin Karneges

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
*/

#ifndef PAPILLON_TASK_H
#define PAPILLON_TASK_H

#include <QObject>
#include <papillon_macros.h>

class QString;

namespace Papillon
{

class Transfer;
class Connection;
/**
 * @author Michaël Larouche <michael.larouche@kdemail.net>
 * @author Matt Rogers  <mattr@kde.org>
 * @author SuSE Linux AG <http://www.suse.com>
 * @author Justin Karneges
 */
class PAPILLON_EXPORT Task : public QObject
{
	Q_OBJECT
public:
	enum { ErrDisc };
	Task(Task *parent);
	Task(Connection*, bool isRoot);
	virtual ~Task();

	Task *parent() const;
	Connection* client() const;
	Transfer *transfer() const;

	quint32 id() const;
	void setId();

	bool success() const;
	int statusCode() const;
	const QString &statusString() const;

	void go(bool autoDelete = false);

	/**
	 * Allows a task to examine an incoming Transfer and decide whether to 'take' it
	 * for further processing.
	 */
	virtual bool take(Transfer *transfer);
	void safeDelete();

	/**
	 * Direct setter for Tasks which don't have any fields
	 */
	void setTransfer(Transfer *transfer);

signals:
	void finished();

protected:
	virtual void onGo();
	virtual void onDisconnect();
	void setId( quint32 id );
	void send(Transfer *request);
	void setSuccess(int code=0, const QString &str = QLatin1String(""));
	void setError(int code=0, const QString &str = QLatin1String(""));
	void debug(const QString &);

	/**
	 * Used in take() to check if the offered transfer is for this Task
	 * @return true if this Task should take the Transfer.  Default impl always returns false.
	 */
	virtual bool forMe(Transfer *transfer) const;

private slots:
	void clientDisconnected();
	void done();

private:
	void init();

	class Private;
	Private *d;
};

}
#endif
