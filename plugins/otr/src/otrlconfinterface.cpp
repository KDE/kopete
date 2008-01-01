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


/**
  * @author Michael Zanetti
  */

#include <qapplication.h>
#include <qeventloop.h>
#include <qwidget.h>
//Added by qt3to4:
#include <Q3ValueList>

#include <kopetechatsession.h>
#include <kopeteaccount.h>
#include "kopeteuiglobal.h"

#include <kdebug.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <klocale.h>
//#include <kanimwidget.h>


#include "otrlconfinterface.h"
#include "otrlchatinterface.h"
#include "otrplugin.h"
#include "ui_privkeypopupui.h"


/*********************** Konstruktor/Destruktor **********************/

OtrlConfInterface::OtrlConfInterface( QWidget *preferencesDialog ){

	this->preferencesDialog = preferencesDialog;

	OTRL_INIT;
	
	userstate = OtrlChatInterface::self()->getUserstate();

//	kdDebug() << "OtrlConfInterface created" << endl;
}

OtrlConfInterface::~ OtrlConfInterface(){
	otrl_userstate_free(userstate);
}

/*********************** Functions for kcm module ************************/

QString OtrlConfInterface::getPrivFingerprint( QString accountId, QString protocol){
//	if (otrl_privkey_read(userstate, QString(KGlobal::dirs()->saveLocation("data", "kopete_otr/", true )) + "privkey" ) == 0){
		char fingerprint[45];
		if( otrl_privkey_fingerprint( userstate, fingerprint, accountId.toLatin1(), protocol.toLatin1()) != 0 ){
			return fingerprint;
		}
//	}
	return i18n("No fingerprint present.");
}


bool OtrlConfInterface::hasPrivFingerprint( QString accountId, QString protocol ){
	char fingerprint[45];
	if( otrl_privkey_fingerprint( userstate, fingerprint, accountId.toLatin1(), protocol.toLatin1() ) != 0 ){
		return true;
	}
	return false;
}


void OtrlConfInterface::generateNewPrivKey( QString accountId, QString protocol ){
#warning Avoid closing the window while generating private key!
	QWidget *popupwidget = new QWidget(preferencesDialog, Qt::Dialog);
	Ui::PrivKeyPopupUI *popup = new Ui::PrivKeyPopupUI( );
	popup->setupUi( popupwidget );
//	popupwidget->setCloseLock( true );
	popupwidget->show();
	popupwidget->activateWindow();
	popupwidget->raise();

	KeyGenThread *keyGenThread = new KeyGenThread ( accountId, protocol );
	keyGenThread->start();
	while( !keyGenThread->wait(100) ){
		qApp->processEvents(QEventLoop::ExcludeUserInputEvents | QEventLoop::ExcludeSocketNotifiers, 100);
	}

//	popupwidget->setCloseLock( false );
	popupwidget->close();
}

QList<QStringList> OtrlConfInterface::readAllFingerprints(){
	ConnContext *context;
	Fingerprint *fingerprint;
	QStringList entry;
	char hash[45];
	QList<QStringList> list;

	for( context = userstate->context_root; context != NULL; context = context->next ){
		fingerprint = context->fingerprint_root.next;
		while( fingerprint ){
			entry << context->username;
			if( ( context->msgstate == OTRL_MSGSTATE_ENCRYPTED ) && ( context->active_fingerprint != fingerprint ) ){
				entry << i18n("Unused");
			} else {
				if (context && context->msgstate == OTRL_MSGSTATE_ENCRYPTED) {
					if (context->active_fingerprint->trust && context->active_fingerprint->trust[0] != NULL) {
						entry << i18n("Private");
					} else {
						entry << i18n("Unverified");
					}
				} else if (context && context->msgstate == OTRL_MSGSTATE_FINISHED) {
					entry << i18n("Finished");
				} else {
					entry << i18n("Not Private");
				}
			}
			if ( fingerprint->trust && fingerprint->trust[0] ){
				entry << i18n("Yes");
			} else {
				entry << i18n("No");
			}
			otrl_privkey_hash_to_human( hash, fingerprint->fingerprint );
			entry << hash;
			entry << context->protocol;
			list << entry;
			fingerprint = fingerprint->next;
		}
	}
	return list;
}

void OtrlConfInterface::verifyFingerprint( QString strFingerprint, bool trust ){
	Fingerprint *fingerprint;
	fingerprint = findFingerprint( strFingerprint );

	if( fingerprint != 0 ){
		if( trust ){
			otrl_context_set_trust( fingerprint, "verified" );
		} else {
			otrl_context_set_trust( fingerprint, NULL );
		}
		otrl_privkey_write_fingerprints( userstate, QString(QString(KGlobal::dirs()->saveLocation("data", "kopete_otr/", true )) + "fingerprints").toLocal8Bit() );
	} else {
		kdDebug() << "could not find fingerprint" << endl;
	}
}

bool OtrlConfInterface::isVerified( QString strFingerprint ){
	Fingerprint *fingerprint;	

	fingerprint = findFingerprint( strFingerprint );

	if( fingerprint->trust && fingerprint->trust[0] ){
//		kdDebug() << "found trust" << endl;
		return true;
	} else {
//		kdDebug() << "not trusted" << endl;
		return false;
	}
}


void OtrlConfInterface::forgetFingerprint( QString strFingerprint ){
	Fingerprint *fingerprint;
	
	fingerprint = findFingerprint( strFingerprint );
	otrl_context_forget_fingerprint( fingerprint, 1 );
	otrl_privkey_write_fingerprints( userstate, QString(QString(KGlobal::dirs()->saveLocation("data", "kopete_otr/", true )) + "fingerprints").toLocal8Bit() );
}

Fingerprint *OtrlConfInterface::findFingerprint( QString strFingerprint ){
//	const char *cFingerprint = ;
//	Fingerprint *fingerprintRoot = &userstate->context_root->fingerprint_root;
	ConnContext *context;
	Fingerprint *fingerprint;
	Fingerprint *foundFingerprint = NULL;
	char hash[45];

	for( context = userstate->context_root; context != NULL; context = context->next ){
		fingerprint = context->fingerprint_root.next;
		while( fingerprint ){
			otrl_privkey_hash_to_human(hash, fingerprint->fingerprint);
			if( strcmp( hash, strFingerprint.toLatin1()) == 0 ){
				foundFingerprint = fingerprint;
			}
			fingerprint = fingerprint->next;
		}
	}	
	return foundFingerprint;
}

bool OtrlConfInterface::isEncrypted( QString strFingerprint ){
	Fingerprint *fingerprint;
	Fingerprint *tmpFingerprint;
	Fingerprint *foundFingerprint = NULL;
	ConnContext *context;
	ConnContext *foundContext = NULL;

	context = userstate->context_root;

	fingerprint = findFingerprint( strFingerprint );
	for( context = userstate->context_root; context != NULL; context = context->next ){
		tmpFingerprint = context->fingerprint_root.next;
		while( tmpFingerprint ){
			if( tmpFingerprint == fingerprint ){
//				kdDebug() << "Found context" << endl;
				foundContext = context;
				foundFingerprint = tmpFingerprint;
			}
			tmpFingerprint = tmpFingerprint->next;
		}
	}

	if( foundContext && foundContext->msgstate != OTRL_MSGSTATE_ENCRYPTED ){
		return false;
	} else if( foundContext && foundFingerprint && foundContext->active_fingerprint == foundFingerprint ){
		return true;
	} else {
		return false;
	}
}
