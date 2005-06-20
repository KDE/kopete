/*
    webpresenceplugin.h

    Kopete Web Presence plugin

    Copyright (c) 2002,2003 by Will Stephenson <will@stevello.free-online.co.uk>

    Kopete    (c) 2002,2003 by the Kopete developers  <kopete-devel@kde.org>

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

#include <qvaluestack.h>

#include <kio/job.h>

#include "kopetecontact.h"
#include "kopeteonlinestatus.h"

class QTimer;
class KTempFile;
namespace Kopete { class MetaContact; }
class KToggleAction;
class KActionCollection;

typedef QValueList<Kopete::Protocol*> ProtocolList;

class WebPresencePlugin : public Kopete::Plugin
{
	Q_OBJECT

private:
	int frequency;
	QString url;
	bool showAddresses;
	bool useImName;
	QString userName;
	bool useDefaultStyleSheet;
	bool justXml;
	QString userStyleSheet;
		
	struct ProtoContactStatus
	{
		const char *name;
		const char *id;
		Kopete::OnlineStatus status;
	};

public:
	WebPresencePlugin( QObject *parent, const char *name, const QStringList &args );
	virtual ~WebPresencePlugin();

protected slots:
	void loadSettings();
	
	/**
	 * Write a file to the specified location,
	 */
	void slotWriteFile();
	/**
	 * Called when an upload finished, displays error if needed
	 */
	 void slotUploadJobResult( KIO::Job * );
	/**
	 * Called to schedule a write, after waiting to see if more changes
	 * occur (accounts tend to change status together)
	 */
	void slotWaitMoreStatusChanges();
	/**
	 * Sets us up to respond to account status changes
	 */
	void listenToAllAccounts();
	/**
	 * Sets us up to respond to a new account
	 */
	void listenToAccount( Kopete::Account* account );

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
	ProtocolList allProtocols();
	/**
	 * Converts numeric status to a string
	 */
	QString statusAsString( const Kopete::OnlineStatus &newStatus );
	/**
     * Schedules writes
	 */
	QTimer* m_writeScheduler;

	// The file to be uploaded to the WWW
	KTempFile *m_output;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:
