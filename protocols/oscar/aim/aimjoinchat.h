// aimjoinchat.h

// Copyright (C)  2005	Matt Rogers <mattr@kde.org>

// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
// 02110-1301  USA

#ifndef AIMJOINCHAT_H
#define AIMJOINCHAT_H

#include <kdialog.h>

#include "oscartypes.h"

class AIMAccount;
namespace Ui { class AIMJoinChatBase; }

class AIMJoinChatUI : public KDialog
{
Q_OBJECT
public:
	explicit AIMJoinChatUI( AIMAccount*, QWidget* parent = 0 );
	~AIMJoinChatUI();

    void setExchangeList( const QList<int>& );
    QList<int> exchangeList() const;

    QString roomName() const;
    QString exchange() const;


protected slots:
	void joinChat();
	void closeClicked();

signals:
	void closing( int );

private:
	Ui::AIMJoinChatBase* m_joinUI;
	AIMAccount* m_account;
    QList<int> m_exchanges;
    QString m_roomName;
    QString m_exchange;

};

#endif
//kate: space-indent on; indent-width 4;
