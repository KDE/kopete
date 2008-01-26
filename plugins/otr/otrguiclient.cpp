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
#include <kicon.h>
#include <kactioncollection.h>

#include "otrguiclient.h"
#include "otrplugin.h"
#include "otrlchatinterface.h"

/**
  * @author Frank Scheffold
  * @author Michael Zanetti
  */


OtrGUIClient::OtrGUIClient( Kopete::ChatSession *parent )
: QObject( parent ), KXMLGUIClient( parent )
{
	setComponentData( OTRPlugin::plugin()->componentData() );

	connect( OTRPlugin::plugin(),
		SIGNAL( destroyed( QObject * ) ), this,
		SLOT( deleteLater() )

	);

	connect(this, SIGNAL( signalOtrChatsession(Kopete::ChatSession*, bool) ), OTRPlugin::plugin(), SLOT(slotEnableOtr(Kopete::ChatSession*, bool)));

	connect( OtrlChatInterface::self(), SIGNAL( goneSecure( Kopete::ChatSession *, int ) ),
	this, SLOT( encryptionEnabled( Kopete::ChatSession *, int ) ) );

	connect( this, SIGNAL( signalVerifyFingerprint( Kopete::ChatSession * ) ), OTRPlugin::plugin(), SLOT(slotVerifyFingerprint( Kopete::ChatSession * )) );

	m_manager = parent;

	otrActionMenu = new KActionMenu(KIcon("document-decrypt"), i18n("OTR Encryption"), actionCollection() );
	otrActionMenu->setDelayed( false );
	actionCollection()->addAction("otr_settings", otrActionMenu);


	actionEnableOtr = new KAction( KIcon("document-encrypt"), i18n( "Start OTR session" ), this);
	actionCollection()->addAction( "enableOtr", actionEnableOtr );
	connect(actionEnableOtr, SIGNAL(triggered(bool)), this, SLOT(slotEnableOtr()));

	actionDisableOtr = new KAction( KIcon("document-decrypt"), i18n( "End OTR session" ), this);
	actionCollection()->addAction( "disableOtr", actionDisableOtr );
	connect(actionDisableOtr, SIGNAL(triggered(bool)), this, SLOT(slotDisableOtr()));

	actionVerifyFingerprint = new KAction( KIcon( "document-sign" ),  i18n("Authenticate Contact"), this);
	actionCollection()->addAction( "verifyFingerprint", actionVerifyFingerprint );
	connect(actionVerifyFingerprint, SIGNAL(triggered(bool)), this,SLOT(slotVerifyFingerprint()));

	 // jpetso says: please request an icon named "document-verify" or something like that, the "sign" icon is not really appropriate for this purpose imho
	// mzanetti says: the "document-sign" icon is the same as kgpg uses to sign fingerprints. Anyways I will discuss that on #kopete. Re-using "document-sign for now.

	otrActionMenu->addAction(actionEnableOtr);
	otrActionMenu->addAction(actionDisableOtr);
	otrActionMenu->addAction(actionVerifyFingerprint);

	setXMLFile("otrchatui.rc");
//	setupGUI();

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
kdDebug() << "OTRGUIClient switched security state to: " << state << endl;
	if( session == m_manager ){
		switch(state){
			case 0:
				otrActionMenu->setIcon(KIcon("document-decrypt"));
				actionEnableOtr->setText( i18n("Start OTR session") );
				actionDisableOtr->setEnabled(false);
				actionVerifyFingerprint->setEnabled(false);
				break;
			case 1:
				otrActionMenu->setIcon(KIcon("document-encrypt-unverified"));
				actionEnableOtr->setText( i18n("Refresh OTR session") );
				actionDisableOtr->setEnabled(true);
				actionVerifyFingerprint->setEnabled(true);
				break;
			case 2:
				otrActionMenu->setIcon(KIcon("document-encrypt-verified"));
				actionEnableOtr->setText( i18n("Refresh OTR session") );
				actionDisableOtr->setEnabled(true);
				actionVerifyFingerprint->setEnabled(true);
				break;
			case 3:
				otrActionMenu->setIcon(KIcon("document-encrypt-finished"));
				actionEnableOtr->setText( i18n("Start OTR session") );
				actionDisableOtr->setEnabled(true);
				actionVerifyFingerprint->setEnabled(false);
				break;
		}
	}
}

#include "otrguiclient.moc"

// vim: set noet ts=4 sts=4 sw=4:
