/*
    kopeteprotocol.h - Kopete Protocol

    Copyright (c) 2002 by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2002 by Martijn Klingens       <klingens@kde.org>

    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef KOPETEPROTOCOL_H
#define KOPETEPROTOCOL_H

#include <qobject.h>
#include <qptrlist.h>
#include <qwidget.h>

#include "kopeteplugin.h"

class KActionMenu;

class AddContactPage;
class KopeteContact;
class KopeteFileTransferInfo;
class KopeteMetaContact;
class KopeteTransfer;

/**
 * @author Duncan Mac-Vicar P. <duncan@kde.org>
 */
class KopeteProtocol : public KopetePlugin
{
	Q_OBJECT

public:
	KopeteProtocol(QObject *parent = 0L, const char *name = 0L);
	virtual ~KopeteProtocol();

	/**
	 * Protocol API. Must be reimplemented
	 */
	virtual bool unload();
	virtual QString protocolIcon() const = 0;
	virtual void Connect()=0;
	virtual void Disconnect()=0;
	virtual bool isConnected() const = 0;

	// this will be called if main-kopete wants
	// the plugin to set the user's mode to away or something similar
	virtual void setAway(void)=0;
	// this will be called if main-kopete wants
	// the plugin to set the user's mode to online or something similar
	virtual void setAvailable(void)=0;
	// plugin has to return wether it is away or not
	// plugins should also return TRUE for modes like occupied not-vailable etc.
	virtual bool isAway(void) const = 0;

	virtual AddContactPage *createAddContactWidget(QWidget *parent)=0;

	/**
	 * The icon of the plugin as shown in the status bar. .mng animations
	 * are supported too, but tried last for performance reasons
	 */
	QString statusIcon() const;
	void setStatusIcon( const QString &icon );

	/**
	 * Return whether the protocol supports offline messages.
	 * FIXME: Make pure virtual, or define protected method
	 *        setOfflineCapable(), instead of default implementation always
	 *        returning false.
	 */
	bool canSendOffline() const { return false; }

	/**
	 * Return a KActionMenu using a custom menu to plug into e.g. the system
	 * tray icon and the protocol icon in the status bar, but maybe elsewhere
	 * too.
	 * The default implementation returns a null pointer, to disable any menu.
	 *
	 * Note that you are responsible for allocating and deleting the
	 * KActionMenu yourself (as far as Qt's API doesn't do it for you ).
	 */
	virtual KActionMenu* protocolActions();

	/**
	 * Function has to be reimplemented in every single protocol
	 * and return the KopeteContact associated with the 'home' user.
	 *
	 * @return contact associated with the currently logged in user
	 */
	virtual KopeteContact* myself() const=0;

signals:
	/**
	 * The status icon changed. See also @ref setStatusIcon().
	 * This signal is only emitted if the new icon is different from
	 * the previous icon.
	 */
	void statusIconChanged( KopeteProtocol *protocol, const QString &icon );

private:
	QString m_statusIcon;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

