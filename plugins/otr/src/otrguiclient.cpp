/***************************************************************************
 *   Copyright (C) 2007 by Michael Zanetti  
 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/


#include <kaction.h>
#include <klocale.h>
//#include <kactionclasses.h>
#include <kactionmenu.h>
#include <kopetechatsession.h>
#include <ui/kopeteview.h>
#include <kopetemessage.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>


#include "otrguiclient.h"
#include "otrplugin.h"

/**
  * @author Frank Scheffold
  * @author Michael Zanetti
  */


OtrGUIClient::OtrGUIClient( Kopete::ChatSession *parent )
: QObject( parent ), KXMLGUIClient( parent )
{
//	setInstance( OTRPlugin::plugin()->instance() );

	connect( OTRPlugin::plugin(),
		SIGNAL( destroyed( QObject * ) ), this,
		SLOT( deleteLater() )

	);

	connect(this, SIGNAL( signalOtrChatsession(Kopete::ChatSession*, bool) ), OTRPlugin::plugin(), SLOT(slotEnableOtr(Kopete::ChatSession*, bool)));

	connect( OTRPlugin::plugin(), SIGNAL( goneSecure( Kopete::ChatSession *, int ) ),
	this, SLOT( encryptionEnabled( Kopete::ChatSession *, int ) ) );

	connect( this, SIGNAL( signalVerifyFingerprint( Kopete::ChatSession * ) ), OTRPlugin::plugin(), SLOT(slotVerifyFingerprint( Kopete::ChatSession * )) );

	m_manager = parent;
//	otrActionMenu = new KActionMenu(i18n("OTR Settings"),"otr_disabled", actionCollection(), "otr_settings");
//	otrActionMenu->setDelayed( false );
//	actionEnableOtr = new KAction(i18n( "Start OTR session" ), "otr_private", 0,this,SLOT(slotEnableOtr()),actionCollection(), "enable_otr");
//	actionDisableOtr = new KAction(i18n("End OTR session"), "otr_disabled",0, this,SLOT(slotDisableOtr()), actionCollection(), "disable_otr");
//	actionVerifyFingerprint = new KAction(i18n("Verify fingerprint"), "signature",0, this,SLOT(slotVerifyFingerprint()), actionCollection(), "verify_fingerprint");

//	otrActionMenu->insert(actionEnableOtr);
//	otrActionMenu->insert(actionDisableOtr);
//	otrActionMenu->insert(actionVerifyFingerprint);

	setXMLFile("otrchatui.rc");

	encryptionEnabled( parent, OtrlChatInterface::self()->privState(parent) );
    

}

OtrGUIClient::~OtrGUIClient()
{
}

void OtrGUIClient::slotEnableOtr()
{
	emit signalOtrChatsession( m_manager, true );
}
void OtrGUIClient::slotDisableOtr()
{
 	emit signalOtrChatsession( m_manager, false );
}

void OtrGUIClient::slotVerifyFingerprint(){
	emit signalVerifyFingerprint( m_manager );
}

void OtrGUIClient::encryptionEnabled(Kopete::ChatSession *session, int state){
/*	if( session == m_manager ){
		switch(state){
			case 0:
				otrActionMenu->setIcon("otr_disabled");
				actionEnableOtr->setText( i18n("Start OTR session") );
				actionDisableOtr->setEnabled(false);
				actionVerifyFingerprint->setEnabled(false);
				break;
			case 1:
				otrActionMenu->setIcon("otr_unverified");
				actionEnableOtr->setText( i18n("Refresh OTR session") );
				actionDisableOtr->setEnabled(true);
				actionVerifyFingerprint->setEnabled(true);
				break;
			case 2:
				otrActionMenu->setIcon("otr_private");
				actionEnableOtr->setText( i18n("Refresh OTR session") );
				actionDisableOtr->setEnabled(true);
				actionVerifyFingerprint->setEnabled(true);
				break;
			case 3:
				otrActionMenu->setIcon("otr_finished");
				actionEnableOtr->setText( i18n("Start OTR session") );
				actionDisableOtr->setEnabled(true);
				actionVerifyFingerprint->setEnabled(false);
				break;
		}
	}*/
}

#include "otrguiclient.moc"

// vim: set noet ts=4 sts=4 sw=4:
