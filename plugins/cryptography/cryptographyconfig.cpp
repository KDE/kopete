/*
    cryptographyconfig.cpp  -  description

    This is based off of texteffectconfig.cpp

    Copyright (c) 2007      by Charles Connell <charles@connells.org>

    Kopete    (c) 2007 by the Kopete developers <kopete-devel@kde.org>

    ***************************************************************************
    *                                                                         *
    *   This program is free software; you can redistribute it and/or modify  *
    *   it under the terms of the GNU General Public License as published by  *
    *   the Free Software Foundation; either version 2 of the License, or     *
    *   (at your option) any later version.                                   *
    *                                                                         *
    ***************************************************************************
*/

#include <qstring.h>

#include <kglobal.h>
#include <ksharedconfig.h>

#include "cryptographyconfig.h"

CryptographyConfig::CryptographyConfig()
{
	load();
}

void CryptographyConfig::load()
{
	KConfigGroup config(KGlobal::config(), "Cryptography Plugin");

	mFingerprint = config.readEntry ("Private key fingerprint", "");
	mAskPassPhrase = config.readEntry ("Ask for passphrase", false);
	mCacheMode = (CryptographyConfig::CacheMode)config.readEntry ("Cache mode", (uint)CryptographyConfig::Close);
	mCacheTime = config.readEntry ("Cache time", 15);
}

void CryptographyConfig::save()
{
	KConfigGroup config(KGlobal::config(), "Cryptography Plugin");

	config.writeEntry("Private key fingerprint", mFingerprint );
	config.writeEntry("Ask for passphrase", mAskPassPhrase);
	config.writeEntry("Cache mode", (uint)mCacheMode);
	config.writeEntry("Cache time", mCacheTime);
	
	config.sync();
}

QString CryptographyConfig::fingerprint() const
{
	return mFingerprint;
}

bool CryptographyConfig::askPassPhrase() const
{
	return mAskPassPhrase;
}

CryptographyConfig::CacheMode CryptographyConfig::cacheMode() const
{
	return mCacheMode;
}

uint CryptographyConfig::cacheTime() const
{
	return mCacheTime;
}

void CryptographyConfig::setFingerprint(QString f) 
{
	mFingerprint = f;
}

void CryptographyConfig::setAskPassPhrase(bool b)
{
	mAskPassPhrase = b;
}

void CryptographyConfig::setCacheMode(CacheMode m)
{
	mCacheMode = m;
}

void CryptographyConfig::setCacheTime(uint t)
{
	mCacheTime = t;
}
