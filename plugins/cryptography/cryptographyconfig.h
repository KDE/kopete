/*
    cryptographyconfig.cpp  -  description

    This is based off of texteffectconfig.h

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

#ifndef CRYPTOGRAPHYCONFIG_H
#define CRYPTOGRAPHYCONFIG_H

class QString;

class CryptographyConfig
{
public:
	
	enum CacheMode { Never, Time, Close };
	
	CryptographyConfig();

	void load();
	void save();

	//accessor functions
	QString fingerprint() const { return mFingerprint; }
	CacheMode cacheMode() const { return mCacheMode; }
	uint cacheTime() const { return mCacheTime; }
	bool askPassphraseOnStartup() const { return mAskPassphraseOnStartup; }
	
	void setFingerprint(QString f) { mFingerprint = f; }
	void setCacheMode(CacheMode mode) { mCacheMode = mode; }
	void setCacheTime(uint time) { mCacheTime = time; }
	void setAskPassphraseOnStartup (bool b) { mAskPassphraseOnStartup = b; }

private:
	QString mFingerprint;
	CacheMode mCacheMode;
	uint mCacheTime;
	bool mAskPassphraseOnStartup;

};

#endif
