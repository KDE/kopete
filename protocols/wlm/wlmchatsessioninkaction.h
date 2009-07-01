/*
    wlmchatsessioninkaction.h - Kopete Wlm Protocol

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
#ifndef WLMCHATSESSIONINKACTION_H
#define WLMCHATSESSIONINKACTION_H

#include <QWidget>
#include <QPixmap>
#include <KActionMenu>

class WlmChatSessionInkAction : public KActionMenu
{
    Q_OBJECT
    public:
        WlmChatSessionInkAction(QObject * parent = 0);
        virtual ~WlmChatSessionInkAction();
    signals:
        void sendInk( const QPixmap & ink );
    private:
        class WlmChatSessionInkActionPrivate;
        WlmChatSessionInkActionPrivate *d;
    public slots:
        void raiseInkWindow();
};
#endif
