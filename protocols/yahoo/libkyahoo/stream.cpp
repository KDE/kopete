/*
    stream.cpp - Kopete Groupwise Protocol

    Copyright (c) 2004      SUSE Linux AG	 	 http://www.suse.com

    Based on Iris, Copyright (C) 2003  Justin Karneges <justin@affinix.com>

    Kopete (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>
 
    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "stream.h"

Stream::Stream(QObject *parent)
:QObject(parent)
{
}

Stream::~Stream()
{
}

#include "stream.moc"
