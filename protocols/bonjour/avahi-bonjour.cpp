/*
    avahi-bonjour.cpp - Kopete Bonjour Protocol

    Copyright (c) 2007      by Tejas Dinkar	<tejas@gja.in>
    Copyright (c) 2003      by Will Stephenson		 <will@stevello.free-online.co.uk>
    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU General Public                   *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

/* 
  Code in This File Should Eventually Be Removed, preferably by getting this functionality
  committed into KDNSSD.
 
  Currently, Functions are needed to:
  	1) Resolve a .local hostname (without mdns)
  	2) Get the local hostname
  	3) Set a NULL DNS Record (containing a userpic)
  	4) Get a NULL DNS Record (again, with userpic)
 
  PS: I don't know why Userpics are encoded in DNS Records. Ask Apple why the protocol does that
*/

#include <QtDBus>

#include "bonjouraccount.h"

// FIXME: Remove Avahi Dependencies
QString BonjourAccount::resolveHostName(const QString &hostname)
{
	QDBusInterface iface("org.freedesktop.Avahi", "/", "org.freedesktop.Avahi.Server", QDBusConnection::systemBus());

	QList<QVariant> list;
	list << -1 << -1 << hostname << 0 << (unsigned int ) 0;

	QDBusMessage reply = iface.callWithArgumentList(QDBus::Block, "ResolveHostName", list);

	if (reply.type() == QDBusMessage::ReplyMessage)
		return reply.arguments()[4].toString();
	else
		return QString();
}

// FIXME: Remove Avahi Dependencies
QString BonjourAccount::getLocalHostName()
{
	QDBusInterface iface("org.freedesktop.Avahi", "/", "org.freedesktop.Avahi.Server", QDBusConnection::systemBus());

	QDBusMessage reply = iface.call(QDBus::Block, "GetHostName");

	if (reply.type() == QDBusMessage::ReplyMessage)
		return reply.arguments()[0].toString();
	else
		return QString();
}

// FIXME: Remove Avahi Dependency
bool BonjourAccount::check_mDNS_running()
{
	QDBusInterface iface("org.freedesktop.Avahi", "/", "org.freedesktop.Avahi.Server", QDBusConnection::systemBus());

	QDBusMessage reply = iface.call(QDBus::Block, "GetVersionString");

	if (reply.type() == QDBusMessage::ReplyMessage)
		return true;
	else
		return false;
}
