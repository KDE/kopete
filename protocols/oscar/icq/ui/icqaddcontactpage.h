 /*
    icqaddcontactpage.h  -  ICQ Protocol Plugin

    Copyright (c) 2002 by Stefan Gehn <metz@gehn.net>
    
    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

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
  *@author duncan
  *@author stefan
  */
class icqAddUI;
class ICQProtocol;
class QWidget;
class ICQEvent;
class ICQAccount;

class ICQAddContactPage : public AddContactPage
{
Q_OBJECT

public:
	ICQAddContactPage(ICQAccount *owner, QWidget *parent=0, const char *name=0);
	~ICQAddContactPage();

	// Duncan...FIXME, why public?
	icqAddUI *icqdata;
	ICQAccount *mAccount;
	virtual bool validateData();

public slots:
	void slotFinish( KopeteMetaContact *parentContact );
	void slotSearchResult ( ICQEvent *e );

private slots:
	void slotStartSearch( void );
	void slotStopSearch( void );
	void slotClearResults( void );
	void slotSearchTabChanged( QWidget * );
	void slotTextChanged( void );

private:
	int searchMode;
	ICQEvent *searchEvent;

	void updateGui( void );
	void removeSearch( void );
};
#endif
