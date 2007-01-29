/*
    oscarversionupdater.h  -  Version Updater

    Copyright (c) 2006 by Roman Jarosz <kedgedev@centrum.cz>
    Kopete    (c) 2006 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef OSCARVERSIONUPDATER_H
#define OSCARVERSIONUPDATER_H

#include <QtCore/QObject>

#include <oscartypes.h>
#include <kconfiggroup.h>

class KJob;

namespace KIO
{
class Job;
class TransferJob;
}

class QDomElement;
class QDomDocument;

/**
	@author Roman Jarosz <kedgedev@centrum.cz>
*/

class OscarVersionUpdater : public QObject
{
	Q_OBJECT
	
public:
	OscarVersionUpdater();
	~OscarVersionUpdater();
	
	static OscarVersionUpdater* self();
	
	/**
	 * Update version info from server.
	 * @param stamp is update number.
	 */
	bool update( unsigned int stamp );
	
	/**
	 * Update version info from server.
	 * @return true if update is in progress or starts.
	 */
	unsigned int stamp() const;
	
	/**
	 * Return structure with version info for ICQ.
	 * @return Oscar::ClientVersion.
	 */
	const Oscar::ClientVersion* getICQVersion() const { return &mICQVersion; }
	
	/**
	 * Return structure with version info for AIM.
	 * @return Oscar::ClientVersion.
	 */
	const Oscar::ClientVersion* getAIMVersion() const { return &mAIMVersion; }
	
	/**
	 * Set structure with ICQ version info to default.
	 */
	void initICQVersionInfo();
	
	/**
	 * Set structure with AIM version info to default.
	 */
	void initAIMVersionInfo();
	
	/**
	 * Print debug info.
	 */
	void printDebug();

private slots:
	void slotTransferData( KIO::Job *job, const QByteArray &data );
	void slotTransferResult( KJob *job );
	
private:
	void parseDocument( QDomDocument& doc );
	bool parseVersion( Oscar::ClientVersion& version, QDomElement& element );
	
	/**
	 * Store version info structure to KConfigGroup
	 * @param group is the group name.
	 * @param version is version info structure.
	 */
	void storeVersionInfo( const QString& group, const Oscar::ClientVersion& version ) const;
	
	/**
	 * Compare two versions.
	 * @return true if a and b is equal.
	 */
	bool equal( const Oscar::ClientVersion& a, const Oscar::ClientVersion& b ) const;
	
private:
	static OscarVersionUpdater *versionUpdaterStatic;
	
	Oscar::ClientVersion mICQVersion;
	Oscar::ClientVersion mAIMVersion;
	
	KIO::TransferJob *mTransferJob;
	QByteArray mVersionData;
	
	unsigned int mStamp;
	bool mUpdating;
};

#endif
