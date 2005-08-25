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
// Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA
// 02110-1301  USA

#ifndef AIMJOINCHAT_H
#define AIMJOINCHAT_H

#include <kdialogbase.h>

class AIMAccount;
class AIMJoinChatBase;

class AIMJoinChatUI : public KDialogBase
{
Q_OBJECT
public:
	AIMJoinChatUI( AIMAccount*,  bool modal, QWidget* parent = 0,
	               const char* name = 0 );
	~AIMJoinChatUI();

protected slots:
	void joinChat();
	void closeClicked();

signals:
	void closing();

private:
	AIMJoinChatBase* m_joinUI;
	AIMAccount* m_account;

};

#endif
