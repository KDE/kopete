 /*
    icqaddcontactpage.h  -  ICQ Protocol Plugin

    Copyright (c) 2002 by Stefan Gehn <metz AT gehn.net>
    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef ICQADDCONTACTPAGE_H
#define ICQADDCONTACTPAGE_H

#include <qwidget.h>
#include <addcontactpage.h>

/**
  *@author Duncan
  *@author Stefan Gehn
  */
class icqAddUI;
class QWidget;
class ICQAccount;
class ICQSearchResult;

class ICQAddContactPage : public AddContactPage
{
	Q_OBJECT

	public:
		ICQAddContactPage(ICQAccount *owner, QWidget *parent=0, const char *name=0);
		~ICQAddContactPage();

		virtual bool validateData();
		virtual bool apply(Kopete::Account* , Kopete::MetaContact *parentContact);

	public slots:

		void slotSearchResult(ICQSearchResult &res, const int missed);

	private slots:
		void slotStartSearch();
		void slotStopSearch();
		void slotClearResults();
		void slotSearchTabChanged(QWidget *);
		void slotTextChanged();
		void slotSelectionChanged();

	protected:
		void showEvent(QShowEvent *e);

	private:
		int searchMode;
		bool searching;
		ICQAccount *mAccount;
		icqAddUI *icqdata;

		void updateGui();
		void removeSearch();
};
#endif
