/***************************************************************************
                          Kopete Instant Messenger
							 kopetewindow.h
                            -------------------
				(C) 2001-2002 by Duncan Mac-Vicar P. <duncan@kde.org>
				(C) 2001-2002 by Stefan Gehn <sgehn@gmx.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KOPETEWINDOW_H
#define KOPETEWINDOW_H

#include <kmainwindow.h>

class QLabel;
class KAction;
class KToggleAction;
class KSelectAction;
class ContactList;
class KopeteSystemTray;
class QListViewItem;

class KopeteWindow : public KMainWindow
{
Q_OBJECT

public:
	KopeteWindow ( QWidget *parent=0, const char *name=0 );
	~KopeteWindow();

private slots:
	void showToolbar(void);
	void slotExecuted( QListViewItem * );

public:
	ContactList *contactlist;

	// Some Actions
	KAction* actionAddContact;
	KAction* actionConnect;
	KAction* actionDisconnect;
	KAction* actionSetAway;
	KAction* actionPrefs;
	KAction* actionQuit;
	KToggleAction *toolbarAction;

//	KSelectAction* actionStatus;
//	KAction* actionHide;

	KopeteSystemTray *tray;

private:
	void initView ( void );
	void initActions ( void );
	void initSystray ( void );
	bool queryExit(void);
	void loadOptions(void);
	void saveOptions(void);
	QWidget *mainwidget;
};

#endif
