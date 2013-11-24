/*
    wlmchatsessioninkaction.cpp - Kopete Wlm Protocol

    Copyright (c) 2009      by Tiago Salem Herrmann <tiagosh@gmail.com>
    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU General Public                   *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "wlmchatsessioninkaction.h"

#include <QHBoxLayout>

#include <KMenu>
#include <KIcon>
#include <KLocale>

#include "wlmchatsessioninkarea.h"
#include "ui_wlmchatsessioninkpopup.h"

class WlmChatSessionInkAction::WlmChatSessionInkActionPrivate
{
public:
    WlmChatSessionInkActionPrivate()
    {
        m_popup = new KMenu(0L);
        m_sessionInk = new QWidget;
        Ui::InkWindow ui;
        ui.setupUi(m_sessionInk);
        m_sessionInk->setObjectName( QLatin1String("WlmChatSessionInkActionPrivate::m_sessionInk") );
        QWidgetAction *act = new QWidgetAction(m_popup);
        act->setDefaultWidget(m_sessionInk);
        m_popup->addAction(act);
    }

    ~WlmChatSessionInkActionPrivate()
    {
        delete m_popup;
        m_popup = 0;
        delete m_sessionInk;
        m_sessionInk = 0;
    }

    KMenu *m_popup;
    QWidget *m_sessionInk;
};


WlmChatSessionInkAction::WlmChatSessionInkAction( QObject* parent )
  : KActionMenu( i18n( "Send Ink" ), parent )
{
    d = new WlmChatSessionInkActionPrivate;
    setMenu( d->m_popup );
    
    setIcon( KIcon("application-pgp-signature") );
    QList<WlmChatSessionInkArea *> sessionInkList = d->m_sessionInk->findChildren<WlmChatSessionInkArea *>();
    WlmChatSessionInkArea *inkArea = sessionInkList.first();
    if(inkArea)
    {
        connect( inkArea, SIGNAL(sendInk(QPixmap)),
                        this, SIGNAL(sendInk(QPixmap)) );
        connect( inkArea, SIGNAL(raiseInkWindow()),
                        this, SLOT(raiseInkWindow()) );
    }
}

void WlmChatSessionInkAction::raiseInkWindow()
{
    menu()->exec();
}

WlmChatSessionInkAction::~WlmChatSessionInkAction()
{
    delete d;
    d = NULL;
}

#include "wlmchatsessioninkaction.moc"
