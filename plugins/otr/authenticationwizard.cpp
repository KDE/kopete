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

#include "authenticationwizard.h"

#include <kdebug.h>
#include <klocale.h>
#include <kopetecontact.h>
#include <knotification.h>
#include <kiconloader.h>

#include <QGroupBox>
#include <QProgressBar>
#include <kopeteview.h>
#include <kopeteaccount.h>

QList<AuthenticationWizard*> wizardList;

AuthenticationWizard::AuthenticationWizard(QWidget *parent, ConnContext *context, Kopete::ChatSession *session, bool initiate, const QString &question):QWizard(parent){
	this->context = context;
	this->session = session;
	this->initiate = initiate;
	this->question = question;

	wizardList.append(this);
	setAttribute(Qt::WA_DeleteOnClose);

	setPage(Page_SelectMethod, createIntroPage());
	setPage(Page_QuestionAnswer, createQAPage());
	setPage(Page_SharedSecret, createSSPage());
	setPage(Page_ManualVerification, createMVPage());
	setPage(Page_Wait1, new WaitPage(i18n("Waiting for %1...", OtrlChatInterface::self()->formatContact(session->members().first()->contactId()))));
	setPage(Page_Wait2, new WaitPage(i18n("Checking if answers match...")));
	setPage(Page_Final, createFinalPage());

	if(!initiate){
		if(question == NULL){
			setStartId(Page_SharedSecret);
		} else {
			setStartId(Page_QuestionAnswer);
		}
	}

	connect(this, SIGNAL(rejected()), this, SLOT(cancelVerification()));
	connect(rbQA, SIGNAL(clicked()), this, SLOT(updateInfoBox()));
	connect(rbSS, SIGNAL(clicked()), this, SLOT(updateInfoBox()));
	connect(rbMV, SIGNAL(clicked()), this, SLOT(updateInfoBox()));

	updateInfoBox();

	if ( session->account()->isBusy() )
		return;

	resize(rbMV->width() * 1.5, rbMV->width() * 0.75);
	show();

	if ( !session->view()->mainWidget() || !session->view()->mainWidget()->isActiveWindow() ) {
		KNotification *notification = new KNotification( "kopete_info_event", KNotification::CloseWhenWidgetActivated | KNotification::CloseOnTimeout );
		notification->setText( i18n( "Incoming authentication request from %1", OtrlChatInterface::self()->formatContact( session->members().first()->contactId() ) ) );
		notification->setPixmap( SmallIcon( "kopete" ) );
		notification->setWidget( this );
		notification->setActions( QStringList() << i18n( "View" ) << i18n( "Close" ) );
		connect( notification, SIGNAL(activated(uint)), SLOT(notificationActivated(uint)) );
		notification->sendEvent();
	}

}


AuthenticationWizard::~AuthenticationWizard(){
	wizardList.removeAll(this);
}

AuthenticationWizard *AuthenticationWizard::findWizard(Kopete::ChatSession *session){
	for(int i = 0; i < wizardList.size(); i++){
		if(wizardList.at(i)->session == session){
			return wizardList.at(i);
		}
	}
	return 0;
}

QWizardPage *AuthenticationWizard::createIntroPage(){

	QWizardPage *page = new QWizardPage();
	page->setTitle(i18nc("@title", "Select authentication method"));

	rbQA = new QRadioButton(i18n("Question and Answer"));
	rbSS = new QRadioButton(i18n("Shared Secret"));
	rbMV = new QRadioButton(i18n("Manual fingerprint verification"));

	QGroupBox *frame = new QGroupBox();
	QVBoxLayout *frameLayout = new QVBoxLayout();
	frame->setLayout(frameLayout);
	infoLabel = new QLabel();
	infoLabel->setWordWrap(true);
	frameLayout->addWidget(infoLabel);

	QVBoxLayout *layout = new QVBoxLayout();
	layout->addWidget(rbQA);
	layout->addWidget(rbSS);
	layout->addWidget(rbMV);

	layout->addSpacing(30);
	layout->addWidget(frame);

	page->setLayout(layout);

	rbQA->setChecked(true);

	return page;
}

QWizardPage *AuthenticationWizard::createQAPage(){
	QWizardPage *page = new QWizardPage();
	QGridLayout *layout = new QGridLayout();

	if(initiate){
		page->setTitle(i18nc("@title", "Question and Answer"));

		lQuestion = new QLabel(i18nc("@info", "Enter a question that only %1 is able to answer:", session->members().first()->contactId()));
		layout->addWidget(lQuestion);
		leQuestion = new QLineEdit();
		layout->addWidget(leQuestion);
		lAnswer = new QLabel(i18nc("@info", "Enter the answer to your question:"));
		layout->addWidget(lAnswer);
	} else {
		if(question != NULL){
			page->setTitle(i18nc("@info", "Authentication with %1", session->members().first()->contactId()));
			lQuestion = new QLabel(i18nc("@info", "%1 would like to verify your authentication. Please answer the following question in the field below:", session->members().first()->contactId()));
			lQuestion->setWordWrap(true);
			layout->addWidget(lQuestion);
			lAnswer = new QLabel(question);
			lAnswer->setWordWrap(true);
			layout->addWidget(lAnswer);
		}
	}
	leAnswer = new QLineEdit();
	layout->addWidget(leAnswer);

	page->setLayout(layout);
	page->setCommitPage(true);
	return page;
}

QWizardPage *AuthenticationWizard::createSSPage(){
	QWizardPage *page = new QWizardPage();
	QGridLayout *layout = new QGridLayout();

	if(initiate){
		page->setTitle(i18nc("@title", "Shared Secret"));

		layout->addWidget(new QLabel(i18nc("@info", "Enter a secret passphrase known only to you and %1:", session->members().first()->contactId())));
	} else {
		page->setTitle(i18nc("@title", "Authentication with %1", session->members().first()->contactId()));
		layout->addWidget(new QLabel(i18nc("@info", "Enter the secret passphrase known only to you and %1:", session->members().first()->contactId())));
	}
	leSecret = new QLineEdit();
	layout->addWidget(leSecret);

	page->setLayout(layout);
	page->setCommitPage(true);
	return page;
}

QWizardPage *AuthenticationWizard::createMVPage(){
	QWizardPage *page = new QWizardPage();
	page->setTitle(i18nc("@title", "Manual Verification"));

	QGridLayout *layout = new QGridLayout();

	QLabel *lMessage1 = new QLabel(i18nc("@info", "Contact %1 via another secure channel and verify that the following fingerprint is correct:", session->members().first()->contactId()));
	lMessage1->setWordWrap(true);
	layout->addWidget(lMessage1);
	layout->addWidget(new QLabel(OtrlChatInterface::self()->fingerprint(session)));

	cbManualAuth = new QComboBox();
	cbManualAuth->addItem(i18nc("@item:inlistbox ...verified that", "I have not"));
	cbManualAuth->addItem(i18nc("@item:inlistbox ...verified that", "I have"));
	cbManualAuth->setSizeAdjustPolicy(QComboBox::AdjustToContents);

	if( OtrlChatInterface::self()->isVerified(session)){
		cbManualAuth->setCurrentIndex(1);
	} else {
		cbManualAuth->setCurrentIndex(0);
	}

	QLabel *lMessage2 = new QLabel(i18nc("@info:label I have...", "verified that this is in fact the correct fingerprint for %1", session->members().first()->contactId()));
	lMessage2->setWordWrap(true);

	QHBoxLayout *verifyLayout = new QHBoxLayout();
	verifyLayout->addWidget(cbManualAuth);
	verifyLayout->addWidget(lMessage2);

	QFrame *frame = new QFrame();
	frame->setLayout(verifyLayout);
	layout->addWidget(frame);

	page->setLayout(layout);
	return page;

}

QWizardPage *AuthenticationWizard::createFinalPage(){
	QWizardPage *page = new QWizardPage();
	QGridLayout *layout = new QGridLayout();

	lFinal = new QLabel();
	lFinal->setWordWrap(true);
	layout->addWidget(lFinal);
	page->setLayout(layout);
	return page;

}

int AuthenticationWizard::nextId() const{
	if(currentId() == Page_SelectMethod){
		if(rbQA->isChecked())
			return Page_QuestionAnswer;
		if(rbSS->isChecked())
			return Page_SharedSecret;
		if(rbMV->isChecked())
			return Page_ManualVerification;
	}
	if(currentId() == Page_SharedSecret || currentId() == Page_QuestionAnswer){
		if(initiate){
			return Page_Wait1;
		} else {
			return Page_Wait2;
		}
	}
	if(currentId() == Page_Wait1){
		return Page_Wait2;
	}
	if(currentId() == Page_Wait2){
		return Page_Final;
	}
	return -1;
}

bool AuthenticationWizard::validateCurrentPage(){
	kDebug(14318) << "currentId:" << currentId();
	switch(currentId()){
		case 1:
			if(initiate){
				OtrlChatInterface::self()->initSMPQ(context, session, leQuestion->text(), leAnswer->text());
			} else {
				OtrlChatInterface::self()->respondSMP(context, session, leAnswer->text());
			}
			break;
		case 2:
			if(initiate){
				OtrlChatInterface::self()->initSMP(context, session, leSecret->text());
			} else {
				OtrlChatInterface::self()->respondSMP(context, session, leSecret->text());
			}
			break;
		case 3:
			if(cbManualAuth->currentIndex() == 0 ){
				OtrlChatInterface::self()->setTrust(session, false);
			} else {
				OtrlChatInterface::self()->setTrust(session, true);
			}
			OtrlChatInterface::self()->emitGoneSecure( session, OtrlChatInterface::self()->privState( session ) );
			break;
	}
	return true;
}

void AuthenticationWizard::cancelVerification(){
	kDebug(14318) << "cancelVerification...";
	if(!initiate){
		OtrlChatInterface::self()->abortSMP(context, session);
	}
}

void AuthenticationWizard::nextState(){
	if(currentId() == Page_Wait1){
		((WaitPage*)currentPage())->ready();
		next();
	}
}

void AuthenticationWizard::finished(bool success, bool trust){
	kDebug(14318) << "authWizard finished";
	if(currentId() == Page_Wait2){
		kDebug(14318) << "Yes, in wait_page2";
		((WaitPage*)currentPage())->ready();
		next();
		if(success){
			kDebug(14318) << "auth succeeded";
			currentPage()->setTitle(i18n("Authentication successful"));
			if(question != NULL || rbQA->isChecked()){
				if(initiate){
					kDebug(14318) << "initiate";
					lFinal->setText(i18n("The authentication with %1 was completed successfully. The conversation is now secure.", OtrlChatInterface::self()->formatContact(session->members().first()->contactId())));
				} else {
					kDebug(14318) << "not initiate";
					if(trust){
						lFinal->setText(i18n("The authentication with %1 was completed successfully. The conversation is now secure.", OtrlChatInterface::self()->formatContact(session->members().first()->contactId())));
					} else {
						lFinal->setText(i18n("<b>%1</b> has successfully authenticated you. You may want to authenticate this contact as well by asking your own question.", OtrlChatInterface::self()->formatContact(session->members().first()->contactId())));
					}
				}
			} else {
				lFinal->setText(i18n("The authentication with %1 was completed successfully. The conversation is now secure.", OtrlChatInterface::self()->formatContact(session->members().first()->contactId())));
			}
		} else {
			currentPage()->setTitle(i18n("Authentication failed"));
			lFinal->setText(i18n("The authentication with %1 failed. To make sure you are not talking to an imposter, try again using the manual fingerprint verification method. Note that the conversation is now insecure.", OtrlChatInterface::self()->formatContact(session->members().first()->contactId())));
		}
	}

	setOption(QWizard::NoCancelButton, true);

}

void AuthenticationWizard::aborted(){
	if(currentId() == Page_SharedSecret || currentId() == Page_QuestionAnswer){
		next();
	}
	if(currentId() == Page_Wait1){
		next();
	}
	if(currentId() == Page_Wait2){
		next();
	}
	currentPage()->setTitle(i18n("Authentication aborted"));
	lFinal->setText(i18n("%1 has aborted the authentication process. To make sure you are not talking to an imposter, try again using the manual fingerprint verification method.", OtrlChatInterface::self()->formatContact(session->members().first()->contactId())));

	setOption(QWizard::NoCancelButton, true);
}

void AuthenticationWizard::updateInfoBox(){
	if(rbQA->isChecked()){
		infoLabel->setText(i18n("Ask %1 a question, the answer to which is known only to you and them. If the answer does not match, you may be talking to an imposter.", OtrlChatInterface::self()->formatContact(session->members().first()->contactId())));
	} else if(rbSS->isChecked()) {
		infoLabel->setText(i18n("Pick a secret known only to you and %1. If the secret does not match, you may be talking to an imposter. Do not send the secret through the chat window, or this authentication method could be compromised with ease.", OtrlChatInterface::self()->formatContact(session->members().first()->contactId())));
	} else {
		infoLabel->setText(i18n("Verify %1's fingerprint manually. For example via a phone call or signed (and verified) email.", OtrlChatInterface::self()->formatContact(session->members().first()->contactId())));
	}
}

void AuthenticationWizard::notificationActivated( unsigned int id){
	kDebug(14318) << "notificationActivated. ButtonId" << id;
	if( id == 1 ){
		// raise the view to bring the chatwindow + authwizard to current desktop and on top
		session->view()->raise( true );
		// now grab focus and keyboard again to the auth-wizard
		setFocus(Qt::ActiveWindowFocusReason);
		leAnswer->grabKeyboard();
	}
}

WaitPage::WaitPage(const QString &text){
	canContinue = false;
	setTitle(i18nc("@title","Authenticating contact..."));
	QVBoxLayout *layout = new QVBoxLayout();
	layout->addWidget(new QLabel(text));
	layout->addStretch();
	QProgressBar *progressBar = new QProgressBar();
	progressBar->setMinimum(0);
	progressBar->setMaximum(0);
	layout->addWidget(progressBar);
	layout->addStretch();
	setCommitPage(true);
	setLayout(layout);
}
