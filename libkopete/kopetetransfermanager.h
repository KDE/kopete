/***************************************************************************
                          kopetetransfermanager.h  -  description
                             -------------------
    begin                : Sat Aug 3 2002
    copyright            : (C) 2002 by nbetcher
    email                : nbetcher@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 
#ifndef KOPETEFILETRANSFER_H
#define KOPETEFILETRANSFER_H

#include <qobject.h>
#include <klistview.h>
#include <qstring.h>
#include <qmap.h>

#include "kopetefiletransferui.h"

class KopeteTransfer;
class KopeteContact;

/**
 * @author Nick Betcher. <nbetcher@kde.org>
 */
class KopeteFileTransferInfo
{
public:
	enum KopeteTransferDirection { Incomming, Outgoing };

	KopeteFileTransferInfo( KopeteContact *, const QString&, const unsigned long size, const QString &,KopeteFileTransferInfo::KopeteTransferDirection di, const unsigned int id, void *internalId=0L);
	~KopeteFileTransferInfo(){};
	KopeteFileTransferInfo(){};
	unsigned int transferId() const { return mId; };
	KopeteContact* contact() const { return mContact; };
	QString file() const { return mFile; };
	QString recipient() const { return mRecipient; };
	unsigned long size() const { return mSize; };
	void *internalId() const { return m_intId; };
	KopeteTransferDirection direction() const { return mDirection; };

private:
	unsigned long mSize;
	QString mRecipient;
	unsigned int mId;
	KopeteContact *mContact;
	QString mFile;
	void *m_intId;
	KopeteTransferDirection mDirection;
};

class KopeteTransferManager : public KopeteFileTransferUI
{
	Q_OBJECT

public:
	/**
	 * Retrieve the transfer manager instance
	 */
	static KopeteTransferManager* transferManager();

	~KopeteTransferManager(){};
	KopeteTransfer *addTransfer( KopeteContact *contact, const QString& file, const unsigned long size, const QString &recipient , KopeteFileTransferInfo::KopeteTransferDirection di);
	int askIncommingTransfer( KopeteContact *contact, const QString& file, const unsigned long size, const QString& description=QString::null, void *internalId=0L);
	void removeTransfer( unsigned int id );
	void paintProgressBar(QListViewItem *item, const int currentPercent);

signals:
	void done( KopeteTransfer* );
	void canceled( KopeteTransfer* );

	void accepted(KopeteTransfer*, const QString &fileName);
	void refused(const KopeteFileTransferInfo& );

private slots:
	void slotAbortClicked();
	void slotClearFinished();
	void slotAccepted(const KopeteFileTransferInfo&, const QString&);

private:
	KopeteTransferManager( QWidget *parent );

	static KopeteTransferManager *s_transferManager;
	int nextID;
	QMap<unsigned int, KopeteTransfer *> mTransfersMap;
};

class KopeteTransfer : public QObject, public QListViewItem
{
	Q_OBJECT

public:
	enum KopeteTransferError
	{
		CanceledLocal,
		CanceledRemote,
		Timeout,
		Refused,
		Other
	};
	KopeteTransfer( const KopeteFileTransferInfo &, QObject *parent = 0, const char *name=0);
	~KopeteTransfer(){};
	KopeteFileTransferInfo info() { return mInfo; };
	void setError(KopeteTransferError error);

public slots:
	void slotPercentCompleted(unsigned int);

signals:
	void transferCanceled(  );
	void done( KopeteTransfer* );

private:
	KopeteFileTransferInfo mInfo;
	int mPercent;

//protected slots:
//	void percentCompleted( const KopeteFileTransferInfo *,int percentDone );
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

