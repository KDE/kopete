/*
    kopeteplugin.h - Kopete Plugin API

    Copyright (c) 2001-2002 by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2002-2003 by Martijn Klingens       <klingens@kde.org>

    Copyright (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef KOPETEPLUGIN_H
#define KOPETEPLUGIN_H

#include <qobject.h>

#include <kxmlguiclient.h>

class KActionCollection;
class KMainWindow;
class KPluginInfo;

namespace DOM
{
	class Node;
}

namespace Kopete
{

class MetaContact;
class MessageManager;
class Message;

/**
 * @author Duncan Mac-Vicar P. <duncan@kde.org>
 *
 * Kopete::Plugin is the base class for all Kopete plugins, and can implement
 * virtually anything you like.
 * Kopete::Plugin inherits from KXMLGUIClient. it is used in the contactlist.
 * please note the the client is added *RIGHT* after the plugin is created.
 * so you have to make every actions in the constructor
 */
class Plugin : public QObject, public KXMLGUIClient
{
	Q_OBJECT

public:
	Plugin( KInstance *instance, QObject *parent, const char *name );
	virtual ~Plugin();

	/**
	 * Returns the KPluginInfo object associated with this plugin
	 */
	KPluginInfo *pluginInfo() const;

	/**
	 * Get the name of the icon for this plugin. The icon name is taken from the
	 * .desktop file.
	 *
	 * May return an empty string if the .desktop file for this plugin specifies
	 * no icon name to use.
	 *
	 * This is a convenience method that simply calls @ref pluginInfo()->icon().
	 */
	QString pluginIcon() const;

	/**
	 * Returns the display name of this plugin.
	 *
	 * This is a convenience method that simply calls @ref pluginInfo()->name().
	 */
	QString displayName() const;

	/**
	 * Returns the plugin id, defined to be the result of calling @ref QObject::className().
	 */
	QString pluginId() const;

	/**
	 * Return the list of all keys from the address book in which the plugin
	 * is interested. Those keys are monitored for changes upon load and
	 * during runtime. When the key actually changes, the plugin's
	 * addressBookKeyChanged( Kopete::MetaContact *mc, const QString &key )
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
	 * are currently used by Kopete::Contact::serialize to autoset the index
	 * when possible.
	 *
	 * Only one field can be index field. Calling this method multiple times
	 * as index field will reset the value of index field!
	 */
	void addAddressBookField( const QString &field, AddressBookFieldAddMode mode = AddOnly );

	/**
	 * The user right-click on the chatwindow
	 */
	virtual QPtrList<KAction> *customChatWindowPopupActions( const Message &, DOM::Node &node );

	/**
	 * @brief Prepare for unloading a plugin
	 *
	 * When unloading a plugin the plugin manager first calls aboutToUnload()
	 * to indicate the pending unload. Some plugins need time to shutdown
	 * asynchronously and thus can't be simply deleted in the destructor.
	 *
	 * The default implementation immediately emits the readyForUnload() signal,
	 * which basically makes the shutdown immediate and synchronous. If you need
	 * more time you can reimplement this method and fire the signal whenever
	 * you're ready.
	 */
	virtual void aboutToUnload();

signals:
	/**
	 * Notify that the settings of a plugin were changed.
	 * These changes are passed on from the new KCDialog code in kdelibs/kutils.
	 */
	void settingsChanged();

	/**
	 * Indicate when we're ready for unload. See aboutToUnload()
	 */
	void readyForUnload();

public slots:
	/**
	 * deserialize() and tell the plugin
	 * to apply the previously stored data again.
	 * This method is also responsible for retrieving the settings from the
	 * address book. Settings that were registered can be retrieved with
	 * @ref Kopete::MetaContact::addressBookField().
	 *
	 * The default implementation does nothing.
	 */
	virtual void deserialize( Kopete::MetaContact *metaContact, const QMap<QString, QString> &data );

private:
	class Private;
	Private *d;
};

}

#endif

// vim: set noet ts=4 sts=4 sw=4:

