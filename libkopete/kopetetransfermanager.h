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

class KopeteFileTransferInfo;
class KopeteMetaContact;
class KopeteTransfer;

/**
 * @author Nick Betcher. <nbetcher@kde.org>
 *
 */

class KopeteTransferManager : public KopeteFileTransferUI
{
Q_OBJECT
public:
	KopeteTransferManager();
	
	KopeteTransfer *addTransfer( const KopeteMetaContact *contact, const QString& file, const unsigned long size, const QString &recipient );
	void removeTransfer( const KopeteFileTransferInfo * );
	void paintProgressBar(QListViewItem *item, const int currentPercent);
private slots:
	void slotAbortClicked();
	void slotClearFinished();
private:
	int nextID;
	QMap<unsigned int, KopeteTransfer *> mTransfersMap;
signals:
	void done( KopeteTransfer* );
	void canceled( KopeteTransfer* );
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
		Other
	};
	KopeteTransfer( KopeteFileTransferInfo *, QObject *parent = 0, const char *name=0);
	KopeteFileTransferInfo *info() { return mInfo; };
	void setError(KopeteTransferError error);
public slots:
	void slotPercentCompleted(unsigned int);
private:
	KopeteFileTransferInfo *mInfo;
	int mPercent;

//protected slots:
//	void percentCompleted( const KopeteFileTransferInfo *,int percentDone );
signals:
	void transferCanceled( const KopeteFileTransferInfo * );
	void done( KopeteTransfer* );
};

class KopeteFileTransferInfo
{
public:
	KopeteFileTransferInfo( const KopeteMetaContact *, const QString&, const unsigned long, const QString &, const unsigned int );
	unsigned int id() const { return mId; };
	const KopeteMetaContact* contact() const { return mContact; };
	QString file() const { return mFile; };
	const QString recipient() { return mRecipient; };
	const unsigned long size() { return mSize; };
private:
	unsigned long mSize;
	QString mRecipient;
	unsigned int mId;
	const KopeteMetaContact *mContact;
	QString mFile;
};

#endif
