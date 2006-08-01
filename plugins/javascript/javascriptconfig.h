
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KOPETE_JAVASCRIPTCONFIG_H
#define KOPETE_JAVASCRIPTCONFIG_H

//#include "downloadfile.h"
#include "javascriptplugin.h"

#include "kopetemimetypehandler.h"

#include <kurl.h>

#include <time.h>

class JavaScriptFile;

namespace Kopete {
	class Account;
}

class JavaScriptConfig
	: public QObject
	, public Kopete::MimeTypeHandler
{
	Q_OBJECT

public:
	static JavaScriptConfig *instance();
	~JavaScriptConfig();

	//For Kopete::MimeTypeHandler
	virtual void handleURL(const QString &mimeType, const KUrl &url) const;

	void setSignalsEnabled( bool );
	bool signalsEnabled() const;

	void setWriteEnabled( bool );
	bool writeEnabled() const;

	void setFactoryEnabled( bool );
	bool factoryEnabled() const;

	void setTreeEnabled( bool );
	bool treeEnabled() const;

	JavaScriptFile* addScript( const QString &fileName, const QString &name, const QString &description,
		const QString &author, const QString &version, const QMap<QString,QString> &functions,
		const QString &id = QString::number( time( NULL ) ) );
	void removeScript( const QString &id );

	void setScriptEnabled( Kopete::Account *account, const QString &scriptId, bool enabled );
	QList<JavaScriptFile *> scriptsFor( Kopete::Account *account );
	QList<JavaScriptFile *> allScripts() const;

	JavaScriptFile *script( const QString &id );

	void apply();

signals:
	void changed();

public slots:
	//For KNewStuff
	void installPackage( const QString &fileName, bool &retVal );

private:
	JavaScriptConfig( QObject * );

	static JavaScriptConfig *m_config;

	struct Private;
	Private * const d;
};

#endif

