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

	if (stat == Online)
		return i18n("Online");

	if (stat == Away)
		return i18n("Away");

	if (stat == Offline)
		return i18n("Offline");
}

QString KopeteContact::statusIcon() const
{
	return "unknown";
}

int KopeteContact::importance() const {
	ContactStatus stat = status();

	if (stat == Online)
		return 20;

	if (stat == Away)
		return 10;

	if (stat == Offline)
		return 0;
}

#include "kopetecontact.moc"
