/*
    webpresenceplugin.h

    Kopete Web Presence plugin

    Copyright (c) 2002 by Will Stephenson <will@stevello.free-online.co.uk>

    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                    	*
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef WEBPRESENCEPLUGIN_H
#define WEBPRESENCEPLUGIN_H

#include "kopeteplugin.h"
#include "kopetecontact.h"
#include <qptrlist.h>
#include <kio/job.h>

class QTimer;
class KTempFile;
class KopeteMetaContact;
class KToggleAction;
class KActionCollection;
class WebPresencePreferences;

class WebPresencePlugin : public KopetePlugin
{
	Q_OBJECT
	struct ProtoContactStatus {
		const char *name;
		const char *id;
		KopeteContact::ContactStatus status;
	};
	public:
		WebPresencePlugin( QObject *parent, const char *name, const QStringList &args );
		virtual ~WebPresencePlugin();

	public slots:
		/**
		 * Apply updated preference dialog settings
		 */
		void slotSettingsChanged();
	protected slots:
		/**
		 * Write a file to the specified location
		 */
		void slotWriteFile();
		/**
		 * Called when an upload finished, displays error if needed
		 */
		 void slotUploadJobResult( KIO::Job * );
	protected:
		/**
		 * Generate the file (HTML, text) to be uploaded
		 */	
		KTempFile* generateFile();
		/**
		* Apply named stylesheet to get content and presentation
		*/
		bool transform( KTempFile* src, KTempFile* dest );
		/** 
		 * Helper method, generates list of all IM protocols
		 */
		QPtrList<KopeteProtocol> allProtocols();
		/**
		 * Converts numeric status to a string
		 */
		QString statusAsString( KopeteContact::ContactStatus c );
		// Triggers a write of the current contactlist
		QTimer *m_timer;
		// Interface to the preferences GUI
		WebPresencePreferences* m_prefs;
		// The file to be uploaded to the WWW
		KTempFile *m_output;

};

#endif

// vim: set noet ts=4 sts=4 sw=4:
