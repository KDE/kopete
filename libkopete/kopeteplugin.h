/*
    kopeteplugin.h - Kopete Plugin API

    Copyright (c) 2001-2002 by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2002-2003 by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2002-2005 by Olivier Goffart        <ogoffart@kde.org>

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

#include <kxmlguiclient.h>
#include <QtCore/QObject>
#include <kdemacros.h>

#include "kopete_export.h"

class KPluginInfo;

namespace Kopete
{

class MetaContact;

/**
 * @brief Base class for all plugins or protocols.
 *
 * To create a plugin, you need to create a .desktop file which looks like that:
 * \verbatim
[Desktop Entry]
Encoding=UTF-8
Type=Service
X-Kopete-Version=1000900
Icon=icon
ServiceTypes=Kopete/Plugin
X-KDE-Library=kopete_myplugin
X-KDE-PluginInfo-Author=Your Name
X-KDE-PluginInfo-Email=your@mail.com
X-KDE-PluginInfo-Name=kopete_myplugin
X-KDE-PluginInfo-Version=0.0.1
X-KDE-PluginInfo-Website=http://yoursite.com
X-KDE-PluginInfo-Category=Plugins
X-KDE-PluginInfo-Depends=
X-KDE-PluginInfo-License=GPL
X-KDE-PluginInfo-EnabledByDefault=false
Name=MyPlugin
Comment=Plugin that do some nice stuff
 \endverbatim
 *
 * The constructor of your plugin should looks like this:
 *
 * \code
	typedef KGenericFactory<MyPlugin> MyPluginFactory;
	static const KAboutData aboutdata("kopete_myplugin", 0, ki18n("MyPlugin") , "1.0" );
	K_EXPORT_COMPONENT_FACTORY( kopete_myplugin, MyPluginFactory( &aboutdata )  )

	MyPlugin::MyPlugin( QObject *parent, const char *name, const QStringList &  args  )
		: Kopete::Plugin( MyPluginFactory::componentData(), parent, name )
	{
		//...
	}
 \endcode
 *
 * Kopete::Plugin inherits from KXMLGUIClient.  That client is added
 * to the Kopete's mainwindow KXMLGUIFactory. So you may add actions
 * on the main window (for hinstance in the meta contact popup menu).
 * Please note that the client is added right after the plugin is created,
 * so you have to create every actions in the constructor.
 *
 * @author Duncan Mac-Vicar P. <duncan@kde.org>
 * @author Olivier Goffart <ogoffart\@kde.org>
 */
class KOPETE_EXPORT Plugin : public QObject, public KXMLGUIClient
{
	Q_OBJECT

public:
	Plugin( const KComponentData &instance, QObject *parent );
	virtual ~Plugin();

	/**
	 * Returns the KPluginInfo object associated with this plugin
	 */
	KPluginInfo pluginInfo() const;

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
	 * @brief Get the plugin id
	 * @return the plugin's id which is gotten by calling QObject::metaObject()->className().
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
	 * @brief Prepare for unloading a plugin
	 *
	 * When unloading a plugin the plugin manager first calls aboutToUnload()
	 * to indicate the pending unload. Some plugins need time to shutdown
	 * asynchronously and thus can't be simply deleted in the destructor.
	 *
	 * The default implementation immediately emits the @ref readyForUnload() signal,
	 * which basically makes the shutdown immediate and synchronous. If you need
	 * more time you can reimplement this method and fire the signal whenever
	 * you're ready. (you have 3 seconds)
	 *
	 * @ref Kopete::Protocol reimplement it.
	 */
	virtual void aboutToUnload();

	/**
	 * Called when user is about to close the main-window.  If your plugin is
	 * going to return false to shouldExitOnclose(), then you can show a
	 * message here.
	 *
	 * Default implementation returns false.
	 *
	 * @return true if a message was shown (so that the user is not spammed
	 * with further messages), false otherwise
	 *
	 * Not implemented as a virtual method to keep binary compatibility. You
	 * *must* mark the method as Q_INVOKABLE for it to be called.
	 */
	Q_INVOKABLE bool showCloseWindowMessage();

	/**
	 * Called when the application is about to exit.
	 * Return false if you want to prevent exit. If you do so you may want to
	 * display an explanation message by reimplementing
	 * showCloseWindowMessage().
	 *
	 * Default implementation returns true.
	 *
	 * Not implemented as a virtual method to keep binary compatibility. You
	 * *must* mark the method as Q_INVOKABLE for it to be called.
	 */
	Q_INVOKABLE bool shouldExitOnclose();

signals:
	/**
	 * Notify that the settings of a plugin were changed.
	 * These changes are passed on from the new KCDialog code in kdelibs/kutils.
	 */
	void settingsChanged();

	/**
	 * Indicate when we're ready for unload.
	 * @see aboutToUnload()
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
	 *
	 * @todo we probably should think to another way to save the contacltist.
	 */
	virtual void deserialize( MetaContact *metaContact, const QMap<QString, QString> &data );

private:
	class Private;
	Private * const d;
};


} //END namespace Kopete


#endif
