/*
    cryptographyconfig.h

    Copyright (c) 2003      by Olivier Goffart       <ogoffart@tiscalinet.be>
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

#ifndef CRYPTOGRAPHYCONFIG_H
#define CRYPTOGRAPHYCONFIG_H

#include <qstring.h>

class CryptographyConfig
{
public:
	enum CacheMode
	{
		Keep	= 0,
		Time	= 1,
		Never	= 2
	};

	CryptographyConfig();

	void save();
	void load();

	QString privateKey() const;
	int cacheMode() const;
	bool encrypt() const;
	bool askPassPhrase() const;
	bool cachePassPhrase() const;
	unsigned int cacheTime() const;

	void setPrivateKey(const QString &newPrivateKey);
	void setCacheTime(const unsigned int newCacheTime);
	void setCacheMode(int newCacheMode);
	void setCachePassPhrase(const bool cachePassPhrase);
	void setAskPassPhrase(const bool newAskPassPhrase);
	void setEncrypt(const bool newEncrypt);

private:
	QString mPrivateKeyID;
	int mCacheMode;
	unsigned int mCacheTime;
	bool mEncrypt;
	bool mAskPassPhrase;
	bool mCachePassPhrase;

};

#endif
