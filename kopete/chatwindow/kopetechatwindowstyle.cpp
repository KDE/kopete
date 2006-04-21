 /*
    kopetechatwindowstyle.cpp - A Chat Window Style.

    Copyright (c) 2005      by Michaël Larouche     <michael.larouche@kdemail.net>

    Kopete    (c) 2002-2005 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "kopetechatwindowstyle.h"

// Qt includes
#include <QFile>
#include <QDir>
#include <QStringList>
#include <QTextStream>

// KDE includes
#include <kdebug.h>


class ChatWindowStyle::Private
{
public:
	QString stylePath;
	StyleVariants variantsList;
	QString baseHref;
	QString currentVariantPath;

	QString headerHtml;
	QString footerHtml;
	QString incomingHtml;
	QString nextIncomingHtml;
	QString outgoingHtml;
	QString nextOutgoingHtml;
	QString statusHtml;
	QString actionIncomingHtml;
	QString actionOutgoingHtml;
};

ChatWindowStyle::ChatWindowStyle(const QString &stylePath, StyleBuildMode styleBuildMode)
	: d(new Private)
{
	init(stylePath, styleBuildMode);
}

ChatWindowStyle::ChatWindowStyle(const QString &stylePath, const QString &variantPath, StyleBuildMode styleBuildMode)
	: d(new Private)
{
	d->currentVariantPath = variantPath;
	init(stylePath, styleBuildMode);
}

void ChatWindowStyle::init(const QString &stylePath, StyleBuildMode styleBuildMode)
{
	d->stylePath = stylePath;
	d->baseHref = stylePath + QString::fromUtf8("/Contents/Resources/");
	readStyleFiles();
	if(styleBuildMode & StyleBuildNormal)
	{
		listVariants();
	}
}

ChatWindowStyle::~ChatWindowStyle()
{
	kDebug(14000) << k_funcinfo << endl;
	delete d;
}

ChatWindowStyle::StyleVariants ChatWindowStyle::getVariants()
{
	// If the variantList is empty, list available variants.
	if( d->variantsList.isEmpty() )
	{
		listVariants();
	}
	return d->variantsList;
}

QString ChatWindowStyle::getStylePath() const
{
	return d->stylePath;
}

QString ChatWindowStyle::getStyleBaseHref() const
{
	return d->baseHref;
}

QString ChatWindowStyle::getHeaderHtml() const
{
	return d->headerHtml;
}

QString ChatWindowStyle::getFooterHtml() const
{
	return d->footerHtml;
}

QString ChatWindowStyle::getIncomingHtml() const
{
	return d->incomingHtml;
}

QString ChatWindowStyle::getNextIncomingHtml() const
{
	return d->nextIncomingHtml;
}

QString ChatWindowStyle::getOutgoingHtml() const
{
	return d->outgoingHtml;
}

QString ChatWindowStyle::getNextOutgoingHtml() const
{
	return d->nextOutgoingHtml;
}

QString ChatWindowStyle::getStatusHtml() const
{
	return d->statusHtml;
}

QString ChatWindowStyle::getActionIncomingHtml() const
{
	return d->actionIncomingHtml;	
}

QString ChatWindowStyle::getActionOutgoingHtml() const
{
	return d->actionOutgoingHtml;
}

bool ChatWindowStyle::hasActionTemplate() const
{
	return ( !d->actionIncomingHtml.isEmpty() && !d->actionOutgoingHtml.isEmpty() );
}

void ChatWindowStyle::listVariants()
{
	QString variantDirPath = d->baseHref + QString::fromUtf8("Variants/");
	QDir variantDir(variantDirPath);

	QStringList variantList = variantDir.entryList("*.css");
	QStringList::ConstIterator it, itEnd = variantList.constEnd();
	for(it = variantList.constBegin(); it != itEnd; ++it)
	{
		QString variantName = *it, variantPath;
		// Retrieve only the file name.
		variantName = variantName.left(variantName.findRev("."));
		// variantPath is relative to baseHref.
		variantPath = QString("Variants/%1").arg(*it);
		d->variantsList.insert(variantName, variantPath);
	}
}

void ChatWindowStyle::readStyleFiles()
{
	QString headerFile = d->baseHref + QString("Header.html");
	QString footerFile = d->baseHref + QString("Footer.html");
	QString incomingFile = d->baseHref + QString("Incoming/Content.html");
	QString nextIncomingFile = d->baseHref + QString("Incoming/NextContent.html");
	QString outgoingFile = d->baseHref + QString("Outgoing/Content.html");
	QString nextOutgoingFile = d->baseHref + QString("Outgoing/NextContent.html");
	QString statusFile = d->baseHref + QString("Status.html");
	QString actionIncomingFile = d->baseHref + QString("Incoming/Action.html");
	QString actionOutgoingFile = d->baseHref + QString("Outgoing/Action.html");

	QFile fileAccess;
	// First load header file.
	if( QFile::exists(headerFile) )
	{
		fileAccess.setFileName(headerFile);
		fileAccess.open(QIODevice::ReadOnly);
		QTextStream headerStream(&fileAccess);
		headerStream.setCodec(QTextCodec::codecForName("UTF-8"));
		d->headerHtml = headerStream.readAll();
		kDebug(14000) << k_funcinfo << "Header HTML: " << d->headerHtml << endl;
		fileAccess.close();
	}
	// Load Footer file
	if( QFile::exists(footerFile) )
	{
		fileAccess.setFileName(footerFile);
		fileAccess.open(QIODevice::ReadOnly);
		QTextStream headerStream(&fileAccess);
		headerStream.setCodec(QTextCodec::codecForName("UTF-8"));
		d->footerHtml = headerStream.readAll();
		kDebug(14000) << k_funcinfo << "Footer HTML: " << d->footerHtml << endl;
		fileAccess.close();
	}
	// Load incoming file
	if( QFile::exists(incomingFile) )
	{
		fileAccess.setFileName(incomingFile);
		fileAccess.open(QIODevice::ReadOnly);
		QTextStream headerStream(&fileAccess);
		headerStream.setCodec(QTextCodec::codecForName("UTF-8"));
		d->incomingHtml = headerStream.readAll();
		kDebug(14000) << k_funcinfo << "Incoming HTML: " << d->incomingHtml << endl;
		fileAccess.close();
	}
	// Load next Incoming file
	if( QFile::exists(nextIncomingFile) )
	{
		fileAccess.setFileName(nextIncomingFile);
		fileAccess.open(IO_ReadOnly);
		QTextStream headerStream(&fileAccess);
		headerStream.setCodec(QTextCodec::codecForName("UTF-8"));
		d->nextIncomingHtml = headerStream.readAll();
		kDebug(14000) << k_funcinfo << "NextIncoming HTML: " << d->nextIncomingHtml << endl;
		fileAccess.close();
	}
	// Load outgoing file
	if( QFile::exists(outgoingFile) )
	{
		fileAccess.setFileName(outgoingFile);
		fileAccess.open(QIODevice::ReadOnly);
		QTextStream headerStream(&fileAccess);
		headerStream.setCodec(QTextCodec::codecForName("UTF-8"));
		d->outgoingHtml = headerStream.readAll();
		kDebug(14000) << k_funcinfo << "Outgoing HTML: " << d->outgoingHtml << endl;
		fileAccess.close();
	}
	// Load next outgoing file
	if( QFile::exists(nextOutgoingFile) )
	{
		fileAccess.setFileName(nextOutgoingFile);
		fileAccess.open(IO_ReadOnly);
		QTextStream headerStream(&fileAccess);
		headerStream.setCodec(QTextCodec::codecForName("UTF-8"));
		d->nextOutgoingHtml = headerStream.readAll();
		kDebug(14000) << k_funcinfo << "NextOutgoing HTML: " << d->nextOutgoingHtml << endl;
		fileAccess.close();
	}
	// Load status file
	if( QFile::exists(statusFile) )
	{
		fileAccess.setFileName(statusFile);
		fileAccess.open(QIODevice::ReadOnly);
		QTextStream headerStream(&fileAccess);
		headerStream.setCodec(QTextCodec::codecForName("UTF-8"));
		d->statusHtml = headerStream.readAll();
		kDebug(14000) << k_funcinfo << "Status HTML: " << d->statusHtml << endl;
		fileAccess.close();
	}
	
	// Load Action Incoming file
	if( QFile::exists(actionIncomingFile) )
	{
		fileAccess.setFileName(actionIncomingFile);
		fileAccess.open(IO_ReadOnly);
		QTextStream headerStream(&fileAccess);
		headerStream.setCodec(QTextCodec::codecForName("UTF-8"));
		d->actionIncomingHtml = headerStream.readAll();
		kDebug(14000) << k_funcinfo << "ActionIncoming HTML: " << d->actionIncomingHtml << endl;
		fileAccess.close();
	}
	// Load Action Outgoing file
	if( QFile::exists(actionOutgoingFile) )
	{
		fileAccess.setFileName(actionOutgoingFile);
		fileAccess.open(IO_ReadOnly);
		QTextStream headerStream(&fileAccess);
		headerStream.setCodec(QTextCodec::codecForName("UTF-8"));
		d->actionOutgoingHtml = headerStream.readAll();
		kDebug(14000) << k_funcinfo << "ActionOutgoing HTML: " << d->actionOutgoingHtml << endl;
		fileAccess.close();
	}
}

void ChatWindowStyle::reload()
{
	d->variantsList.clear();
	readStyleFiles();
	listVariants();
}
