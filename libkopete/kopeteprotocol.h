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

#ifndef IMPROTOCOL_H
#define IMPROTOCOL_H

#include <qobject.h>
#include <qwidget.h>
#include <plugin.h>

class AddContactPage;
class QString;

/**
 * @author duncan
 */
class KopeteProtocol : public Plugin
{
public:
	KopeteProtocol(QObject *parent = 0L, const char *name = 0L);
	virtual ~KopeteProtocol();

	/**
	 * Protocol API. Must be reimplemented
	 */
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
	 * The icon of the plugin as shown in the status bar
	 *
	 * WARNING: For future use, not implemented yet! - Martijn
	 */
	QString icon() const;
	void setIcon( const QString &icon );

	/**
	 * If the protocol supports the new experimental contact list stuff, set
	 * this to true. Otherwise, leave the default and don't change!
	 */
	bool canStream() const { return m_canStream; }
	void enableStreaming( bool b ) { m_canStream = b; }

	/**
	 * Return whether the protocol supports offline messages.
	 * FIXME: Make pure virtual, or define protected method
	 *        setOfflineCapable(), instead of default implementation always
	 *        returning false.
	 */
	bool canSendOffline() const { return false; }

private:
	QString m_icon;

	bool m_canStream;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

