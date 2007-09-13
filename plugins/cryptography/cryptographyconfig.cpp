/*
    cryptographyconfig.cpp  -  description

    This is based off of texteffectconfig.cpp

    Copyright (c) 2007      by Charles Connell <charles@connells.org>

    Kopete    (c) 2002-2007 by the Kopete developers <kopete-devel@kde.org>

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

CryptographyConfig* CryptographyConfig::mSelf = 0L;

CryptographyConfig * CryptographyConfig::self()
{
	if (!mSelf)
		mSelf = new CryptographyConfig;
	return mSelf;
}

CryptographyConfig::CryptographyConfig()
{
	load();
}

void CryptographyConfig::load()
{
	KConfigGroup config(KGlobal::config(), "Cryptography Plugin");

	mFingerprint = config.readEntry ("Private key fingerprint", "");
}

void CryptographyConfig::save()
{
	KConfigGroup config(KGlobal::config(), "Cryptography Plugin");

	config.writeEntry("Private key fingerprint", mFingerprint );
	
	config.sync();
}

void CryptographyConfig::defaults()
{
	mFingerprint = "";
}
