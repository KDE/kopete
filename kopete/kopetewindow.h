/*
    kopetewindow.h  -  Kopete Main Window

	Copyright (c) 2001-2002 by Duncan Mac-Vicar Prett   <duncan@kde.org>
    Copyright (c) 2001-2002 by Stefan Gehn <sgehn@gmx.net>

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

#ifndef KOPETEWINDOW_H
#define KOPETEWINDOW_H

#include <kmainwindow.h>


class QLabel;
class KAction;
class KActionMenu;
class KToggleAction;
class KSelectAction;
class KopeteSystemTray;
class QListViewItem;
class KopeteContactListView;

/**
 * @author Duncan Mac-Vicar P. <duncan@kde.org>
 *
 */

class KopeteWindow : public KMainWindow
{
Q_OBJECT

public:
	KopeteWindow ( QWidget *parent=0, const char *name=0 );
	~KopeteWindow();

protected:
	virtual void closeEvent(QCloseEvent*);
	virtual bool queryExit();

private slots:
	void showToolbar();
	void slotExecuted(QListViewItem *);
	void slotToggleShowOffliners(void);
	void slotConfigChanged(void);
	void slotConfKeys(void);
	void slotConfToolbar(void);

	void slotQuit();
public:
	KopeteContactListView *contactlist;

	// Some Actions
	KAction* actionAddContact;
	
	KActionMenu* actionConnectionMenu;
	KAction* actionConnect;
	KAction* actionDisconnect;
	
	KActionMenu* actionAwayMenu;
	KAction* actionSetAway;
	KAction* actionSetAvailable;
	
	KAction* actionPrefs;
	KAction* actionQuit;
	KAction* actionSave;
	KToggleAction *toolbarAction;
	KToggleAction *actionShowOffliners;
	KAction *actionShowTransfers;

	KopeteSystemTray *tray;

private:
	void initView ( void );
	void initActions ( void );
	void initSystray ( void );
	void loadOptions(void);
	void saveOptions(void);

	bool isClosing;
};

#endif
/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

