/*
    yahooaddcontact.cpp - UI Page for Adding a Yahoo Contact

    Copyright (c) 2003 by Gav Wood               <gav@kde.org>
    Copyright (c) 2003 by Matt Rogers            <mattrogers@sbcglobal.net>
    Based on code by Duncan Mac-Vicar Prett      <duncan@kde.org>
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
// Own Header
#include "yahooaddcontact.h"

// QT Includes
#include <QLayout>
#include <QVBoxLayout>

// KDE Includes
#include "yahoo_protocol_debug.h"
#include <QLineEdit>

// Kopete Includes
#include <addcontactpage.h>
#include <kopeteaccount.h>

// Local Includes
#include "ui_yahooadd.h"
#include "yahooaccount.h"

// Yahoo Add Contact page
YahooAddContact::YahooAddContact(YahooProtocol *owner, QWidget *parent): AddContactPage(parent)
{
	qCDebug(YAHOO_PROTOCOL_LOG) << "YahooAddContact::YahooAddContact(<owner>, <parent>, " << objectName() << ")";

	QVBoxLayout *topLayout = new QVBoxLayout( this );
	QWidget* w = new QWidget( this );
	topLayout->addWidget( w );
	theDialog = new Ui::YahooAddContactBase;
	theDialog->setupUi( w );
	theProtocol = owner;
	theDialog->contactID->setFocus();
}

// Destructor
YahooAddContact::~YahooAddContact()
{
	qCDebug(YAHOO_PROTOCOL_LOG) ;
	delete theDialog;
}

bool YahooAddContact::validateData()
{
	qCDebug(YAHOO_PROTOCOL_LOG) ;

	return !theDialog->contactID->text().isEmpty();
}

bool YahooAddContact::apply(Kopete::Account *theAccount, Kopete::MetaContact *theMetaContact)
{
	qCDebug(YAHOO_PROTOCOL_LOG) ;

	QString displayName = theDialog->contactID->text();
	YahooAccount* myAccount = static_cast<YahooAccount*>(theAccount);
	myAccount->addContact(theDialog->contactID->text().toLower(), theMetaContact, Kopete::Account::ChangeKABC );
	return true;
}

