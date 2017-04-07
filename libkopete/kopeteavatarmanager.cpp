/*
    kopeteavatarmanager.cpp - Global avatar manager

    Copyright (c) 2007      by Michaël Larouche      <larouche@kde.org>

    Kopete    (c) 2002-2007 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/
#include "kopeteavatarmanager.h"

// Qt includes

#include <QBuffer>
#include <QFile>
#include <QPointer>
#include <QStringList>
#include <QDir>
#include <QDebug>
#include <QCryptographicHash>
#include <QUrl>
#include <QPainter>
#include <QImageReader>
#include <QStandardPaths>

// KDE includes
#include <KSharedConfig>
#include <kconfig.h>
#include <kcodecs.h>
#include <kio/job.h>
#include <kio/netaccess.h>

// Kopete includes
#include <kopetecontact.h>
#include <kopeteprotocol.h>
#include <kopeteaccount.h>

namespace Kopete {
//BEGIN AvatarManager
AvatarManager *AvatarManager::s_self = 0;

AvatarManager *AvatarManager::self()
{
    if (!s_self) {
        s_self = new AvatarManager;
    }
    return s_self;
}

class AvatarManager::Private
{
public:
    QUrl baseDir;

    /**
     * Create directory if needed
     * @param directory URL of the directory to create
     */
    void createDirectory(const QUrl &directory);

    /**
     * Scale the given image to 96x96.
     * @param source Original image
     */
    QImage scaleImage(const QImage &source);
};

static const QString UserDir(QStringLiteral("User"));
static const QString ContactDir(QStringLiteral("Contacts"));
static const QString AvatarConfig(QStringLiteral("avatarconfig.rc"));

AvatarManager::AvatarManager(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
    // Locate avatar data dir on disk
    const QString avatarPath = QStandardPaths::writableLocation(QStandardPaths::DataLocation) + QLatin1String("/avatars");
    d->createDirectory(QUrl::fromLocalFile(avatarPath));
    d->baseDir = QUrl::fromLocalFile(avatarPath);
}

AvatarManager::~AvatarManager()
{
    s_self = nullptr;
    delete d;
}

Kopete::AvatarManager::AvatarEntry AvatarManager::add(Kopete::AvatarManager::AvatarEntry newEntry)
{
    Q_ASSERT(!newEntry.name.isEmpty());

    QUrl avatarUrl = d->baseDir;

    // First find where to save the file
    switch (newEntry.category) {
    case AvatarManager::User:
        avatarUrl = avatarUrl.adjusted(QUrl::StripTrailingSlash);
        avatarUrl.setPath(avatarUrl.path() + '/' + (UserDir));
        d->createDirectory(avatarUrl);
        break;
    case AvatarManager::Contact:
        avatarUrl = avatarUrl.adjusted(QUrl::StripTrailingSlash);
        avatarUrl.setPath(avatarUrl.path() + '/' + (ContactDir));
        d->createDirectory(avatarUrl);
        // Use the account id for sub directory
        if (newEntry.contact && newEntry.contact->account()) {
            QString accountName = newEntry.contact->account()->accountId();
            QString protocolName = newEntry.contact->account()->protocol()->pluginId();
            avatarUrl = avatarUrl.adjusted(QUrl::StripTrailingSlash);
            avatarUrl.setPath(avatarUrl.path() + '/' + (protocolName));
            d->createDirectory(avatarUrl);
            avatarUrl = avatarUrl.adjusted(QUrl::StripTrailingSlash);
            avatarUrl.setPath(avatarUrl.path() + '/' + (accountName));
            d->createDirectory(avatarUrl);
        }
        break;
    default:
        break;
    }

    QUrl dataUrl(avatarUrl);

    qCDebug(LIBKOPETE_LOG) << "Base directory: " << avatarUrl.toLocalFile();

    // Second, open the avatar configuration in current directory.
    QUrl configUrl = avatarUrl;
    configUrl = configUrl.adjusted(QUrl::StripTrailingSlash);
    configUrl.setPath(configUrl.path() + '/' + (AvatarConfig));

    QByteArray data = newEntry.data;
    QImage avatar = newEntry.image;

    if (!data.isNull()) {
        avatar.loadFromData(data);
    } else if (!newEntry.dataPath.isEmpty()) {
        QFile f(newEntry.dataPath);
        f.open(QIODevice::ReadOnly);
        data = f.readAll();
        f.close();

        avatar.loadFromData(data);
    } else if (!avatar.isNull()) {
        QByteArray tempArray;
        QBuffer tempBuffer(&tempArray);
        tempBuffer.open(QIODevice::WriteOnly);
        avatar.save(&tempBuffer, "PNG");

        data = tempArray;
    } else if (!newEntry.path.isEmpty()) {
        avatar = QImage(newEntry.path);

        QFile f(newEntry.path);
        f.open(QIODevice::ReadOnly);
        data = f.readAll();
        f.close();
    } else {
        qCDebug(LIBKOPETE_LOG) << "Warning: No valid image source!";
    }

    // Scale avatar
    avatar = d->scaleImage(avatar);

    QString avatarFilename;

    // for the contact avatar, save it with the contactId + .png
    if (newEntry.category == AvatarManager::Contact && newEntry.contact) {
        avatarFilename = KIO::encodeFileName(newEntry.contact->contactId()) + ".png";
    } else {
        // Find MD5 hash for filename
        // MD5 always contain ASCII caracteres so it is more save for a filename.
        // Name can use UTF-8 characters.
        QByteArray tempArray;
        QBuffer tempBuffer(&tempArray);
        tempBuffer.open(QIODevice::WriteOnly);
        avatar.save(&tempBuffer, "PNG");
        QCryptographicHash context(QCryptographicHash::Md5);
        context.addData(tempArray);
        avatarFilename = context.result() + QLatin1String(".png");
    }

    // Save image on disk
    qCDebug(LIBKOPETE_LOG) << "Saving " << avatarFilename << " on disk.";
    avatarUrl = avatarUrl.adjusted(QUrl::StripTrailingSlash);
    avatarUrl.setPath(avatarUrl.path() + '/' + (avatarFilename));

    if (!avatar.save(avatarUrl.toLocalFile(), "PNG")) {
        qCDebug(LIBKOPETE_LOG) << "Saving of scaled avatar to " << avatarUrl.toLocalFile() << " failed !";
        return AvatarEntry();
    }

    QString dataFilename;

    // for the contact avatar, save it with the contactId + .png
    if (newEntry.category == AvatarManager::Contact && newEntry.contact) {
        dataFilename = KIO::encodeFileName(newEntry.contact->contactId()) + QLatin1String("_");
    }

    QCryptographicHash md5(QCryptographicHash::Md5);
    md5.addData(data);
    dataFilename += md5.result();

    QBuffer buffer(&data);
    buffer.open(QIODevice::ReadOnly);
    QImageReader ir(&buffer);
    dataFilename += QLatin1String(".") + QLatin1String(ir.format());

    // Save (original) data on disk
    dataUrl = dataUrl.adjusted(QUrl::StripTrailingSlash);
    dataUrl.setPath(dataUrl.path() + '/' + (dataFilename));
    QFile f(dataUrl.toLocalFile());
    if (!f.open(QIODevice::WriteOnly)) {
        qCDebug(LIBKOPETE_LOG) << "Saving of original avatar to " << dataUrl.toLocalFile() << " failed !";
        return AvatarEntry();
    }
    f.write(data);
    f.flush();
    f.close();

    // Save metadata of image
    KConfigGroup avatarConfig(KSharedConfig::openConfig(configUrl.toLocalFile(), KConfig::SimpleConfig), newEntry.name);

    avatarConfig.writeEntry("Filename", avatarFilename);
    avatarConfig.writeEntry("DataFilename", dataFilename);
    avatarConfig.writeEntry("Category", int(newEntry.category));

    avatarConfig.sync();

    // Add final path to the new entry for avatarAdded signal
    newEntry.path = avatarUrl.toLocalFile();
    newEntry.dataPath = dataUrl.toLocalFile();

    emit avatarAdded(newEntry);

    return newEntry;
}

bool AvatarManager::remove(Kopete::AvatarManager::AvatarEntry entryToRemove)
{
    // We need name and path to remove an avatar from the storage.
    if (entryToRemove.name.isEmpty() && entryToRemove.path.isEmpty()) {
        return false;
    }

    // We don't allow removing avatars from Contact category
    if (entryToRemove.category & Kopete::AvatarManager::Contact) {
        return false;
    }

    // Delete the image file first, file delete is more likely to fail than config group remove.
    if (KIO::NetAccess::del(QUrl(entryToRemove.path), 0)) {
        qCDebug(LIBKOPETE_LOG) << "Removing avatar from config.";

        QUrl configUrl = d->baseDir;
        configUrl = configUrl.adjusted(QUrl::StripTrailingSlash);
        configUrl.setPath(configUrl.path() + '/' + (UserDir));
        configUrl = configUrl.adjusted(QUrl::StripTrailingSlash);
        configUrl.setPath(configUrl.path() + '/' + (AvatarConfig));

        KConfigGroup avatarConfig(KSharedConfig::openConfig(configUrl.toLocalFile(), KConfig::SimpleConfig), entryToRemove.name);
        avatarConfig.deleteGroup();
        avatarConfig.sync();

        emit avatarRemoved(entryToRemove);

        return true;
    }

    return false;
}

bool AvatarManager::exists(Kopete::AvatarManager::AvatarEntry entryToCheck)
{
    if (entryToCheck.name.isEmpty()) {
        return false;
    }
    return exists(entryToCheck.name);
}

bool AvatarManager::exists(const QString &avatarName)
{
    QUrl configUrl = d->baseDir;
    configUrl = configUrl.adjusted(QUrl::StripTrailingSlash);
    configUrl.setPath(configUrl.path() + '/' + (UserDir));
    configUrl = configUrl.adjusted(QUrl::StripTrailingSlash);
    configUrl.setPath(configUrl.path() + '/' + (AvatarConfig));

    KConfigGroup avatarConfig(KSharedConfig::openConfig(configUrl.toLocalFile(), KConfig::SimpleConfig), avatarName);
    qCDebug(LIBKOPETE_LOG) << "Checking if an avatar exists: " << avatarName;
    if (!avatarConfig.exists()) {
        return false;
    }
    return true;
}

void AvatarManager::Private::createDirectory(const QUrl &directory)
{
    if (!QFile::exists(directory.toLocalFile())) {
        qCDebug(LIBKOPETE_LOG) << "Creating directory: " << directory.toLocalFile();
        if (!KIO::NetAccess::mkdir(directory, 0)) {
            qCDebug(LIBKOPETE_LOG) << "Directory " << directory.toLocalFile() <<" creating failed.";
        }
    }
}

QImage AvatarManager::Private::scaleImage(const QImage &source)
{
    if (source.isNull()) {
        return QImage();
    }

    //make an empty image and fill with transparent color
    QImage result(96, 96, QImage::Format_ARGB32);
    result.fill(0);

    QPainter paint(&result);
    float x = 0, y = 0;

    // scale and center the image
    if (source.width() > 96 || source.height() > 96) {
        const QImage scaled = source.scaled(96, 96, Qt::KeepAspectRatio, Qt::SmoothTransformation);

        x = (96 - scaled.width()) / 2.0;
        y = (96 - scaled.height()) / 2.0;

        paint.translate(x, y);
        paint.drawImage(QPoint(0, 0), scaled);
    } else {
        x = (96 - source.width()) / 2.0;
        y = (96 - source.height()) / 2.0;

        paint.translate(x, y);
        paint.drawImage(QPoint(0, 0), source);
    }

    return result;
}

//END AvatarManager

//BEGIN AvatarQueryJob
class AvatarQueryJob::Private
{
public:
    Private(AvatarQueryJob *parent)
        : queryJob(parent)
        , category(AvatarManager::All)
    {
    }

    QPointer<AvatarQueryJob> queryJob;
    AvatarManager::AvatarCategory category;
    QList<AvatarManager::AvatarEntry> avatarList;
    QUrl baseDir;

    void listAvatarDirectory(const QString &path);
};

AvatarQueryJob::AvatarQueryJob(QObject *parent)
    : KJob(parent)
    , d(new Private(this))
{
}

AvatarQueryJob::~AvatarQueryJob()
{
    delete d;
}

void AvatarQueryJob::setQueryFilter(Kopete::AvatarManager::AvatarCategory category)
{
    d->category = category;
}

void AvatarQueryJob::start()
{
    d->baseDir = QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::DataLocation) + QLatin1String("/avatars"));

    if (d->category & Kopete::AvatarManager::User) {
        d->listAvatarDirectory(UserDir);
    }
    if (d->category & Kopete::AvatarManager::Contact) {
        QUrl contactUrl(d->baseDir);
        contactUrl = contactUrl.adjusted(QUrl::StripTrailingSlash);
        contactUrl.setPath(contactUrl.path() + '/' + (ContactDir));

        const QDir contactDir(contactUrl.toLocalFile());
        const QStringList subdirsList = contactDir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
        foreach (const QString &subdir, subdirsList) {
            d->listAvatarDirectory(ContactDir + QDir::separator() + subdir);
        }
    }

    // Finish the job
    emitResult();
}

QList<Kopete::AvatarManager::AvatarEntry> AvatarQueryJob::avatarList() const
{
    return d->avatarList;
}

void AvatarQueryJob::Private::listAvatarDirectory(const QString &relativeDirectory)
{
    QUrl avatarDirectory = baseDir;
    avatarDirectory = avatarDirectory.adjusted(QUrl::StripTrailingSlash);
    avatarDirectory.setPath(avatarDirectory.path() + '/' + (relativeDirectory));

    qCDebug(LIBKOPETE_LOG) << "Listing avatars in " << avatarDirectory.toLocalFile();

    // Look for Avatar configuration
    QUrl avatarConfigUrl = avatarDirectory;
    avatarConfigUrl = avatarConfigUrl.adjusted(QUrl::StripTrailingSlash);
    avatarConfigUrl.setPath(avatarConfigUrl.path() + '/' + (AvatarConfig));
    if (QFile::exists(avatarConfigUrl.toLocalFile())) {
        KConfig *avatarConfig = new KConfig(avatarConfigUrl.toLocalFile(), KConfig::SimpleConfig);
        // Each avatar entry in configuration is a group
        const QStringList groupEntryList = avatarConfig->groupList();
        foreach (const QString &groupEntry, groupEntryList) {
            KConfigGroup cg(avatarConfig, groupEntry);

            Kopete::AvatarManager::AvatarEntry listedEntry;
            listedEntry.name = groupEntry;
            listedEntry.category = static_cast<Kopete::AvatarManager::AvatarCategory>(cg.readEntry("Category", 0));

            const QString filename = cg.readEntry("Filename", QString());
            QUrl avatarPath(avatarDirectory);
            avatarPath = avatarPath.adjusted(QUrl::StripTrailingSlash);
            avatarPath.setPath(avatarPath.path() + '/' + (filename));
            listedEntry.path = avatarPath.toLocalFile();

            const QString dataFilename = cg.readEntry("DataFilename", QString());
            QUrl dataPath(avatarDirectory);
            dataPath = dataPath.adjusted(QUrl::StripTrailingSlash);
            dataPath.setPath(dataPath.path() + '/' + (dataFilename));
            listedEntry.dataPath = dataPath.toLocalFile();

            avatarList << listedEntry;
        }
        delete avatarConfig;
    }
}

//END AvatarQueryJob
}
