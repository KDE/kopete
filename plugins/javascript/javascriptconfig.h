
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _JAVASCRIPTCONFIG_H_
#define _JAVASCRIPTCONFIG_H_

#include <qobject.h>
#include <qfile.h>

#include <kglobal.h>
#include <kconfig.h>
#include <kstandarddirs.h>

#include "downloadfile.h"
#include "kopetemimetypehandler.h"
#include "javascriptplugin.h"
#include <time.h>

namespace Kopete { class Account; }
class KConfig;
class JavaScriptConfigPrivate;

class Script
{
	public:
		const QString script( bool reload = false )
		{
			if( reload || m_script.isEmpty() )
			{
				m_script = QString::null;
				QString localScriptsDir( locateLocal("data", QString::fromLatin1("kopete/scripts")) );
				QFile f( localScriptsDir + "/" +  id + "/" + fileName );

				if ( f.open( IO_ReadOnly ) )
				{
					QTextStream stream( &f );
					m_script = stream.read();
					f.close();
				}
			}

			return m_script;
		}

		QString id;
		QString name;
		QString description;
		QString author;
		QString version;
		QStringList accounts;
		QString fileName;
		QMap<QString,QString> functions;
		bool immutable;

	private:
		QString m_script;
};

class JavaScriptConfig : public QObject, public Kopete::MimeTypeHandler
{
	Q_OBJECT

	public:
		static JavaScriptConfig *instance();
		~JavaScriptConfig();

		//For Kopete::MimeTypeHandler
		virtual void handleURL( const QString &mimeType, const KURL &url ) const;

		void setSignalsEnabled( bool );
		bool signalsEnabled() const;

		void setWriteEnabled( bool );
		bool writeEnabled() const;

		void setFactoryEnabled( bool );
		bool factoryEnabled() const;

		void setTreeEnabled( bool );
		bool treeEnabled() const;

		Script* addScript( const QString &fileName, const QString &name, const QString &description,
			const QString &author, const QString &version, const QMap<QString,QString> &functions,
			const QString &id = QString::number( time( NULL ) ) );
		void removeScript( const QString &id );

		void setScriptEnabled( Kopete::Account *account, const QString &scriptId, bool enabled );
		QValueList<Script*> scriptsFor( Kopete::Account *account );
		QValueList<Script*> allScripts() const;

		Script *script( const QString &id );

		void apply();

	signals:
		void changed();

	public slots:
		//For KNewStuff
		void installPackage( const QString &fileName, bool &retVal );

	private:
		JavaScriptConfig( QObject *, const char* name );

		static JavaScriptConfig *m_config;

		JavaScriptConfigPrivate *d;
};

#endif
