/***************************************************************************
                          kopetecontact.cpp  -  description
                             -------------------
    begin                : Wed Jan 2 2002
    copyright            : (C) 2002 by duncan
    email                : duncan@tarro
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kopetecontact.h"
#include "kopete.h"

#include <qtimer.h>
#include <klistview.h>
#include <kdebug.h>
#include <klocale.h>

KopeteContact::KopeteContact(QObject *parent)
	: QObject(parent)
{
	connect(this, SIGNAL(incomingEvent(KopeteEvent *)), kopeteapp, SLOT(notifyEvent(KopeteEvent *)));
}

KopeteContact::~KopeteContact()
{
}

void KopeteContact::setName( const QString &name )
{
	mName = name;
	emit nameChanged(name);
}

QString KopeteContact::name() const
{
	return mName;
}

KopeteContact::ContactStatus KopeteContact::status() const
{
	return Online;
}

QString KopeteContact::statusText() const
{
	ContactStatus stat = status();

	kdDebug() << "[KopeteContact] statusText() with status= " << stat << endl;

	switch( stat )
	{
	case Online:
		return i18n("Online");
	case Away:
		return i18n("Away");
	case Offline:
	default:
		return i18n("Offline");
	}
}

QString KopeteContact::statusIcon() const
{
	return "unknown";
}

int KopeteContact::importance() const
{
	ContactStatus stat = status();

	kdDebug() << "[KopeteContact] importance() with status= " << stat << endl;

	if (stat == Online)
		return 20;

	if (stat == Away)
		return 10;

	if (stat == Offline)
		return 0;

	return 0;
}

QStringList KopeteContact::groups()
{
	return QStringList();
}

void KopeteContact::addToGroup( const QString & /* group */ )
{
	kdDebug() << "KopeteContact::addToGroup: WARNING: "
		<< "Default implementation called! Function not implemented?" << endl;
}

void KopeteContact::removeFromGroup( const QString & /* group */ )
{
	kdDebug() << "KopeteContact::removeFromGroup: WARNING: "
		<< "Default implementation called! Function not implemented?" << endl;
}

void KopeteContact::moveToGroup( const QString & /* from */,
	const QString & /* to */ )
{
	kdDebug() << "KopeteContact::moveToGroup: WARNING: "
		<< "Default implementation called! Function not implemented?" << endl;
}

#include "kopetecontact.moc"

// vim: set noet ts=4 sts=4 sw=4:

