/*
    Kopete Groupwise Protocol
    gwchatsearchdialog.h - dialog for searching for chatrooms

    Copyright (c) 2006      Novell, Inc	 	 	 http://www.opensuse.org
    Copyright (c) 2005      SUSE Linux AG	 	 http://www.suse.com
    
    Kopete (c) 2002-2005 by the Kopete developers <kopete-devel@kde.org>
 
    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU General Public                   *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef GWCHATSEARCHDIALOG_H
#define GWCHATSEARCHDIALOG_H

class GroupWiseAccount;

#include "gwchatrooms.h"
#include "ui_gwchatsearch.h"
#include "client.h"

#include <kdialog.h>

class GroupWiseChatSearchDialog : public KDialog
{
	Q_OBJECT
	public:
		GroupWiseChatSearchDialog( GroupWiseAccount * account, QWidget * parent, const char * name );
		~GroupWiseChatSearchDialog();
	protected:
		void populateWidget();
	protected slots:
		/* Button handlers */
		void slotPropertiesClicked();
		void slotUpdateClicked();
		/* Manager update handler */
		void slotManagerUpdated();
		void slotGotProperties( const GroupWise::Chatroom & room );
	private:
		GroupWiseAccount * m_account;
		ChatroomManager * m_manager;
		Ui::GroupWiseChatSearch m_ui;
};
#endif
