//
// Current author and maintainer: Grzegorz Jaskiewicz
//				gj at pointblue.com.pl
//
// Kopete initial author:
// Copyright (C) 	2002-2003	 Zack Rusin <zack@kde.org>
//
// gaducommands.h - all basic, and not-session dependent commands
// (meaning you don't have to be logged in for any
//  of these). These delete themselves, meaning you don't
//  have to/can't delete them explicitely and have to create
//  them dynamically (via the 'new' call).
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

#include <klineedit.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <klocale.h>


GaduEditAccount::GaduEditAccount( GaduProtocol *proto, KopeteAccount *ident,
                                  QWidget *parent, const char *name )
  : GaduAccountEditUI( parent, name ), EditAccountWidget( ident )
  ,
    protocol_( proto ), rcmd(0)
{

    if (!m_account){
	reg_in_progress=false;
	
	kdDebug(14100)<<"GJ"<<"NEW ACCOUNT WIZ"<<endl;

	radio1->setDisabled(false);
	radio2->setDisabled(false);
	textLabel2_2->setDisabled(false);
	textLabel1_2->setDisabled(false);
	emailedit_->setDisabled(false);
	passwordEdit2__->setDisabled(false);
    }
    else{
	kdDebug(14100)<<"GJ"<<"ACCOUNT EDIT"<<endl;
	radio1->setDisabled(true);
	radio2->setDisabled(true);
	textLabel2_2->setDisabled(true);
	textLabel1_2->setDisabled(true);
	emailedit_->setDisabled(true);
	passwordEdit2__->setDisabled(true);
	loginEdit_->setText(m_account->accountId());
	if (m_account->rememberPassword()){
	    passwordEdit_->setText(m_account->getPassword());
	}
	else{
	    passwordEdit_->setText("");
	}

        rememberCheck_->setChecked(m_account->rememberPassword());
	autoLoginCheck_->setChecked(m_account->autoLogin());
    }    
}

void GaduEditAccount::registrationComplete( const QString& title, const QString& )
{
	reg_in_progress=false;
	
	kdDebug(14100)<<"GJ"<<" ID " << title << endl;	    
	    
	// i am sure rcmd is valid, since it sends this signal ;)
	loginEdit_->setText(QString::number(rcmd->newUin()));
	passwordEdit_->setText(passwordEdit2__->text());

	radio1->setChecked(true);
	radio2->setChecked(false);
	radio1->setDisabled(true);
	radio2->setDisabled(true);
	textLabel2_2->setDisabled(true);
	textLabel1_2->setDisabled(true);
	emailedit_->setDisabled(true);
	passwordEdit2__->setDisabled(true);
        rememberCheck_->setChecked(true);
		    
	KMessageBox::information(this, i18n("<b>You've registered new acount</b>"), i18n("Gadu-gadu"));
}

void GaduEditAccount::registrationError( const QString& title, const QString& what )
{
	reg_in_progress=false;
	KMessageBox::information(this, what, title);
}

bool GaduEditAccount::validateData()
{

    // FIXME: I need to disable somehow next, finish and back button here
    if (reg_in_progress){
	return false;
    }

    if (radio1->isChecked()){
	if (loginEdit_->text().toInt()<0 || loginEdit_->text().toInt()==0){
	    KMessageBox::sorry(this, i18n("<b>UIN should be a positive number</b>"), i18n("Gadu-gadu"));
	    return false;
	}
    
	if (passwordEdit_->text().isEmpty() && rememberCheck_->isChecked()){
	    KMessageBox::sorry(this, i18n("<b>Enter Password please</b>"), i18n("Gadu-gadu"));
	    return false;
	}
    }
    else{
	if (emailedit_->text().isEmpty()){
	    KMessageBox::sorry(this, i18n("<b>Please enter a valid email addres</b>"), i18n("Gadu-gadu"));
	    return false;
	}
    
	if (passwordEdit2__->text().isEmpty()){
	    KMessageBox::sorry(this, i18n("<b>Enter Password please</b>"), i18n("Gadu-gadu"));
	    return false;
	}

	kdDebug(14100)<<"GJ"<<"email:" << emailedit_->text() <<endl;

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

    if (m_account){
	kdDebug(14100)<<"GJ"<<"ACCOUNT CHANGE"<<endl;	    
    }
    else{
	if (radio1->isChecked()){
	    kdDebug(14100)<<"GJ"<<"NEW ACCOUNT"<<endl;
    	    m_account = new GaduAccount( protocol_, loginEdit_->text() );
    	    if (!m_account){
		kdDebug(14100)<<"Couldn't create GaduAccount object, fatal!"<<endl;
		return NULL;
	    }
	}
	else{
	    // should never happend
	    return NULL;
	}
    }
    
    m_account->setAccountId(loginEdit_->text());
    m_account->setAutoLogin(autoLoginCheck_->isChecked());
    
    if(rememberCheck_->isChecked()){
        m_account->setPassword(passwordEdit_->text());
    }
    else{
        m_account->setPassword(QString::null);
    }
    
    m_account->setAutoLogin(autoLoginCheck_->isChecked());
    
    return m_account;
}

#include "gadueditaccount.moc"
