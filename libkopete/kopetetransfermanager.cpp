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

#include "kopete.h"
#include "kopetecontactlist.h"
#include "kopeteprotocol.h"
#include "kopetemetacontact.h"

#include <klocale.h>
#include <kpushbutton.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qfontmetrics.h>

KopeteFileTransferInfo::KopeteFileTransferInfo( const KopeteMetaContact *contact, const QString& file, const unsigned long size, const QString &recipient, const unsigned int id)
{
	mContact = contact;
	mFile = file;
	mId = id;
	mSize = size;
	mRecipient = recipient;
}

KopeteTransfer::KopeteTransfer( KopeteFileTransferInfo *kfti, QObject *parent, const char *name)
	: QObject(parent, name),
	  QListViewItem(kopeteapp->transferManager()->mListView)
{
	if (!kfti)
		delete this;
	mInfo = kfti;
	setText(0, kfti->file());
	setText(1, kfti->recipient());
	setText(2, QString::number(kfti->size()));
	setText(3, i18n("Waiting"));
	listView()->setColumnWidth(4, 150);
	slotPercentCompleted(0);
}

void KopeteTransfer::slotPercentCompleted(unsigned int percent)
{
	if (percent == 100)
		setText(3, i18n("Finished"));
	else
		setText(3, i18n("Transfering"));
		
	kopeteapp->transferManager()->paintProgressBar(this, percent);
}

KopeteTransferManager::KopeteTransferManager()
	: KopeteFileTransferUI()
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
	int fillWidth = (width * ( (double)currentPercent/100 ) );
	
	// This makes the percentage bar colored coded according to how far it is in the transfer
	QColor blendColor;
	blendColor.setHsv(currentPercent, 255, 204); // Oh, I know, I'm good :)
	p.fillRect(0, 0, fillWidth, item->height(), blendColor);
	
	// The following is to align and draw the percentage text in the center
	QString percentText = QString(QString::number(currentPercent) + "%");
	int textWidth = (item->listView()->fontMetrics().width(percentText) / 2);
	int newWidth = ( width / 2 );
	int startPosY = ((double)item->height() /2) + 2;
	int startPosX = newWidth - textWidth;
	p.drawText(startPosX, startPosY, percentText);
	
	p.end();
	item->setPixmap(4, pixmap);
}

void KopeteTransfer::setError(KopeteTransferError error)
{
	QString errorString;
	switch (error)
	{
		case CanceledLocal:
			errorString = i18n("Aborted");
			break;
		case CanceledRemote:
			errorString = i18n("Remote user aborted");
			break;
		case Timeout:
			errorString = i18n("Connection timed out");
			break;
		case Other:
		default:
			errorString = i18n("Unknown error occured");
			break;
	}
	setText(3, errorString);
}

KopeteTransfer* KopeteTransferManager::addTransfer( const KopeteMetaContact *contact, const QString& file, const unsigned long size, const QString &recipient )
{
	if (nextID != 0)
		nextID++;
	KopeteFileTransferInfo *info = new KopeteFileTransferInfo(contact, file, size, recipient, nextID);
	KopeteTransfer *trans = new KopeteTransfer(info, this, "KopeteTransfer");
	connect(trans, SIGNAL(done(KopeteTransfer *)), this, SIGNAL(done(KopeteTransfer *))); // Just for handiness
	mTransfersMap.insert(nextID, trans);
	mListView->insertItem(trans);
	show();
	return trans;
}

void KopeteTransferManager::removeTransfer( const KopeteFileTransferInfo *transinfo )
{
	KopeteTransfer *trans = mTransfersMap[transinfo->id()];
	mTransfersMap.remove(transinfo->id());
	mListView->takeItem(trans);
	if (mListView->childCount() == 0)
		hide();
	if (trans)
		delete trans;
	if (transinfo)
		delete transinfo;
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
		if (it->text(3) != i18n("Transfering"))
		{
			KopeteTransfer *trans = dynamic_cast<KopeteTransfer *>(it);
			if (!trans) continue;
			emit canceled(trans);
			delete trans;
		}
	}
}

#include "kopetetransfermanager.moc"
