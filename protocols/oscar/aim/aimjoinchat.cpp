// aimjoinchat.cpp

// Copyright (C)  2005  Matt Rogers <mattr@kde.org>

// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA
// 02110-1301  USA

#include "aimjoinchat.h"

#include <klocale.h>

#include "aimjoinchatbase.h"
#include "aimaccount.h"




AIMJoinChatUI::AIMJoinChatUI( AIMAccount* account,  bool modal,
                              QWidget* parent, const char* name )
    : KDialogBase( parent, name, modal, i18n( "Join an AIM chat room" ),
                   Cancel | User1, User1, true, i18n( "Join" ) )
{
    kdDebug(14200) << k_funcinfo << "Account " << account->accountId()
                   << " joining a chat room" << endl;

    m_account = account;

    m_joinUI = new AIMJoinChatBase( this, "aimjoinchatbase" );

    setMainWidget( m_joinUI  );

    QObject::connect( this, SIGNAL( user1Clicked() ), this, SLOT( joinChat() ) );
    QObject::connect( this, SIGNAL( cancelClicked() ), this, SLOT( closeClicked() ) );


    //add exchanges to the spin box
}

AIMJoinChatUI::~AIMJoinChatUI()
{

}

void AIMJoinChatUI::joinChat()
{
    //join a chat room
}

void AIMJoinChatUI::closeClicked()
{
    //hmm, do nothing?
    emit closing();
}

#include "aimjoinchat.moc"
