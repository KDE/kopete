/***************************************************************************
                          kopeteprotocol.h  -  description
                             -------------------
    begin                : Tue Jan 1 2002
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

private:
	QString m_icon;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

