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

#include "jabberbobcache.h"

JabberBoBCache::JabberBoBCache(QObject *parent) : BoBCache(parent)
{
}

void JabberBoBCache::put(const XMPP::BoBData &data)
{
	mem.insert(data.cid(), data);
}

XMPP::BoBData JabberBoBCache::get(const QString &cid)
{
	return mem.value(cid);
}

#include "jabberbobcache.moc"
