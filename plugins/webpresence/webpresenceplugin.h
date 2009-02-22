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

#include <QtCore/QList>

#include <kurl.h>

#include "kopeteplugin.h"

class QTimer;
class KTemporaryFile;
class KJob;
namespace Kopete { class MetaContact; }
namespace Kopete { class Protocol; }
namespace Kopete { class Account; }
namespace Kopete { class OnlineStatus; }

typedef QList<Kopete::Protocol*> ProtocolList;

class WebPresencePlugin : public Kopete::Plugin
{
	Q_OBJECT

private:
	KUrl userStyleSheet;

	// Is set to true when Kopete has notified us
	// that we're about to be unloaded.
	bool shuttingDown;

	enum {
		WEB_HTML,
		WEB_XHTML,
		WEB_XML,
		WEB_CUSTOM,
		WEB_UNDEFINED
	} resultFormatting;

public:
	WebPresencePlugin( QObject *parent, const QVariantList &args );
	virtual ~WebPresencePlugin();

	virtual void aboutToUnload();

protected slots:
	/**
	 * Called when settings were changed
	 */
	void slotSettingsChanged();
	/**
	 * Write a file to the specified location,
	 */
	void slotWriteFile();
	/**
	 * Called when an upload finished, displays error if needed
	 */
	 void slotUploadJobResult( KJob * );
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
	KTemporaryFile* generateFile();
	/**
	* Apply named stylesheet to get content and presentation
	*/
	bool transform( KTemporaryFile* src, KTemporaryFile* dest );
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
	KTemporaryFile *m_output;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:
