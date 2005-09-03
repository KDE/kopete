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

#include <qspinbox.h>
#include <klocale.h>

#include "aimjoinchatbase.h"
#include "aimaccount.h"




AIMJoinChatUI::AIMJoinChatUI( AIMAccount* account,  bool modal,
                              QWidget* parent, const char* name )
    : KDialogBase( parent, name, modal, i18n( "Join AIM Chat Room" ),
                   Cancel | User1, User1, true, i18n( "Join" ) )
{

    kdDebug(OSCAR_AIM_DEBUG) << k_funcinfo << "Account " << account->accountId()
                   << " joining a chat room" << endl;

    m_account = account;

    m_joinUI = new AIMJoinChatBase( this, "aimjoinchatbase" );

    setMainWidget( m_joinUI  );

    QObject::connect( this, SIGNAL( user1Clicked() ), this, SLOT( joinChat() ) );
    QObject::connect( this, SIGNAL( cancelClicked() ), this, SLOT( closeClicked() ) );
    QObject::connect( m_joinUI->exchange, SIGNAL( valueChanged( int ) ),
                      this, SLOT( checkExchangeValue( int ) ) );
}

AIMJoinChatUI::~AIMJoinChatUI()
{

}

void AIMJoinChatUI::setExchangeList( const QValueList<int>& list )
{
    m_exchanges = list;
    m_joinUI->setExchangeList( list );

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

void AIMJoinChatUI::checkExchangeValue( int newValue )
{
    if ( m_exchanges->findIndex( newValue ) == -1 )
    {


#include "aimjoinchat.moc"
