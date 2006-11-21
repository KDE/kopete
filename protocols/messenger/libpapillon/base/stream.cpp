/*
   stream.cpp - Papillon Abstract class for a Stream(Notification/Switchboard)

   Copyright (c) 2006 by Michaël Larouche <larouche@kde.org>

   Based on code Copyright (c) 2004 Matt Rogers <mattr@kde.org>
   Based on code copyright (c) 2004 SuSE Linux AG <http://www.suse.com>
   Based on Iris, Copyright (C) 2003 Justin Karneges

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
*/

#include "Papillon/Base/Stream"

namespace Papillon
{

Stream::Stream(QObject *parent)
:QObject(parent)
{
}

Stream::~Stream()
{
}

}

#include "stream.moc"
