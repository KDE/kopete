//
// Current author and maintainer: Grzegorz Jaskiewicz
//				gj at pointblue.com.pl
//
// Kopete initial author:
// Copyright (C) 	2002-2003	 Zack Rusin <zack@kde.org>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.
//

#include "gadueditaccount.h"
#include "gaduaccount.h"
#include "gaduprotocol.h"
#include <qradiobutton.h>
#include <qlabel.h>
#include <qstring.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qbutton.h>
#include <qregexp.h>

#include <klineedit.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <klocale.h>

#define EMAIL_REGEXP "[\\w\\d\\.]{1,}\\@[\\w\\d\\.]{1,}"

GaduEditAccount::GaduEditAccount( GaduProtocol *proto, KopeteAccount *ident,
                                  QWidget *parent, const char *name )
  : GaduAccountEditUI( parent, name ), EditAccountWidget( ident )
  ,
    protocol_( proto ), rcmd(0)
{

    reg_in_progress=false;
    
    if (!m_account){
	
	radio1->setDisabled(false);
	radio2->setDisabled(false);
	emailedit_->setDisabled(false);
	passwordEdit2__->setDisabled(false);
    }
    else{
	radio1->setDisabled(true);
	radio2->setDisabled(true);
	radio2->setDisabled(true);
	loginEdit_->setDisabled(true);
	emailedit_->setDisabled(true);
	passwordEdit2__->setDisabled(true);
	loginEdit_->setText(m_account->accountId());
	if (m_account->rememberPassword()){
	    passwordEdit_->setText(m_account->password());
	}
	else{
	    passwordEdit_->setText("");
	}
	
	nickName->setText(m_account->myself()->displayName());
	
        rememberCheck_->setChecked(m_account->rememberPassword());
	autoLoginCheck_->setChecked(m_account->autoLogin());
    }    
}

void GaduEditAccount::registrationComplete( const QString& , const QString& )
{
	reg_in_progress=false;
	
	QRegExp regexp(EMAIL_REGEXP);
	regexp.search(emailedit_->text());
	
	// i am sure rcmd is valid, since it sends this signal ;)
	loginEdit_->setText(QString::number(rcmd->newUin()));
	passwordEdit_->setText(passwordEdit2__->text());
	nickName->setText(regexp.cap(1)+"-"+loginEdit_->text());
	
	radio1->setChecked(true);
	radio2->setChecked(false);
	radio1->setDisabled(true);
	radio2->setDisabled(true);
	textLabel2_2->setDisabled(true);
	textLabel1_2->setDisabled(true);
	emailedit_->setDisabled(true);
	passwordEdit2__->setDisabled(true);
        rememberCheck_->setChecked(true);
		    
	KMessageBox::information(this, i18n("<b>You've registered a new account.</b>"), i18n("Gadu-Gadu"));
}

void GaduEditAccount::registrationError( const QString& title, const QString& what )
{
	reg_in_progress=false;
	KMessageBox::information(this, what, title);
}

bool GaduEditAccount::validateData()
{
    QRegExp regexp(EMAIL_REGEXP);

    // FIXME: I need to disable somehow next, finish and back button here
    if (reg_in_progress){
	return false;
    }

    if (radio1->isChecked()){
	if (loginEdit_->text().toInt()<0 || loginEdit_->text().toInt()==0){
	    KMessageBox::sorry(this, i18n("<b>UIN should be a positive number.</b>"), i18n("Gadu-Gadu"));
	    return false;
	}
    
	if (passwordEdit_->text().isEmpty() && rememberCheck_->isChecked()){
	    KMessageBox::sorry(this, i18n("<b>Enter password please.</b>"), i18n("Gadu-Gadu"));
	    return false;
	}
    }
    else{
	if (emailedit_->text().isEmpty()){
	    KMessageBox::sorry(this, i18n("<b>Please enter a valid email address.</b>"), i18n("Gadu-Gadu"));
	    return false;
	}

	if (regexp.exactMatch(emailedit_->text())==FALSE){
	    KMessageBox::sorry(this, i18n("<b>Please enter a valid email address.</b>"), i18n("Gadu-Gadu"));
	    return false;
	}

	if (passwordEdit2__->text().isEmpty()){
	    KMessageBox::sorry(this, i18n("<b>Enter password please.</b>"), i18n("Gadu-Gadu"));
	    return false;
	}

	reg_in_progress=true;
	rcmd=new RegisterCommand(emailedit_->text(), 
				    passwordEdit2__->text(), this);
	
	connect( rcmd, SIGNAL(done(const QString&, const QString&)),
        	 SLOT(registrationComplete(const QString&, const QString&)) );
	connect( rcmd, SIGNAL(error(const QString&, const QString&)),
        	 SLOT(registrationError(const QString&, const QString&)) );
  
	rcmd->execute();
	
	return false;
		    
    }
    return true;
}

KopeteAccount* GaduEditAccount::apply()
{

    if (m_account==NULL){
	if (radio1->isChecked()){
    	    m_account = new GaduAccount( protocol_, loginEdit_->text() );
    	    if (!m_account){
		kdDebug(14100)<<"Couldn't create GaduAccount object, fatal!"<<endl;
		return NULL;
	    }
	    m_account->setAccountId(loginEdit_->text());
	}
	else{
	    // should never happend
	    return NULL;
	}
    }
    
    m_account->setAutoLogin(autoLoginCheck_->isChecked());
    
    if(rememberCheck_->isChecked()){
        m_account->setPassword(passwordEdit_->text());
    }
    else{
        m_account->setPassword();
    }
    
    m_account->myself()->rename(nickName->text());
    
    // this is changed only here, so i won't add any proper handling now
    m_account->setPluginData(m_account->protocol(), 
	    QString::fromLatin1("nickName"), nickName->text());
    
    m_account->setAutoLogin(autoLoginCheck_->isChecked());
    
    return m_account;
}

#include "gadueditaccount.moc"
