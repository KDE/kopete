 /*

    Copyright (c) 2013 by Pali Rohár <pali.rohar@gmail.com>


    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
 */

#ifndef JABBERBOBCACHE_H
#define JABBERBOBCACHE_H

#include <xmpp_status.h>

class JabberBoBCache : public XMPP::BoBCache
{
	Q_OBJECT

public:
	JabberBoBCache(QObject *parent = NULL);

	virtual void put(const XMPP::BoBData &data);
	virtual XMPP::BoBData get(const QString &cid);

private:
	QHash <QString, XMPP::BoBData> mem;
};

#endif
