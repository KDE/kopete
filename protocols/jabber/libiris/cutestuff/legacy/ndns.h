/*
 * ndns.h - native DNS resolution
 * Copyright (C) 2001, 2002  Justin Karneges
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef CS_NDNS_H
#define CS_NDNS_H

#include <qobject.h>
#include <q3cstring.h>
#include <qthread.h>
#include <qmutex.h>
#include <qhostaddress.h>
//Added by qt3to4:
#include <QEvent>

// CS_NAMESPACE_BEGIN

class NDnsManager;

class NDns : public QObject
{
	Q_OBJECT
public:
	NDns(QObject *parent=0);
	~NDns();

	void resolve(const QString &);
	void stop();
	bool isBusy() const;

	uint result() const;
	QString resultString() const;

signals:
	void resultsReady();

private:
	QHostAddress addr;

	friend class NDnsManager;
	void finished(const QHostAddress &);
};

class NDnsManager : public QObject
{
	Q_OBJECT
public:
	~NDnsManager();
	
//! \if _hide_doc_
protected:
	bool event(QEvent *);
//! \endif

private slots:
	void app_aboutToQuit();

private:
	class Private;
	class Item;
	Private *d;

	friend class NDns;
	NDnsManager();
	void resolve(NDns *self, const QString &name);
	void stop(NDns *self);
	bool isBusy(const NDns *self) const;
	void tryDestroy();
};

// CS_NAMESPACE_END

#endif
