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
#include <qstringlist.h>

class KopeteMetaContact;
class KopeteMessageManager;
class KActionCollection;
class KMainWindow;

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

	virtual bool unload();
	QString pluginId() const;

	/**
	 * Return the list of all keys from the address book in which the plugin
	 * is interested. Those keys are monitored for changes upon load and
	 * during runtime. When the key actually changes, the plugin's
	 * addressBookKeyChanged( KopeteMetaContact *mc, const QString &key )
	 * is called.
	 * You can add fields to the list using @ref addAddressBookField()
	 */
	QStringList addressBookFields() const;

	/**
	 * Return the index field as set by @ref addAddressBookField()
	 */
	QString addressBookIndexField() const;

	/**
	 * Mode for an address book field as used by @ref addAddressBookField()
	 */
	enum AddressBookFieldAddMode { AddOnly, MakeIndexField };

	/**
	 * Add a field to the list of address book fields. See also @ref addressBookFields()
	 * for a description of the fields.
	 *
	 * Set mode to MakeIndexField to make this the index field. Index fields
	 * are currently used by KopeteContact::serialize to autoset the index
	 * when possible.
	 *
	 * Only one field can be index field. Calling this method multiple times
	 * as index field will reset the value of index field!
	 */
	void addAddressBookField( const QString &field, AddressBookFieldAddMode mode = AddOnly );

	/**
	 * Returns a set of custom menu items for the meta contact's context menu
	 */
	virtual KActionCollection *customContextMenuActions(KopeteMetaContact*) { return 0l; };

	/**
	 * Returns a set of action items for the chatWindows
	 */
	virtual KActionCollection *customChatActions(KopeteMessageManager*) { return 0l; };

	/**
	 * Returns a set of action items to be shown on chat window toolbars
	 */
	virtual KActionCollection *customToolbarActions() { return 0L; };

	/**
	 * Get the name of the icon for this plugin. The icon name is taken from the
	 * .desktop file.
	 *
	 * May return an empty string if the .desktop file for this plugin specifies
	 * no icon name to use.
	 *
	 * This is a convenience method that simply calls @ref PluginLoader::pluginIcon().
	 */
	QString pluginIcon() const;
	
	/**
	 * Returns the display name of this plugin.
	 *
	 * This is a convenience method that simply calls @ref PluginLoader::pluginName().
	 */
	QString displayName() const;

public slots:
	/**
	 * deserialize() does the opposite of serialize() and tells the plugin
	 * to apply the previously stored data again.
	 * This method is also responsible for retrieving the settings from the
	 * address book. Settings that were registered can be retrieved with
	 * @ref KopeteMetaContact::addressBookField().
	 *
	 * The default implementation does nothing.
	 */
	virtual void deserialize( KopeteMetaContact *metaContact, const QMap<QString, QString> &data );

signals:
	/*
	 * Signal emitted when the protocol is unloaded
	 */
	void unloading();

private:
	QStringList m_addressBookFields;
	QString m_indexField;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

