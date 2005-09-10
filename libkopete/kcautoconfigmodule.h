/*  This file is part of the KDE project
    Copyright (C) 2003 Olivier Goffart <ogoffart @ kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

*/

#ifndef KCAUTOCONFIGMODULE_H
#define KCAUTOCONFIGMODULE_H

#include <kcmodule.h>

#include "kopete_export.h"

class KAutoConfig;
class KConfig;


/**
 * @short Convenience KCModule for creating config page handled with KAutoConfig
 *
 * This class makes it very easy to create a configuration page using KAutoConfig.
 * All you need to do is create a class that is derived from KCAutoConfigModule, create your
 * config page with QDesigner, and add it to the module
 * This can be  done using the setMainWidget() method:
 * \code
 * typedef KGenericFactory<MyPageConfig, QWidget> MyPageConfigFactory;
 * K_EXPORT_COMPONENT_FACTORY( kcm_mypageconfig, MyPageConfigFactory( "kcm_mypageconfig" ) )
 *
 * MyPageConfig( QWidget * parent, const char *, const QStringList & args )
 *     : KCAutoConfigModule( MyPageConfigFactory::instance(), parent, args )
 * {
 *       setMainWidget( new MyPageConfigBase(this) , "MyGroup" );
 * }
 * \endcode
 *
 *
 * @author Olivier Goffart <ogoffart(@)tisclinet.be>
 * @since 3.2
 */
class KOPETE_EXPORT KCAutoConfigModule : public KCModule
{
	Q_OBJECT
	public:
		/**
		* Standard KCModule constructor.  Use KGlobal::config()
		*/
		KCAutoConfigModule( QWidget * parent = 0, const char * name = 0, const QStringList & args = QStringList() );

		/**
		* Standard KCModule constructor.  Use KGlobal::config()
		*/
		KCAutoConfigModule( KInstance * instance, QWidget * parent = 0, const QStringList & args = QStringList() );

		/**
		*  Constructor.
		*  @param config the KConfig to use
		*  @param instance KInstance object for this KCM
		*  @param parent parent widget
		*  @param args special arguments for this KCM
		*
		*  @todo document what the args mean (inherited from KCModule?)
		*/
		KCAutoConfigModule(KConfig* config, KInstance * instance, QWidget * parent = 0, const QStringList & args = QStringList() );

		/**
		*  Constructor, much like the one above, except with
		*  no instance and with a name.
		*  @param config the KConfig to use
		*  @param parent parent widget
		*  @param name name of the object
		*  @param args special arguments for this KCM
		*/
		KCAutoConfigModule(KConfig* config, QWidget * parent = 0, const char * name=0 , const QStringList & args = QStringList() );


		~KCAutoConfigModule();

		/**
		 * Set the main widget. @p widget will be lay out to take all available place in the module.
		 * @p widget must have this module as parent.
		 *
		 * This method automatically call KAutoConfig::addWidget() and KAutoConfig::retrieveSettings()
		 *
		 * @param widget the widget to place on the page and to add in the KAutoConfig
		 * @param group the name of the group where settings are stored in the config file
		 */
		void setMainWidget(QWidget *widget, const QString& group);

		/**
		 * @brief a reference to the KAutoConfig
		 *
		 * You can add or remove manually some widget from the KAutoWidget.
		 * If you choose to don't add the main widget with setMainWidget() , you need
		 * to call  KAutoConfig::retrieveSettings(true) yourself
		 *
		 * @return a reference to the KAutoConfig
		 */
		KAutoConfig *autoConfig();

		/**
		 * Reload the config from the configfile.
		 *
		 * You can also reimplement this method, but you should always call the parent KCModule::load()
		 * be sure you know what you are doing
		 */
		virtual void load();

		/**
		 * Save the config to the configfile.
		 *
		 * You can also reimplement this method, but you should always call the parent KCModule::save()
		 * be sure you know what you are doing
		 */
		virtual void save();

		/**
		 * Reload the default config
		 *
		 * You can also reimplement this method, but you should always call the parent KCModule::defaults()
		 * be sure you know what you are doing
		 */
		virtual void defaults();


	protected slots:
		/**
		 * Some setting was modified, updates buttons
		 */
		virtual void slotWidgetModified();

	private:
		class KCAutoConfigModulePrivate;
		KCAutoConfigModulePrivate * d;
};


#endif
