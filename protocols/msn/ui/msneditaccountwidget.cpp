/*
    msneditaccountwidget.cpp - MSN Account Widget

    Copyright (c) 2003 by Olivier Goffart  <ogoffart@tiscalinet.be>

    Kopete    (c) 2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include <qlayout.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qgroupbox.h>
#include <qlistbox.h>
#include <qpushbutton.h>
#include <qimage.h>
#include <qregexp.h>


#include <klocale.h>
#include <kfiledialog.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>

#include "msneditaccountwidget.h"
#include "msnaccount.h"
#include "msncontact.h"
#include "msnnotifysocket.h"

MSNEditAccountWidget::MSNEditAccountWidget(MSNProtocol *proto, KopeteAccount *ident, QWidget *parent, const char * )
				  : MSNEditAccountUI(parent), EditAccountWidget(ident)
{
	m_protocol=proto;

	//TODO: actually, i don't know how to set fonts for qlistboxitem
	 label_font->hide();

	//default fields
	if(ident)
	{
		if(ident->rememberPassword())
		{
			m_rememberpasswd->setChecked(true);
			m_password->setText(ident->password());
		}
		m_login->setText(ident->accountId());
		//remove me after we can change account ids (Matt)
		m_login->setDisabled(true);
		m_autologin->setChecked((ident && ident->autoLogin()));

		MSNContact *myself=static_cast<MSNContact*>(m_account->myself());

		m_displayName->setText( myself->displayName() );
		m_phw->setText( m_account->pluginData( m_protocol , "PHW") );
		m_phm->setText( m_account->pluginData( m_protocol , "PHM") );
		m_phh->setText( m_account->pluginData( m_protocol , "PHH") );

		bool connected=m_account->isConnected();
		if(connected)
		{
			m_warning_1->hide();
			m_warning_2->hide();
		}
		m_phones->setEnabled(connected);
		m_displayName->setEnabled(connected);
		m_allowButton->setEnabled(connected);
		m_blockButton->setEnabled(connected);



		QStringList blockList=QStringList::split(',' ,m_account->pluginData(m_protocol,QString::fromLatin1("blockList")) );
		QStringList allowList=QStringList::split(',' ,m_account->pluginData(m_protocol,QString::fromLatin1("allowList")) );
		//QStringList reverseList=QStringList::split(',' ,m_account->pluginData(m_protocol,QString::fromLatin1("reverseList")) );

		for ( QStringList::Iterator it = blockList.begin(); it != blockList.end(); ++it )
		{
			m_BL->insertItem( *it);
		}
		for ( QStringList::Iterator it = allowList.begin(); it != allowList.end(); ++it )
		{
			m_AL->insertItem( *it);
		}

		m_blp->setChecked(m_account->pluginData(m_protocol,QString::fromLatin1("BLP"))=="BL");

		connect(m_allowButton,SIGNAL(pressed()), this, SLOT(slotAllow()));
		connect(m_blockButton,SIGNAL(pressed()), this, SLOT(slotBlock()));
		connect(m_RLButton,SIGNAL(pressed()), this, SLOT(slotShowReverseList()));

		m_displayPicture->setPixmap( locateLocal( "appdata", "msnpicture-"+ m_account->accountId().lower().replace(QRegExp("[./~]"),"-")  +".png" ) );
		connect(m_selectImage , SIGNAL(pressed()) , this , SLOT(slotSelectImage()));

		m_useDisplayPicture->setChecked(m_account->pluginData( m_protocol , "exportCustomPicture")=="1");
	}
	else
	{
		m_rememberpasswd->setChecked(true);
		tab_info->setDisabled(true);
		tab_contacts->setDisabled(true);
	}
}

MSNEditAccountWidget::~MSNEditAccountWidget()
{
}

KopeteAccount *MSNEditAccountWidget::apply()
{
	if(!m_account)
		m_account=new MSNAccount(m_protocol, m_login->text() );
	if(m_rememberpasswd->isChecked())
	{
		m_account->setPassword( m_password->text() );
	}
	else
		m_account->setPassword( QString::null );

	m_account->setAutoLogin(m_autologin->isChecked());

	m_account->setPluginData( m_protocol , "exportCustomPicture" , m_useDisplayPicture->isChecked() ? "1" : QString::null );
	static_cast<MSNAccount*>(m_account)->resetPictureObject();

	if(m_account->isConnected())
	{
		MSNContact *myself=static_cast<MSNContact*>(m_account->myself());
		MSNNotifySocket *notify=static_cast<MSNAccount*>(m_account)->notifySocket();
		if(m_displayName->text() != myself->displayName())
			static_cast<MSNAccount*>(m_account)->setPublicName(m_displayName->text());
		if(notify)
		{
			if(m_phw->text() != myself->phoneWork() && ( !m_phw->text().isEmpty() || !myself->phoneWork().isEmpty() ))
				notify->changePhoneNumber( "PHW" , m_phw->text() );
			if(m_phh->text() != myself->phoneHome() && ( !m_phh->text().isEmpty() || !myself->phoneHome().isEmpty() ))
				notify->changePhoneNumber( "PHH" , m_phh->text() );
			if(m_phm->text() != myself->phoneMobile() && ( !m_phm->text().isEmpty() || !myself->phoneMobile().isEmpty() ) )
				notify->changePhoneNumber( "PHM" , m_phm->text() );
			//(the && .isEmpty is because one can be null and the other empty)

			if( (m_account->pluginData(m_protocol,QString::fromLatin1("BLP"))=="BL") !=  m_blp->isChecked())
			{
				//yes, i know, calling sendcommand here is not verry clean.
				notify->sendCommand("BLP" , m_blp->isChecked() ? "BL" : "AL" );
			}
		}
	}
	return m_account;
}


bool MSNEditAccountWidget::validateData()
{
	QString userid = m_login->text();
	if( MSNProtocol::validContactId(userid) )
		return true;

	KMessageBox::sorry(this, i18n("<qt>You must enter a valid email address as login.</qt>"), i18n("MSN Messenger"));
	return false;
}

void MSNEditAccountWidget::slotAllow()
{
	//TODO: play with multiple selection
	QListBoxItem *item=m_BL->selectedItem();
	if(!item)
		return;
	QString handle=item->text();

	MSNNotifySocket *notify=static_cast<MSNAccount*>(m_account)->notifySocket();
	if(!notify)
		return;
	notify->removeContact(handle,0,MSNProtocol::BL);

	m_BL->takeItem(item);
	m_AL->insertItem(item);
}


void MSNEditAccountWidget::slotBlock()
{
	//TODO: play with multiple selection
	QListBoxItem *item=m_AL->selectedItem();
	if(!item)
		return;
	QString handle=item->text();

	MSNNotifySocket *notify=static_cast<MSNAccount*>(m_account)->notifySocket();
	if(!notify)
		return;
	notify->removeContact(handle,0,MSNProtocol::AL);

	m_AL->takeItem(item);
	m_BL->insertItem(item);
}

void MSNEditAccountWidget::slotShowReverseList()
{
	QStringList reverseList=QStringList::split(',' ,m_account->pluginData(m_protocol,QString::fromLatin1("reverseList")) );
	KMessageBox::informationList( this, i18n("Here you can see a list of contact which added you in their contactlist") , reverseList , i18n("Reverse List - MSN Plugin") );
}

void MSNEditAccountWidget::slotSelectImage()
{
	//FIXME: the change will take effect imadiatly, even if the user press cancel.
	if(!m_account) //FIXME: since we need toe accountId to create the file HERE (and it's the problem) we need the account
		return;

	QString filePath = KFileDialog::getOpenFileName( QString::null ,"*", 0l  , i18n( "MSN Display Picture" ));
	if(filePath.isEmpty())
		return;

	QString futurName=locateLocal( "appdata", "msnpicture-"+ m_account->accountId().lower().replace(QRegExp("[./~]"),"-")  +".png" );

	QImage img(filePath);
	img=img.smoothScale(96,96);
	if(!img.isNull() && img.save( futurName , "PNG"))
	{
		m_displayPicture->setPixmap( futurName );
	}
	else
	{
		KMessageBox::sorry(this, i18n("<qt>An error occured when trying to change the display picture. <br>"
						"Make sure that you have select a correct image file</qt>"), i18n("MSN Messenger"));
	}

}

#include "msneditaccountwidget.moc"

// vim: set noet ts=4 sts=4 sw=4:

