/*************************************************************************
 * Copyright <2007 - 2013>  <Michael Zanetti> <mzanetti@kde.org>         *
 *                                                                       *
 * This program is free software; you can redistribute it and/or         *
 * modify it under the terms of the GNU General Public License as        *
 * published by the Free Software Foundation; either version 2 of        *
 * the License or (at your option) version 3 or any later version        *
 * accepted by the membership of KDE e.V. (or its successor approved     *
 * by the membership of KDE e.V.), which shall act as a proxy            *
 * defined in Section 14 of version 3 of the license.                    *
 *                                                                       *
 * This program is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 * GNU General Public License for more details.                          *
 *                                                                       *
 * You should have received a copy of the GNU General Public License     *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *************************************************************************/ 


/**
  * @author Michael Zanetti
  */


#include "otrlconfinterface.h"
#include "otrlchatinterface.h"
#include "privkeypopup.h"

#include <qapplication.h>
#include <qeventloop.h>
#include <qwidget.h>

#include <kopetechatsession.h>
#include <kopeteaccount.h>
#include "kopeteuiglobal.h"

#include <kdebug.h>
#include <kstandarddirs.h>
#include <klocale.h>

/*********************** Konstruktor/Destruktor **********************/

OtrlConfInterface::OtrlConfInterface( QWidget *preferencesDialog ){

	this->preferencesDialog = preferencesDialog;

	OTRL_INIT;
	
	userstate = OtrlChatInterface::self()->getUserstate();

}

OtrlConfInterface::~ OtrlConfInterface(){
	otrl_userstate_free(userstate);
}

/*********************** Functions for kcm module ************************/

QString OtrlConfInterface::getPrivFingerprint( const QString &accountId, const QString &protocol){
	char fingerprint[45];

	if( otrl_privkey_fingerprint( userstate, fingerprint, accountId.toLatin1(), protocol.toLatin1()) != 0 ){
		return fingerprint;
	}

	return i18nc("@info", "No fingerprint present.");
}


bool OtrlConfInterface::hasPrivFingerprint( const QString &accountId, const QString &protocol ){
	char fingerprint[45];
	if( otrl_privkey_fingerprint( userstate, fingerprint, accountId.toLatin1(), protocol.toLatin1() ) != 0 ){
		return true;
	}
	return false;
}


void OtrlConfInterface::generateNewPrivKey( const QString &accountId, const QString &protocol ){
	PrivKeyPopup *popup = new PrivKeyPopup( preferencesDialog );
	popup->show();
	popup->setCloseLock( true );

	OtrlChatInterface::self()->generatePrivateKey(accountId, protocol);

	popup->setCloseLock( false );
	popup->close();
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
				entry << i18nc("@item:intable Fingerprint was never used", "Unused");
			} else {
				if (context && context->msgstate == OTRL_MSGSTATE_ENCRYPTED) {
					if (context->active_fingerprint->trust && context->active_fingerprint->trust[0]) {
						entry << i18nc("@item:intable Fingerprint is used in a private conversation", "Private");
					} else {
						entry << i18nc("@item:intable Fingerprint is used in an unverified conversation", "Unverified");
					}
				} else if (context && context->msgstate == OTRL_MSGSTATE_FINISHED) {
					entry << i18nc("@item:intable Private conversation finished", "Finished");
				} else {
					entry << i18nc("@item:intable Conversation is not private", "Not Private");
				}
			}
			if ( fingerprint->trust && fingerprint->trust[0] ){
				entry << i18nc( "@item:intable The fingerprint is trusted", "Yes" );
			} else {
				entry << i18nc( "@item:intable The fingerprint is not trusted", "No" );
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

void OtrlConfInterface::verifyFingerprint( const QString &strFingerprint, bool trust ){
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
		kDebug(14318) << "could not find fingerprint";
	}
}

bool OtrlConfInterface::isVerified( const QString &strFingerprint ){
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


void OtrlConfInterface::forgetFingerprint( const QString &strFingerprint ){
	Fingerprint *fingerprint;
	
	fingerprint = findFingerprint( strFingerprint );
	otrl_context_forget_fingerprint( fingerprint, 1 );
	otrl_privkey_write_fingerprints( userstate, QString(QString(KGlobal::dirs()->saveLocation("data", "kopete_otr/", true )) + "fingerprints").toLocal8Bit() );
}

Fingerprint *OtrlConfInterface::findFingerprint( const QString &strFingerprint ){
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

bool OtrlConfInterface::isEncrypted( const QString &strFingerprint ){
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
