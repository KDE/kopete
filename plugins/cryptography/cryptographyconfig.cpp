/*
    cryptographyconfig.cpp

    Copyright (c) 2003      by Roberto Pariset       <ogoffart@tiscalinet.be>
    Copyright (c) 2003      by Matt Rogers           <matt@matt.rogers.name>

    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include <qstring.h>
#include <kconfig.h>
#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>


#include "cryptographyconfig.h"

CryptographyConfig::CryptographyConfig()
{
	load();
}

void CryptographyConfig::load()
{
	KConfig *config = KGlobal::config();
	config->setGroup("Cryptography Plugin");

	mPrivateKeyID = config->readEntry("PGP private key");
	mCachePassPhrase = config->readNumEntry("Cache Passphrase", Keep);
	mCacheTime = config->readNumEntry("Cache Time", 15);
	mEncrypt = config->readBoolEntry("Also My Key", false);
	mAskPassPhrase = config->readBoolEntry("No Passphrase Handling", false);

}

void CryptographyConfig::save()
{
	KConfig *config = KGlobal::config();
	config->setGroup("Cryptography Plugin");
	config->writeEntry("PGP private key", mPrivateKeyID );
	config->writeEntry("Cache Passphrase",  mCachePassPhrase );
	config->writeEntry("Cache Time", mCacheTime );
	config->writeEntry("Also My Key", mEncrypt);
	config->writeEntry("No Passphrase Handling", mAskPassPhrase);

	config->sync();
}

void CryptographyConfig::setPrivateKey(const QString &newKey)
{
	mPrivateKeyID = newKey;
}

void CryptographyConfig::setCacheMode(int newCacheTime)
{
	mCacheTime = newCacheTime;
}

void CryptographyConfig::setAskPassPhrase(const bool newAskPassPhrase)
{
	mAskPassPhrase = newAskPassPhrase;
}

void CryptographyConfig::setEncrypt(const bool newEncrypt)
{
	mEncrypt = newEncrypt;
}

void CryptographyConfig::setCachePassPhrase(const bool cachePassPhrase)
{
	mCachePassPhrase = cachePassPhrase;
}

void CryptographyConfig::setCacheTime(const unsigned int newCacheTime)
{
	mCacheTime = newCacheTime;
}

QString CryptographyConfig::privateKey() const
{
	return mPrivateKeyID;
}

unsigned int CryptographyConfig::cacheTime() const
{
	return mCacheTime;
}

bool CryptographyConfig::cachePassPhrase() const
{
	return mCachePassPhrase;
}

int CryptographyConfig::cacheMode() const
{
	return mCacheMode;
}

bool CryptographyConfig::askPassPhrase() const
{
	return mAskPassPhrase;
}

bool CryptographyConfig::encrypt() const
{
	return mEncrypt;
}

