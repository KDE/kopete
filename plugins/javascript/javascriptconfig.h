
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

#include "javascriptplugin.h"

class KopeteAccount;
class KConfig;
class JavaScriptConfigPrivate;

struct Script
{
	QString name;
	QString description;
	QString author;
	QString script;
	QMap<QString,QString> accounts;
	bool immutable;
};

class JavaScriptConfig : public QObject
{
	Q_OBJECT

	public:
		static JavaScriptConfig *instance();
		~JavaScriptConfig();

		void setSignalsEnabled( bool );
		bool signalsEnabled() const;

		void setWriteEnabled( bool );
		bool writeEnabled() const;

		void setFactoryEnabled( bool );
		bool factoryEnabled() const;

		void setTreeEnabled( bool );
		bool treeEnabled() const;

		Script* addScript( const QString &name, const QString &description,
			const QString &author, const QString &contents );
		void writeScript( const QString &name, const QString &contents );
		void removeScript( const QString &name );

		void setScriptEnabled( KopeteAccount *account, int type, const QString &script, bool enabled );
		QStringList scriptsFor( KopeteAccount *account, int );
		QStringList scriptNamesFor( KopeteAccount *account, int );
		QStringList accountsFor( int type, const QString &script );

		Script *script( const QString &name );

		void apply();

	signals:
		void changed();

	private:
		JavaScriptConfig( QObject *, const char* name );

		static JavaScriptConfig *m_config;

		JavaScriptConfigPrivate *d;
};

#endif
