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
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
// 02110-1301  USA

#include "aimjoinchat.h"

#include <qlineedit.h>
#include <klocale.h>

#include "ui_aimjoinchatbase.h"
#include "aimaccount.h"

AIMJoinChatUI::AIMJoinChatUI( AIMAccount* account, QWidget* parent )
    : KDialog( parent )
{

    setCaption(  i18n( "Join AIM Chat Room" ) );
    setButtons( KDialog::Cancel | KDialog::User1 );
    setDefaultButton( KDialog::User1 );
    setButtonGuiItem( KDialog::User1, KGuiItem( i18n("Join") ) );
    showButtonSeparator( true );

    kDebug(OSCAR_AIM_DEBUG) << "Account " << account->accountId()
                   << " joining a chat room" << endl;

    m_account = account;

    QWidget* w = new QWidget( this );
    m_joinUI = new Ui::AIMJoinChatBase();
    m_joinUI->setupUi( w );

    setMainWidget( w  );

    QObject::connect( this, SIGNAL(user1Clicked()), this, SLOT(joinChat()) );
    QObject::connect( this, SIGNAL(cancelClicked()), this, SLOT(closeClicked()) );
}

AIMJoinChatUI::~AIMJoinChatUI()
{
    m_exchanges.clear();
    delete m_joinUI;
}

void AIMJoinChatUI::setExchangeList( const QList<int>& list )
{
    m_exchanges = list;
    QStringList exchangeList;
    QList<int>::const_iterator it = list.begin();
    while ( it != list.end() )
    {
        exchangeList.append( QString::number( ( *it ) ) );
        ++it;
    }


    m_joinUI->exchange->insertItems( 0, exchangeList );
}

void AIMJoinChatUI::joinChat()
{
    m_roomName = m_joinUI->roomName->text();
    int item = m_joinUI->exchange->currentIndex();
    m_exchange = m_joinUI->exchange->itemText( item );

    emit closing( QDialog::Accepted );
}

void AIMJoinChatUI::closeClicked()
{
    //hmm, do nothing?
    emit closing( QDialog::Rejected );
}

QString AIMJoinChatUI::roomName() const
{
    return m_roomName;
}

QString AIMJoinChatUI::exchange() const
{
    return m_exchange;
}

#include "aimjoinchat.moc"
//kate: space-indent on; indent-width 4;
