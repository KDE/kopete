/*
    yahoobuddyiconloader.h - Fetches YahooBuddyIcons

    Copyright (c) 2005 by André Duffeck <duffeck@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef YAHOOBUDDYICONLOADER_
#define	YAHOOBUDDYICONLOADER_

// QT Includes
#include <qobject.h>
#include <qstring.h>
#include <qmap.h>

// KDE Includes
#include <kurl.h>

class KJob;
namespace KYahoo {
	class Client;
}
namespace KIO {
	class Job;
	class TransferJob;
}

struct IconLoadJob {
	KUrl url;
	QString who;
	int checksum;
	QByteArray icon;
};

/**
 * @author André Duffeck
 *
 * This class handles the download of a Buddy icon.
 * If the download was succesfull it emits a signal with a pointer
 * to the temporary file, the icon was stored at
 */
class YahooBuddyIconLoader : public QObject
{
	Q_OBJECT
public:
	YahooBuddyIconLoader( KYahoo::Client *c );
	~YahooBuddyIconLoader();

	/**
	 *	Add a BuddyIcon for download.
	 */
	void fetchBuddyIcon( const QString &who, KUrl url, int checksum );

signals:
	/**
	 * 	The account can connect to this signal and append the icon
	 * 	stored in 'file' to the appropriate contact
	 */
	void fetchedBuddyIcon( const QString &who, const QByteArray &icon, int checksum );

private slots:
	void slotData( KIO::Job *job, const QByteArray &data );
	void slotComplete( KJob *job );

private:
	typedef QMap< KIO::TransferJob *, IconLoadJob > TransferJobMap;
	TransferJobMap m_jobs;
	KYahoo::Client *m_client;
};

#endif
