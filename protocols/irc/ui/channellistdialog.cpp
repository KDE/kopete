/*
    channellistdialog.cpp - IRC Channel Search Dialog

    Copyright (c) 2004      by Jason Keirstead <jason@keirstead.org>
    Copyright (c) 2005-2007 by Michel Hermier <michel.hermier@gmail.com>

    Kopete    (c) 2002-2007 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "channellistdialog.h"

#include "kircengine.h"

#include "kopeteuiglobal.h"

#include "qlayout.h"

#include <Q3HBoxLayout>

ChannelListDialog::ChannelListDialog(KIRC::Engine *engine, const QString &caption, QObject *target, const char *slotJoinChan)
    : KDialog(Kopete::UI::Global::mainWidget())
{
    m_engine = engine;
    setCaption(caption);
    setButtons(Close);
    setDefaultButton(Close);
    setModal(false);
    m_list = new ChannelList(this, engine);

    connect(m_list, SIGNAL(channelDoubleClicked(QString)),
            target, slotJoinChan);

    connect(m_list, SIGNAL(channelDoubleClicked(QString)),
            this, SLOT(slotChannelDoubleClicked(QString)));

    new Q3HBoxLayout(m_list, 0, spacingHint());

    setInitialSize(QSize(500, 400));
    setMainWidget(m_list);
    show();
}

void ChannelListDialog::clear()
{
    m_list->clear();
}

void ChannelListDialog::search()
{
    m_list->search();
}

void ChannelListDialog::slotChannelDoubleClicked(const QString &)
{
    close();
}
