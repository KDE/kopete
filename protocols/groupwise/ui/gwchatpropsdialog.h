/*
    Kopete Groupwise Protocol
    gwchatpropsdialog.h - dialog for viewing/modifying chat properties

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

#ifndef GWCHATPROPSDIALOG_H
#define GWCHATPROPSDIALOG_H

#include <kdialog.h>

#include "gwchatrooms.h"
#include "ui_gwchatprops.h"

/**
 * Dialog for viewing/modifying chat properties.
 * Chatroom list dialog gets props from manager
 * Chatroom list dialog opens chatpropsdlg using props, connects to OkClicked signal
 * User makes changes
 * CLD asks CPD for changes.
 * CLD passes changes to manager
 * manager sends ChatUpdate to server
 * on success, manager updates own model.

 1) Create dialog with populated widget from supplied Chatroom.
 2) Add readonly mode.
 3) Track which things changed?  Easier to get the changed Chatroom back and diff in the manager, simpler api connecting 
 */
class GroupWiseChatPropsDialog : public KDialog
{
	Q_OBJECT
	public:
		/**
		 * Construct an empty dialog
		 */
		GroupWiseChatPropsDialog( QWidget * parent );
		/**
		 * Construct a populated dialog
		 */
		GroupWiseChatPropsDialog( const GroupWise::Chatroom & room, bool readOnly,
									   QWidget * parent );
		
		~GroupWiseChatPropsDialog();
		
		bool dirty() { return m_dirty; };
		GroupWise::Chatroom room();
	protected:
		void initialise();
	protected slots:
		void slotWidgetChanged();
	private:
		Ui::GroupWiseChatProps m_ui;
		GroupWise::Chatroom m_room;
		bool m_dirty;
};

#endif
