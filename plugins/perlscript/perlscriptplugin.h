/*
    perlplugin.h

    Kopete Perl Scriping plugin

    Copyright (c) 2003 by Jason Keirstead   <jason@keirstead.org>

    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef PERLPLUGIN_H

#include <EXTERN.h>
#include <perl.h>

#include <qmap.h>

#include "kopetemessage.h"
#include "kopeteplugin.h"

class KopeteMetaContact;
class KopeteMessageManager;
class KActionCollection;
class QStringList;
class KTempFile;
class PerlScriptPreferences;

class PerlScript
{
	public:
		PerlScript( const QString &scriptPath, const QString &scriptName, const QString &desc );
		~PerlScript();
		QString path;
		QString name;
		QString description;
		QString scriptText;
		QStringList *tempArgs;
		void load();
		
	private:
		KTempFile *m_localFile;
};

class PerlPlugin : public KopetePlugin
{
	Q_OBJECT

	public:
		static PerlPlugin  *plugin();

		PerlPlugin( QObject *parent, const char *name, const QStringList &args );
		~PerlPlugin();
		PerlScript *script(  const QString &scriptPath );
		virtual KActionCollection *customContextMenuActions(KopeteMetaContact*);

	private:
		static PerlPlugin* pluginStatic_;
		QStringList getArguments( KopeteMessage &msg );
		void setHeader();
		
		QPtrList<PerlScript> m_incomingScripts;
		QPtrList<PerlScript> m_outgoingScripts;
		QPtrList<PerlScript> m_actionScripts;
		QMap<QString,PerlScript*> m_allScripts;
		PerlInterpreter *my_perl;
		PerlScriptPreferences *m_prefs;
		QString HeaderScript;

	private slots: 
		void slotIncomingMessage( KopeteMessage& msg );
		void slotOutgoingMessage( KopeteMessage& msg );
		void slotAddScript( const QString &scriptPath, const QString &scriptName, const QString &desc );
		void slotRemoveScript( const QString &scriptPath );
		void slotClearScripts();
		void slotScriptModified( const QString &scriptPath );
		void slotContextScript( const QString &scriptPath );
		void executeScript( const QString &scriptText, const QString &subName, QStringList &args, KopeteMessage *msg = 0L );
		
	signals:
		void scriptExecuted( const QString &scriptPath, const QString &scriptName );

};


#endif


