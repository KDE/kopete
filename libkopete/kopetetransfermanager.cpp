/***************************************************************************
                          kopetetransfermanager.cpp  -  description
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

#include "kopetetransfermanager.h"
#include "kopetefileconfirmdialog.h"

#include "kopete.h"
#include "kopetecontactlist.h"
#include "kopeteprotocol.h"
#include "kopetemetacontact.h"
#include "kopetecontact.h"

#include <klocale.h>
#include <kpushbutton.h>
#include <kdialogbase.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qfontmetrics.h>


/***************************
 *  KopeteFileTransferInfo *
 ***************************/

KopeteFileTransferInfo::KopeteFileTransferInfo(  KopeteContact *contact, const QString& file, const unsigned long size, const QString &recipient, KopeteTransferDirection di, const unsigned int id, void *internalId)
{
	mContact = contact;
	mFile = file;
	mId = id;
	mSize = size;
	mRecipient = recipient;
	m_intId= internalId;
	mDirection= di;
}

/***************************
 *     KopeteTransfer      *
 ***************************/


KopeteTransfer::KopeteTransfer( const KopeteFileTransferInfo &kfti, QObject *parent, const char *name)
	: QObject(parent, name),
	  QListViewItem( KopeteTransferManager::transferManager()->mListView)
{
//	if (!kfti)
//		kfti = new KopeteFileTransferInfo(0L, QString("Unknown"), 0, QString("Unknown"), 0); // icky
	mInfo = kfti;
	setText(0, kfti.file());
	setText(1, kfti.recipient());
	setText(2, QString::number(kfti.size()));
	setText(3, i18n("Waiting"));
	listView()->setColumnWidth(4, 150);
	slotPercentCompleted(0);
}

void KopeteTransfer::slotPercentCompleted(unsigned int percent)
{
	if (percent == 100)
		setText(3, i18n("Finished"));
	else
		setText(3, i18n("Transferring"));

	KopeteTransferManager::transferManager()->paintProgressBar(this, percent);
}

void KopeteTransfer::setError(KopeteTransferError error)
{
	QString errorString;
	switch (error)
	{
		case CanceledLocal:
			errorString = i18n("Aborted");
			emit transferCanceled();
			break;
		case CanceledRemote:
			errorString = i18n("Remote user aborted");
			break;
		case Timeout:
			errorString = i18n("Connection timed out");
			break;
		case Other:
		default:
			errorString = i18n("Unknown error occurred");
			break;
	}
	setText(3, errorString);
}

/***************************
 *  KopeteTransferManager  *
 ***************************/

KopeteTransferManager* KopeteTransferManager::s_transferManager = 0L;

KopeteTransferManager* KopeteTransferManager::transferManager()
{
	if( !s_transferManager )
		s_transferManager = new KopeteTransferManager( 0L );

	//FIXME: the transfer manager is never deleted!
	//	we can't add a parent because it is a widget
	//	and it shouldn't appaers in it

	return s_transferManager;
}

KopeteTransferManager::KopeteTransferManager( QWidget *parent )
	: KopeteFileTransferUI( parent )
{
	nextID = 0;
	hide();
	connect(cmdAbort, SIGNAL(clicked()), this, SLOT(slotAbortClicked()));
	connect(cmdRmComplete, SIGNAL(clicked()), this, SLOT(slotClearFinished()));
}

void KopeteTransferManager::paintProgressBar(QListViewItem *item, const int currentPercent)
{
	int width = (item->listView()->columnWidth(4) -4);
	QPixmap pixmap(width, (item->height() - 4));
	pixmap.fill(QColor(255,255,255));
	QPainter p(&pixmap);
	int fillWidth = static_cast<int>(width * ( (double)currentPercent/100 ) );

	// This makes the percentage bar colored coded according to how far it is in the transfer
	QColor blendColor;
	blendColor.setHsv(currentPercent, 255, 204); // Oh, I know, I'm good :)
	p.fillRect(0, 0, fillWidth, item->height(), blendColor);

	// The following is to align and draw the percentage text in the center
	QString percentText = QString(QString::number(currentPercent) + "%");
	int textWidth = (item->listView()->fontMetrics().width(percentText) / 2);
	int newWidth = ( width / 2 );
	int startPosY = static_cast<int>(((double)item->height() /2) + 2);
	int startPosX = newWidth - textWidth;
	p.drawText(startPosX, startPosY, percentText);

	p.end();
	item->setPixmap(4, pixmap);
}


KopeteTransfer* KopeteTransferManager::addTransfer(  KopeteContact *contact, const QString& file, const unsigned long size, const QString &recipient , KopeteFileTransferInfo::KopeteTransferDirection di)
{
//	if (nextID != 0)
		nextID++;
	KopeteFileTransferInfo info(contact, file, size, recipient,di,  nextID);
	KopeteTransfer *trans = new KopeteTransfer(info, this, "KopeteTransfer");
	connect(trans, SIGNAL(done(KopeteTransfer *)), this, SIGNAL(done(KopeteTransfer *))); // Just for handiness
	mTransfersMap.insert(nextID, trans);
	mListView->insertItem(trans);
	show();
	return trans;
}

int KopeteTransferManager::askIncommingTransfer(  KopeteContact *contact, const QString& file, const unsigned long size, const QString& description, void *internalId)
{
//	if (nextID != 0)
		nextID++;
	KopeteFileTransferInfo info(contact, file, size, contact->metaContact()->displayName(), KopeteFileTransferInfo::Incomming , nextID , internalId);

	KopeteFileConfirmDialog *diag= new KopeteFileConfirmDialog(info, description , this )  ;

	connect( diag, SIGNAL( accepted(const KopeteFileTransferInfo&, const QString&)) , this, SLOT( slotAccepted(const KopeteFileTransferInfo&, const QString&) ) );
	connect( diag, SIGNAL( refused(const KopeteFileTransferInfo&)) , this, SIGNAL( refused(const KopeteFileTransferInfo&) ) );
	diag->show();
	return nextID;
}

void KopeteTransferManager::removeTransfer( unsigned int id )
{
	KopeteTransfer *trans = mTransfersMap[id];
	mTransfersMap.remove(id);
	mListView->takeItem(trans);
	if (mListView->childCount() == 0)
		hide();
	if (trans)
		delete trans;
}

void KopeteTransferManager::slotAbortClicked()
{
	QListViewItem *curr = mListView->currentItem();
	if (!curr) return;
	KopeteTransfer *trans = dynamic_cast<KopeteTransfer *>(curr);
	if (!trans) return;
	trans->setError(KopeteTransfer::CanceledLocal);
	emit canceled(trans);
}

void KopeteTransferManager::slotClearFinished()
{
	for (QListViewItem *it = mListView->firstChild(); it != 0L; it = it->itemBelow())
	{
		if (it->text(3) != i18n("Transferring"))
		{
			KopeteTransfer *trans = dynamic_cast<KopeteTransfer *>(it);
			if (!trans) continue;
			emit canceled(trans);
			delete trans;
		}
	}
}

void KopeteTransferManager::slotAccepted(const KopeteFileTransferInfo& info, const QString& filename)
{
	KopeteTransfer *trans = new KopeteTransfer(info, this, "KopeteTransfer");
	connect(trans, SIGNAL(done(KopeteTransfer *)), this, SIGNAL(done(KopeteTransfer *))); // Just for handiness
	mTransfersMap.insert(info.id(), trans);
	mListView->insertItem(trans);
	show();
	emit accepted(trans,filename);
}


#include "kopetetransfermanager.moc"

