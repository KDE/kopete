/*
    kopeteplugin.h - Kopete Plugin API

    Copyright (c) 2001-2002 by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2002-2003 by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2002-2004 by Olivier Goffart        <ogoffart@ tiscalinet.be>

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
#include <qobject.h>

class KPluginInfo;

namespace Kopete {


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
	static const KAboutData aboutdata("kopete_myplugin", I18N_NOOP("MyPlugin") , "1.0" );
	K_EXPORT_COMPONENT_FACTORY( kopete_myplugin, MyPluginFactory( &aboutdata )  )

	MyPlugin::MyPlugin( QObject *parent, const char *name, const QStringList &  args  )
		: Kopete::Plugin( MyPluginFactory::instance(), parent, name )
	{
		//...
	}
 \endcode
 *
 * Kopete::Plugin inherits from @ref KXMLGUIClient.  That client is added
 * to the Kopete's mainwindow @ref KXMLGUIFactory. So you may add actions
 * on the main window (for hinstance in the meta contact popup menu).
 * Please note the the client is added right after the plugin is created.
 * so you have to create every actions in the constructor
 *
 * @author Duncan Mac-Vicar P. <duncan@kde.org>
 * @author Olivier Goffart <ogoffart @ tiscalinet.be>
 */
class Plugin : public QObject, public KXMLGUIClient
{
	Q_OBJECT

public:
	Plugin( KInstance *instance, QObject *parent, const char *name );
	~Plugin();

	/**
	 * return the plugin id. this is practically the name of the class.
	 * see @ref QObject::className()
	 */
	QString pluginId() const;

	/**
	 * Get the name of the icon for this plugin. The icon name is taken from the
	 * .desktop file.
	 *
	 * May return an empty string if the .desktop file for this plugin specifies
	 * no icon name to use.
	 */
	QString pluginIcon() const;

	/**
	 * Returns the display name of this plugin.
	 */
	QString displayName() const;

	/**
	 * Returns the info of that plugin
	 */
	KPluginInfo *pluginInfo() const;

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

#if 0
	/**
	 * deserialize() and tell the plugin
	 * to apply the previously stored data again.
	 * This method is also responsible for retrieving the settings from the
	 * address book. Settings that were registered can be retrieved with
	 * @ref KopeteMetaContact::addressBookField().
	 *
	 * The default implementation does nothing.
	 *
	 * @todo we probably should think to another way to save the contacltist.
	 */
	//virtual void deserialize( KopeteMetaContact *metaContact, const QMap<QString, QString> &data );
#endif

protected:
	virtual void virtual_hook( uint id, void *data );

private:
	class Private;
	Private *d;
};

} //END namespace Kopete

#endif
