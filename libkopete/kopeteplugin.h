/*
    kopeteplugin.h - Kopete Plugin API

    Copyright (c) 2001-2002 by Duncan Mac-Vicar P. <duncan@kde.org>
    Copyright (c) 2002      by Martijn Klingens    <klingens@kde.org>

    Copyright (c) 2002 by the Kopete developers    <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef KOPETEPLUGIN_H
#define KOPETEPLUGIN_H

#include <qobject.h>

class KopeteMetaContact;
class KActionCollection;

/**
 * @author Duncan Mac-Vicar P. <duncan@kde.org>
 *
 * KopetePlugin is the base class for all Kopete plugins, and can implement
 * virtually anything you like. Currently not the full Kopete API allows it
 * to have plugins hooked into it, but that will hopefully change soon.
 */
class KopetePlugin : public QObject
{
	Q_OBJECT

public:
	KopetePlugin( QObject *parent = 0L, const char *name = 0L );
	virtual ~KopetePlugin();

	virtual void init();
	virtual bool unload();
	virtual const char *id() const;

	/**
	 * Return the list of all keys from the address book in which the plugin
	 * is interested. Those keys are monitored for changes upon load and
	 * during runtime. When the key actually changes, the plugin's
	 * addressBookKeyChanged( KopeteMetaContact *mc, const QString &key )
	 * is called.
	 * The default implementation returns an empty list.
	 */
	virtual QStringList addressBookFields() const;

	/**
	 * Returns a set of custom menu items for the meta contact's context menu
	 */
	virtual KActionCollection *customContextMenuActions(KopeteMetaContact*) { return 0l; };


public slots:
	/**
	 * Store the plugin data for a given meta contact in QStringList and
	 * return it. If a given metacontact contains more than one KopeteContact
	 * for which the plugin wishes to store data it has to append that
	 * data to the returned QStringList too.
	 * If this plugin doesn't has any useful data to store regarding a meta
	 * contact, return false. Otherwise return true and stream the required
	 * settings into the provided stream.
	 *
	 * This method is also responsible for storing the settings in the KDE
	 * address book. You can save all fields that you registered for using
	 * @ref KopeteMetaContact::setAddressBookField().
	 *
	 * The default implementation returns false to disable streaming and
	 * doesn't store anything in the KDE address book.
	 */
	virtual bool serialize( KopeteMetaContact *metaContact,
		QStringList &strList ) const;

	/**
	 * deserialize() does the opposite of serialize() and tells the plugin
	 * to apply the previously stored data again.
	 * This method is also responsible for retrieving the settings from the
	 * address book. Settings that were registered can be retrieved with
	 * @ref KopeteMetaContact::addressBookField().
	 *
	 * The default implementation does nothing.
	 */
	virtual void deserialize( KopeteMetaContact *metaContact, const QStringList& data );

	/**
	 * Notify that an address book field was changed.
	 * You need to register your fields to receive the notifications
	 *
	 * The default implementation does nothing.
	 */
	virtual void addressBookFieldChanged( KopeteMetaContact *c,
		const QString &key );
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

