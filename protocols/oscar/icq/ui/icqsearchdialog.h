/*
 Kopete Oscar Protocol
 icqsearchdialog.h - search for people

 Copyright (c) 2005 Matt Rogers <mattr@kde.org>

 Kopete (c) 2002-2005 by the Kopete developers <kopete-devel@kde.org>

 *************************************************************************
 *                                                                       *
 * This library is free software; you can redistribute it and/or         *
 * modify it under the terms of the GNU Lesser General Public            *
 * License as published by the Free Software Foundation; either          *
 * version 2 of the License, or (at your option) any later version.      *
 *                                                                       *
 *************************************************************************
*/

#ifndef ICQSEARCHDIALOG_H
#define ICQSEARCHDIALOG_H

#include <kdialogbase.h>
#include "icquserinfo.h"

class ICQAccount;
class ICQSearchBase;
class ICQContact;
class ICQUserInfoWidget;
/**
@author Kopete Developers
*/
class ICQSearchDialog : public KDialogBase
{
Q_OBJECT
public:
	ICQSearchDialog( ICQAccount* account, QWidget* parent = 0, const char* name = 0 );
	~ICQSearchDialog();

private slots:
	void startSearch();
	void stopSearch();
	void addContact();
	void clearResults();
	void closeDialog();
	void userInfo();
	void closeUserInfo();
	void newSearch();

	/// Enable/disable buttons when the selection changes
	void resultSelectionChanged();
	
	/// Add a search result to the listview
	void newResult( const ICQSearchResult& info );
	
	/// The search is finished
	void searchFinished( int numLeft );

private:
	ICQAccount* m_account;
	ICQSearchBase* m_searchUI;
	ICQContact* m_contact;
	ICQUserInfoWidget* m_infoWidget;
	
	void clearFields();
};

#endif

//kate: indent-mode csands; space-indent off; replace-tabs off; tab-width 4;
