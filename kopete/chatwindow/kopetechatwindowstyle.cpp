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
#include <KGlobal>
#include <kdebug.h>
#include <KLocalizedString>
#include <kstandarddirs.h>
#include <QStandardPaths>

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
    : QObject()
    , d(new Private)
{
    init(styleName, styleBuildMode);
}

ChatWindowStyle::ChatWindowStyle(const QString &styleName, const QString &variantPath, StyleBuildMode styleBuildMode)
    : QObject()
    , d(new Private)
{
    d->currentVariantPath = variantPath;
    init(styleName, styleBuildMode);
}

void ChatWindowStyle::init(const QString &styleName, StyleBuildMode styleBuildMode)
{
    QStringList styleDirs = QStandardPaths::locateAll(QStandardPaths::DataLocation, QStringLiteral("/styles/%1/Contents/Resources/").arg(styleName),
                                                      QStandardPaths::LocateDirectory);
    if (styleDirs.isEmpty()) {
        kDebug(14000) << "Failed to find style" << styleName;
        return;
    }
    d->styleName = styleName;
    if (styleDirs.count() > 1) {
        kDebug(14000) << "found several styles with the same name. using first";
    }
    d->baseHref = styleDirs.at(0);
    kDebug(14000) << "Using style:" << d->baseHref;
    readStyleFiles();
    if (styleBuildMode & StyleBuildNormal) {
        listVariants();
    }
}

ChatWindowStyle::~ChatWindowStyle()
{
    kDebug(14000);
    delete d;
}

bool ChatWindowStyle::isValid() const
{
    return !d->statusHtml.isEmpty() && !d->fileTransferIncomingHtml.isEmpty() && !d->nextIncomingHtml.isEmpty()
           && !d->incomingHtml.isEmpty() && !d->nextOutgoingHtml.isEmpty() && !d->outgoingHtml.isEmpty();
}

ChatWindowStyle::StyleVariants ChatWindowStyle::getVariants()
{
    // If the variantList is empty, list available variants.
    if (d->variantsList.isEmpty()) {
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
    return !d->actionIncomingHtml.isEmpty() && !d->actionOutgoingHtml.isEmpty();
}

void ChatWindowStyle::listVariants()
{
    QString variantDirPath = d->baseHref + QLatin1String("Variants/");
    QDir variantDir(variantDirPath);

    QStringList variantList = variantDir.entryList(QStringList(QStringLiteral("*.css")));
    QStringList::ConstIterator it, itEnd = variantList.constEnd();
    QLatin1String compactVersionPrefix("_compact_");
    for (it = variantList.constBegin(); it != itEnd; ++it) {
        QString variantName = *it, variantPath;
        // Retrieve only the file name.
        variantName = variantName.left(variantName.lastIndexOf(QLatin1String(".")));
        if (variantName.startsWith(compactVersionPrefix)) {
            if (variantName == compactVersionPrefix) {
                d->compactVariants.insert(QLatin1String(""), true);
            }
            continue;
        }
        QString compactVersionFilename = *it;
        QString compactVersionPath = variantDirPath + compactVersionFilename.prepend(compactVersionPrefix);
        if (QFile::exists(compactVersionPath)) {
            d->compactVariants.insert(variantName, true);
        }
        // variantPath is relative to baseHref.
        variantPath = QStringLiteral("Variants/%1").arg(*it);
        d->variantsList.insert(variantName, variantPath);
    }
}

void ChatWindowStyle::readStyleFiles()
{
    QString headerFile = d->baseHref + QStringLiteral("Header.html");
    QString footerFile = d->baseHref + QStringLiteral("Footer.html");
    QString incomingFile = d->baseHref + QStringLiteral("Incoming/Content.html");
    QString nextIncomingFile = d->baseHref + QStringLiteral("Incoming/NextContent.html");
    QString outgoingFile = d->baseHref + QStringLiteral("Outgoing/Content.html");
    QString nextOutgoingFile = d->baseHref + QStringLiteral("Outgoing/NextContent.html");
    QString statusFile = d->baseHref + QStringLiteral("Status.html");
    QString actionIncomingFile = d->baseHref + QStringLiteral("Incoming/Action.html");
    QString actionOutgoingFile = d->baseHref + QStringLiteral("Outgoing/Action.html");
    QString fileTransferIncomingFile = d->baseHref + QStringLiteral("Incoming/FileTransferRequest.html");
    QString voiceClipIncomingFile = d->baseHref + QStringLiteral("Incoming/voiceClipRequest.html");
    QString outgoingStateUnknownFile = d->baseHref + QStringLiteral("Outgoing/StateUnknown.html");
    QString outgoingStateSendingFile = d->baseHref + QStringLiteral("Outgoing/StateSending.html");
    QString outgoingStateSentFile = d->baseHref + QStringLiteral("Outgoing/StateSent.html");
    QString outgoingStateErrorFile = d->baseHref + QStringLiteral("Outgoing/StateError.html");

    QFile fileAccess;
    // First load header file.
    if (QFile::exists(headerFile)) {
        fileAccess.setFileName(headerFile);
        fileAccess.open(QIODevice::ReadOnly);
        QTextStream headerStream(&fileAccess);
        headerStream.setCodec(QTextCodec::codecForName("UTF-8"));
        d->headerHtml = headerStream.readAll();
        kDebug(14000) << "Header HTML: " << d->headerHtml;
        fileAccess.close();
    }
    // Load Footer file
    if (QFile::exists(footerFile)) {
        fileAccess.setFileName(footerFile);
        fileAccess.open(QIODevice::ReadOnly);
        QTextStream headerStream(&fileAccess);
        headerStream.setCodec(QTextCodec::codecForName("UTF-8"));
        d->footerHtml = headerStream.readAll();
        kDebug(14000) << "Footer HTML: " << d->footerHtml;
        fileAccess.close();
    }
    // Load incoming file
    if (QFile::exists(incomingFile)) {
        fileAccess.setFileName(incomingFile);
        fileAccess.open(QIODevice::ReadOnly);
        QTextStream headerStream(&fileAccess);
        headerStream.setCodec(QTextCodec::codecForName("UTF-8"));
        d->incomingHtml = headerStream.readAll();
        kDebug(14000) << "Incoming HTML: " << d->incomingHtml;
        fileAccess.close();
    }
    // Load next Incoming file
    if (QFile::exists(nextIncomingFile)) {
        fileAccess.setFileName(nextIncomingFile);
        fileAccess.open(QIODevice::ReadOnly);
        QTextStream headerStream(&fileAccess);
        headerStream.setCodec(QTextCodec::codecForName("UTF-8"));
        d->nextIncomingHtml = headerStream.readAll();
        kDebug(14000) << "NextIncoming HTML: " << d->nextIncomingHtml;
        fileAccess.close();
    }
    // Load outgoing file
    if (QFile::exists(outgoingFile)) {
        fileAccess.setFileName(outgoingFile);
        fileAccess.open(QIODevice::ReadOnly);
        QTextStream headerStream(&fileAccess);
        headerStream.setCodec(QTextCodec::codecForName("UTF-8"));
        d->outgoingHtml = headerStream.readAll();
        kDebug(14000) << "Outgoing HTML: " << d->outgoingHtml;
        fileAccess.close();
    }
    // Load next outgoing file
    if (QFile::exists(nextOutgoingFile)) {
        fileAccess.setFileName(nextOutgoingFile);
        fileAccess.open(QIODevice::ReadOnly);
        QTextStream headerStream(&fileAccess);
        headerStream.setCodec(QTextCodec::codecForName("UTF-8"));
        d->nextOutgoingHtml = headerStream.readAll();
        kDebug(14000) << "NextOutgoing HTML: " << d->nextOutgoingHtml;
        fileAccess.close();
    }
    // Load status file
    if (QFile::exists(statusFile)) {
        fileAccess.setFileName(statusFile);
        fileAccess.open(QIODevice::ReadOnly);
        QTextStream headerStream(&fileAccess);
        headerStream.setCodec(QTextCodec::codecForName("UTF-8"));
        d->statusHtml = headerStream.readAll();
        kDebug(14000) << "Status HTML: " << d->statusHtml;
        fileAccess.close();
    }

    // Load Action Incoming file
    if (QFile::exists(actionIncomingFile)) {
        fileAccess.setFileName(actionIncomingFile);
        fileAccess.open(QIODevice::ReadOnly);
        QTextStream headerStream(&fileAccess);
        headerStream.setCodec(QTextCodec::codecForName("UTF-8"));
        d->actionIncomingHtml = headerStream.readAll();
        kDebug(14000) << "ActionIncoming HTML: " << d->actionIncomingHtml;
        fileAccess.close();
    }
    // Load Action Outgoing file
    if (QFile::exists(actionOutgoingFile)) {
        fileAccess.setFileName(actionOutgoingFile);
        fileAccess.open(QIODevice::ReadOnly);
        QTextStream headerStream(&fileAccess);
        headerStream.setCodec(QTextCodec::codecForName("UTF-8"));
        d->actionOutgoingHtml = headerStream.readAll();
        kDebug(14000) << "ActionOutgoing HTML: " << d->actionOutgoingHtml;
        fileAccess.close();
    }
    // Load FileTransfer Incoming file
    if (QFile::exists(fileTransferIncomingFile)) {
        fileAccess.setFileName(fileTransferIncomingFile);
        fileAccess.open(QIODevice::ReadOnly);
        QTextStream headerStream(&fileAccess);
        headerStream.setCodec(QTextCodec::codecForName("UTF-8"));
        d->fileTransferIncomingHtml = headerStream.readAll();
        kDebug(14000) << "fileTransferIncoming HTML: " << d->fileTransferIncomingHtml;
        fileAccess.close();
    }

    if (d->fileTransferIncomingHtml.isEmpty()
        || (!d->fileTransferIncomingHtml.contains(QLatin1String("saveFileHandlerId"))
            && !d->fileTransferIncomingHtml.contains(QLatin1String("saveFileAsHandlerId")))) { // Create default html
        d->fileTransferIncomingHtml = d->incomingHtml;
        QString message = QString("%message%\n"
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
                                  "</div>")
                          .arg(i18n("Download"), i18n("Cancel"));
        d->fileTransferIncomingHtml.replace(QLatin1String("%message%"), message);
    }

    // Load VoiceClip Incoming file
    if (QFile::exists(voiceClipIncomingFile)) {
        fileAccess.setFileName(voiceClipIncomingFile);
        fileAccess.open(QIODevice::ReadOnly);
        QTextStream headerStream(&fileAccess);
        headerStream.setCodec(QTextCodec::codecForName("UTF-8"));
        d->voiceClipIncomingHtml = headerStream.readAll();
        kDebug(14000) << "voiceClipIncoming HTML: " << d->voiceClipIncomingHtml;
        fileAccess.close();
    }

    if (d->voiceClipIncomingHtml.isEmpty()
        || (!d->voiceClipIncomingHtml.contains(QLatin1String("playVoiceHandlerId"))
            && !d->voiceClipIncomingHtml.contains(QLatin1String("saveAsVoiceHandlerId")))) { // Create default html
        d->voiceClipIncomingHtml = d->incomingHtml;
        QString message = QString("%message%\n"
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
                                  "</div>")
                          .arg(i18n("Play"), i18n("Save as"));
        d->voiceClipIncomingHtml.replace(QLatin1String("%message%"), message);
    }

    // Load outgoing file
    if (QFile::exists(outgoingStateUnknownFile)) {
        fileAccess.setFileName(outgoingStateUnknownFile);
        fileAccess.open(QIODevice::ReadOnly);
        QTextStream headerStream(&fileAccess);
        headerStream.setCodec(QTextCodec::codecForName("UTF-8"));
        d->outgoingStateUnknownHtml = headerStream.readAll();
        kDebug(14000) << "Outgoing StateUnknown HTML: " << d->outgoingStateUnknownHtml;
        fileAccess.close();
    }

    if (QFile::exists(outgoingStateSendingFile)) {
        fileAccess.setFileName(outgoingStateSendingFile);
        fileAccess.open(QIODevice::ReadOnly);
        QTextStream headerStream(&fileAccess);
        headerStream.setCodec(QTextCodec::codecForName("UTF-8"));
        d->outgoingStateSendingHtml = headerStream.readAll();
        kDebug(14000) << "Outgoing StateSending HTML: " << d->outgoingStateSendingHtml;
        fileAccess.close();
    }

    if (QFile::exists(outgoingStateSentFile)) {
        fileAccess.setFileName(outgoingStateSentFile);
        fileAccess.open(QIODevice::ReadOnly);
        QTextStream headerStream(&fileAccess);
        headerStream.setCodec(QTextCodec::codecForName("UTF-8"));
        d->outgoingStateSentHtml = headerStream.readAll();
        kDebug(14000) << "Outgoing StateSent HTML: " << d->outgoingStateSentHtml;
        fileAccess.close();
    }

    if (QFile::exists(outgoingStateErrorFile)) {
        fileAccess.setFileName(outgoingStateErrorFile);
        fileAccess.open(QIODevice::ReadOnly);
        QTextStream headerStream(&fileAccess);
        headerStream.setCodec(QTextCodec::codecForName("UTF-8"));
        d->outgoingStateErrorHtml = headerStream.readAll();
        kDebug(14000) << "Outgoing StateError HTML: " << d->outgoingStateErrorHtml;
        fileAccess.close();
    }
}

void ChatWindowStyle::reload()
{
    d->variantsList.clear();
    readStyleFiles();
    listVariants();
}

bool ChatWindowStyle::hasCompact(const QString &styleVariant) const
{
    if (d->compactVariants.contains(styleVariant)) {
        return d->compactVariants.value(styleVariant);
    }
    return false;
}

QString ChatWindowStyle::compact(const QString &styleVariant) const
{
    QString compacted = styleVariant;
    if (styleVariant.isEmpty()) {
        return QStringLiteral("Variants/_compact_.css");
    } else {
        return compacted.insert(compacted.lastIndexOf('/') + 1, QStringLiteral("_compact_"));
    }
}
