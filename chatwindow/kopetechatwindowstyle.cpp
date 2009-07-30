 /*
    kopetechatwindowstyle.cpp - A Chat Window Style.

    Copyright (c) 2005      by Michaël Larouche     <larouche@kde.org>

    Kopete    (c) 2002-2008 by the Kopete developers <kopete-devel@kde.org>

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
#include <QHash>
#include <QStringList>
#include <QTextCodec>
#include <QTextStream>

// KDE includes
#include <kdebug.h>
#include <klocale.h>
#include <kstandarddirs.h>

class ChatWindowStyle::Private
{
public:
	QString styleName;
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
	QString fileTransferIncomingHtml;
	QString voiceClipIncomingHtml;
	QString outgoingStateSendingHtml;
	QString outgoingStateErrorHtml;
	QString outgoingStateSentHtml;
	QString outgoingStateUnknownHtml;

	QHash<QString, bool> compactVariants;
};

ChatWindowStyle::ChatWindowStyle(const QString &styleName, StyleBuildMode styleBuildMode)
	: d(new Private)
{
	init(styleName, styleBuildMode);
}

ChatWindowStyle::ChatWindowStyle(const QString &styleName, const QString &variantPath, StyleBuildMode styleBuildMode)
	: d(new Private)
{
	d->currentVariantPath = variantPath;
	init(styleName, styleBuildMode);
}

void ChatWindowStyle::init(const QString &styleName, StyleBuildMode
styleBuildMode)
{
    d->styleName = styleName;
    
    QStringList styleDirs = getStyleDirs(styleName);
    if( styleDirs.isEmpty())
    {
        kDebug(14000) << "Failed to find style" << styleName;
        return;
    }

    if(styleDirs.count() > 1)
    {
        kDebug(14000) << "found several styles with the same name. using first";
    }
    d->baseHref = styleDirs.at(0);
    kDebug(14000) << "Using style:" << d->baseHref;

    readStyleFiles();

    if(styleBuildMode & StyleBuildNormal)
    {
        listVariants();
    }
}

QStringList ChatWindowStyle::getStyleDirs(const QString &styleName) const
{
    QStringList styleDirs = KGlobal::dirs()->findDirs("appdata",
    QString("styles/%1/Contents/Resources/").arg(styleName));
    if( styleDirs.isEmpty() )
    {
        kDebug(14000) << "Failed to find style" << styleName;
        return QStringList();
    }

    return styleDirs;
}

ChatWindowStyle::~ChatWindowStyle()
{
	kDebug(14000) ;
	delete d;
}

bool ChatWindowStyle::isValid() const
{
	return ( !d->statusHtml.isEmpty() && !d->fileTransferIncomingHtml.isEmpty() && !d->nextIncomingHtml.isEmpty()
	         && !d->incomingHtml.isEmpty() && !d->nextOutgoingHtml.isEmpty() && !d->outgoingHtml.isEmpty() );
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

QString ChatWindowStyle::getStyleName() const
{
	return d->styleName;
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

QString ChatWindowStyle::getFileTransferIncomingHtml() const
{
	return d->fileTransferIncomingHtml;
}

QString ChatWindowStyle::getVoiceClipIncomingHtml() const
{
	return d->voiceClipIncomingHtml;
}

QString ChatWindowStyle::getOutgoingStateSendingHtml() const
{
	return d->outgoingStateSendingHtml;
}

QString ChatWindowStyle::getOutgoingStateSentHtml() const
{
	return d->outgoingStateSentHtml;
}

QString ChatWindowStyle::getOutgoingStateErrorHtml() const
{
	return d->outgoingStateErrorHtml;
}

QString ChatWindowStyle::getOutgoingStateUnknownHtml() const
{
	return d->outgoingStateUnknownHtml;
}

bool ChatWindowStyle::hasActionTemplate() const
{
	return ( !d->actionIncomingHtml.isEmpty() && !d->actionOutgoingHtml.isEmpty() );
}

void ChatWindowStyle::listVariants()
{
    QDir variantDir( getVariantDirPath() );

    QStringList variantList = variantDir.entryList( QStringList("*.css") );
    QStringList::ConstIterator it, itEnd = variantList.constEnd();

    QLatin1String compactVersionPrefix("_compact_");
    for(it = variantList.constBegin(); it != itEnd; ++it)
    {
        QString variantName = *it;

        // Retrieve only the file name.
        variantName = variantName.left(variantName.lastIndexOf("."));

        if( variantName.startsWith( compactVersionPrefix ) )
        {
            addCompactVariantIfPresent(variantName,compactVersionPrefix);
        }
        else
        {
            addVariant(variantName);
        }
    }
}

QString ChatWindowStyle::getVariantDirPath() const
{
    return d->baseHref + QString::fromUtf8("Variants/");
}
void ChatWindowStyle::addCompactVariantIfPresent(const QString &variantName,const QLatin1String compactVersionPrefix)
{
    if( variantName.startsWith( compactVersionPrefix ) )
    {
        if( variantName == compactVersionPrefix )
        {
            d->compactVariants.insert( "", true );
        }
    }

    QString compactVersionFilename = variantName;
    QString compactVersionPath = getVariantDirPath() + compactVersionFilename.prepend( compactVersionPrefix );
    if( QFile::exists( compactVersionPath ))
    {
        d->compactVariants.insert( variantName, true );
    }
}

void ChatWindowStyle::addVariant(const QString &name)
{
    // variantPath is relative to baseHref.
    QString variantPath = QString("Variants/%1").arg(name);
    d->variantsList.insert(name, variantPath);
}
void ChatWindowStyle::loadStyleFile(QString &styleType, const QString &fileName)
{
    QFile fileAccess;
    if( QFile::exists(fileName) )
    {
        fileAccess.setFileName(fileName);
        fileAccess.open(QIODevice::ReadOnly);
        QTextStream headerStream(&fileAccess);
        headerStream.setCodec(QTextCodec::codecForName("UTF-8"));
        styleType = headerStream.readAll();
        kDebug(14000) << fileName << d->headerHtml;
        fileAccess.close();
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
	QString fileTransferIncomingFile = d->baseHref + QString("Incoming/FileTransferRequest.html");
	QString voiceClipIncomingFile = d->baseHref + QString("Incoming/voiceClipRequest.html");
	QString outgoingStateUnknownFile = d->baseHref + QString("Outgoing/StateUnknown.html");
	QString outgoingStateSendingFile = d->baseHref + QString("Outgoing/StateSending.html");
	QString outgoingStateSentFile = d->baseHref + QString("Outgoing/StateSent.html");
	QString outgoingStateErrorFile = d->baseHref + QString("Outgoing/StateError.html");

	QFile fileAccess;
    // First load header file.
    loadStyleFile(d->headerHtml,headerFile);
    // Load Footer file
    loadStyleFile(d->footerHtml,footerFile);
    // Load incoming file
    loadStyleFile(d->incomingHtml,incomingFile);
    // Load next Incoming file
    loadStyleFile(d->nextIncomingHtml,nextIncomingFile);
    // Load outgoing file
    loadStyleFile(d->outgoingHtml,outgoingFile);
    // Load next outgoing file
    loadStyleFile(d->nextOutgoingHtml,nextOutgoingFile);
    // Load status file
    loadStyleFile(d->statusHtml,statusFile);
    // Load Action Incoming file
    loadStyleFile(d->actionIncomingHtml,actionIncomingFile);
    // Load Action Outgoing file
    loadStyleFile(d->actionOutgoingHtml,actionOutgoingFile);
    // Load FileTransfer Incoming file
    loadStyleFile(d->fileTransferIncomingHtml,fileTransferIncomingFile);
    setDefaultFileTransferIncomingHtml();
	// Load VoiceClip Incoming file
    loadStyleFile(d->voiceClipIncomingHtml,voiceClipIncomingFile);

	if ( d->voiceClipIncomingHtml.isEmpty() ||
	     ( !d->voiceClipIncomingHtml.contains( "playVoiceHandlerId" ) &&
	       !d->voiceClipIncomingHtml.contains( "saveAsVoiceHandlerId" ) ) )
	{	// Create default html
		d->voiceClipIncomingHtml = d->incomingHtml;
		QString message = QString( "%message%\n"
		                           "<div>\n"
		                           " <div style=\"width:37px; float:left;\">\n"
		                           "  <img src=\"%fileIconPath%\" style=\"width:32px; height:32px; vertical-align:middle;\" />\n"
		                           " </div>\n"
		                           " <div>\n"
		                           "  <span>\n"
		                           "   <input id=\"%playVoiceHandlerId%\" type=\"button\" value=\"%1\">\n"
		                           "   <input id=\"%saveAsVoiceHandlerId%\" type=\"button\" value=\"%2\">\n"
		                           "  </span>\n"
		                           " </div>\n"
		                           "</div>" )
		                           .arg( i18n( "Play" ), i18n( "Save as" ) );
		d->voiceClipIncomingHtml.replace( QLatin1String("%message%"), message );
	}

}
void ChatWindowStyle::setDefaultFileTransferIncomingHtml()
{
    if ( d->fileTransferIncomingHtml.isEmpty() ||
         ( !d->fileTransferIncomingHtml.contains( "saveFileHandlerId" ) &&
           !d->fileTransferIncomingHtml.contains( "saveFileAsHandlerId" ) ) )
    {   // Create default html
        d->fileTransferIncomingHtml = d->incomingHtml;
        QString message = QString( "%message%\n"
                                   "<div>\n"
                                   " <div style=\"width:37px; float:left;\">\n"
                                   "  <img src=\"%fileIconPath%\" style=\"width:32px; height:32px; vertical-align:middle;\" />\n"
                                   " </div>\n"
                                   " <div>\n"
                                   "  <span><b>%fileName%</b> (%fileSize%)</span><br>\n"
                                   "  <span>\n"
                                   "   <input id=\"%saveFileAsHandlerId%\" type=\"button\" value=\"%1\">\n"
                                   "   <input id=\"%cancelRequestHandlerId%\" type=\"button\" value=\"%2\">\n"
                                   "  </span>\n"
                                   " </div>\n"
                                   "</div>" )
                                   .arg( i18n( "Download" ), i18n( "Cancel" ) );
        d->fileTransferIncomingHtml.replace( QLatin1String("%message%"), message );
    }

}
void ChatWindowStyle::reload()
{
	d->variantsList.clear();
	readStyleFiles();
	listVariants();
}

bool ChatWindowStyle::hasCompact( const QString & styleVariant ) const
{
	if ( d->compactVariants.contains( styleVariant ) ) {
		return d->compactVariants.value( styleVariant );
	}
	return false;
}

QString ChatWindowStyle::compact( const QString & styleVariant ) const
{
	QString compacted = styleVariant;
	if ( styleVariant.isEmpty() ) {
		return QLatin1String( "Variants/_compact_.css" );
	} else {
		return compacted.insert( compacted.lastIndexOf('/') + 1, QString("_compact_") );
	}
}
