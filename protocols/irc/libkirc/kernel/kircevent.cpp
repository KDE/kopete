/*
    kircmessage.cpp - IRC Client

    Copyright (c) 2003-2007 by Michel Hermier <michel.hermier@gmail.com>

    Kopete    (c) 2003-2007 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "kircevent.h"

using namespace KIrc;

const QEvent::Type KIrc::CommandEvent::Type = (QEvent::Type)QEvent::registerEventType();

const QEvent::Type KIrc::MessageEvent::Type = (QEvent::Type)QEvent::registerEventType();

const QEvent::Type KIrc::TextEvent::Type = (QEvent::Type)QEvent::registerEventType();

const QEvent::Type KIrc::ControlEvent::Type = (QEvent::Type)QEvent::registerEventType();
