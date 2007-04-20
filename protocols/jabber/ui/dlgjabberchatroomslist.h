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

#include <kdialog.h>
#include "jabberaccount.h"
#include "xmpp_tasks.h"

#include "ui_dlgchatroomslist.h"

class QTableWidgetItem;
class dlgJabberChatRoomsList : public KDialog
{
  Q_OBJECT

public:
	explicit dlgJabberChatRoomsList(JabberAccount* account, const QString& server = QString(), const QString& nick = QString(), QWidget* parent = 0);
	~dlgJabberChatRoomsList();
  /*$PUBLIC_FUNCTIONS$*/

public slots:
  /*$PUBLIC_SLOTS$*/
  virtual void slotJoin();
  virtual void slotQuery();
  virtual void slotDoubleClick(QTableWidgetItem *item);
  virtual void slotClick(QTableWidgetItem *item);

protected:
  /*$PROTECTED_FUNCTIONS$*/

protected slots:
  /*$PROTECTED_SLOTS$*/

	void slotQueryFinished();

private:
	JabberAccount *m_account;
	QTableWidgetItem *m_selectedItem;
	QString m_chatServer;
	QString m_nick;

	Ui::dlgChatRoomsList m_ui;
};

#endif

