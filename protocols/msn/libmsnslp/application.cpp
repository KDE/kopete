/*
    application.cpp - Peer to Peer Application

    Copyright (c) 2006 by Gregg Edghill     <gregg.edghill@gmail.com>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; version 2 of the License.               *
    *                                                                       *
    *************************************************************************
*/

#include "application.h"
#include <kdebug.h>

namespace PeerToPeer
{

Application::Application(QObject *parent)
{
}

Application::~Application()
{
}

void Application::handleRequest(const Q_UINT32 sessionId, const Q_UINT32 appId, const QString& context)
{
	kdDebug() << k_funcinfo << "called" << endl;
}

}

#include "application.moc"
