
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

		void setScript( KopeteAccount *account, int, const QString &script );
		QString script( KopeteAccount *account, int );

	signals:
		void changed();

	private:
		JavaScriptConfig( QObject *, const char* name );

		static JavaScriptConfig *m_config;

		JavaScriptConfigPrivate *d;
};

#endif
