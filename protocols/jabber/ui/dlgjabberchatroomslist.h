//
// C++ Interface: 
//
// Description: 
//
//
// Author: Kopete Developers <kopete-devel@kde.org>, (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef DLGJABBERCHATROOMSLIST_H
#define DLGJABBERCHATROOMSLIST_H

#include "jabberaccount.h"
#include "xmpp_tasks.h"

#include "dlgchatroomslist.h"

class dlgJabberChatRoomsList : public dlgChatRoomsList
{
  Q_OBJECT

public:
	dlgJabberChatRoomsList(JabberAccount* account, const QString& server = QString::null, const QString& nick = QString::null, QWidget* parent = 0, const char* name = 0);
  ~dlgJabberChatRoomsList();
  /*$PUBLIC_FUNCTIONS$*/

public slots:
  /*$PUBLIC_SLOTS$*/
  virtual void slotJoin();
  virtual void slotQuery();
  virtual void slotDoubleClick(int row, int col, int button, const QPoint& mousePos);
  virtual void slotClick(int row, int col, int button, const QPoint& mousePos);

protected:
  /*$PROTECTED_FUNCTIONS$*/

protected slots:
  /*$PROTECTED_SLOTS$*/

	void slotQueryFinished();

private:

	JabberAccount *m_account;
	int m_selectedRow;
	QString m_chatServer;
	QString m_nick;
};

#endif

