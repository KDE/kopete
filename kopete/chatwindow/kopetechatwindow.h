/*
    kopetechatwindow.h - Chat Window

    Copyright (c) 2002 by Olivier Goffart <ogoffart@tiscalinet.be>

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

#ifndef KOPETECHATWINDOW_H
#define KOPETECHATWINDOW_H

#include <kparts/mainwindow.h>
#include <qmovie.h>
#include "kopetecontact.h"

class KopeteMessage;
class KopeteMessageManager;
class KAction;
class KToggleAction;
class KActionMenu;
class KopeteContact;
class KopeteProtocol;
class KTempFile;
class QPixmap;
class QTabWidget;
class KSqueezedTextLabel;
class KPushButton;
class QVBox;
class QVBoxLayout;
class QFrame;
class KTabWidget;
class QLabel;
class KopeteEmoticonAction;
class KopeteView;
class ChatView;

typedef  QPtrList<KopeteContact>  KopeteContactPtrList;

class KopeteChatWindow : public KParts::MainWindow
{
	Q_OBJECT

public:
	/**
	 * Find the appropriate window for a ChatView of the given protocol to
	 * dock into. If no such window exists, create one.
	 * @param protocol The protocol we are creating a view for
	 * @return A KopeteChatWindow suitable for docking a ChatView into. Guaranteed
	 *  to be a valid pointer.
	 */
	static KopeteChatWindow *window( KopeteProtocol *protocol );
	~KopeteChatWindow();

	KopeteMessage currentMessage();

	/**
	 * Attach an unattached chatview to this window
	 * @param chat The chat view to attach
	 */
	void attachChatView( ChatView *chat );

	/**
	 * Detach a chat view from this window
	 * @param chat The chat view to detach
	 */
	void detachChatView( ChatView *chat );

	/**
	 * Returns the number of chat views attached to this window
	 */
	const int chatViewCount() { return chatViewList.count(); }

	/**
	 * Returns the chatview in the currently active tab, or the only chat view
	 * if chatViewCount() == 1
	 */
	ChatView *activeView();

	void updateMembersActions();
	void setStatus( const QString & );
	void setSendEnabled( bool );

	KTempFile *backgroundFile;
	QPtrList<ChatView> chatViewList;

private:
	// All KopeteChatWindows are created by the findWindow function
	KopeteChatWindow(QWidget *parent = 0, const char* name = "KopeteChatWindow" );

	void initActions(void);
	void saveOptions(void);
	void readOptions(void);
	void checkDetachEnable();
	void createTabBar();
	void deleteTabBar();
	void addTab( ChatView* );
	void setPrimaryChatView( ChatView* );

	ChatView *m_activeView;
	ChatView *m_popupView;
	bool widgetSet;
	bool tabsEnabled;
	const KopeteContact *m_us;
	bool updateBg;
	KTabWidget *m_tabBar;
	KSqueezedTextLabel *m_status;
	KPushButton* m_button_send;
	QVBox *vBox;
	QVBoxLayout *mainLayout;
	QFrame *mainArea;
	QLabel *anim;
	QMovie animIcon;
	QPixmap normalIcon;
	QWidget *statusArea;

	KAction *chatSend;
	KAction *historyUp;
	KAction *historyDown;
	KAction *nickComplete;

	KToggleAction *viewStatusBar;

	KAction *tabLeft;
	KAction *tabRight;
	KAction *tabDetach;
	KAction* tabClose;

	KToggleAction* membersLeft;
	KToggleAction* membersRight;
	KToggleAction* toggleMembers;

	KopeteEmoticonAction *actionSmileyMenu;
	KActionMenu *actionActionMenu;
	KActionMenu *actionContactMenu;
	KActionMenu *actionDetachMenu;
	KActionMenu *actionTabPlacementMenu;
	QString statusMsg;

signals:
	void closing(KopeteChatWindow*);

public slots:
	void slotSmileyActivated(const QString &);
	void setActiveView( QWidget *active );
	void updateBackground( const QPixmap &pm );

private slots:
//	void slotPrepareSmileyMenu();
	void slotPrepareContactMenu();
	void slotPrepareDetachMenu();
	void slotPreparePlacementMenu();

	void slotCut();
	void slotCopy();
	void slotPaste();

	void slotSetBgColor();
	void slotSetFgColor();
	void slotSetFont();

	void slotHistoryUp();
	void slotHistoryDown();

	void slotSendMessage();
	void slotChatSave();
	void slotChatPrint();

	void slotPreviousTab();
	void slotNextTab();
	void slotDetachChat( int newWindowIndex = -1 );
	void slotPlaceTabs( int tabPlacement );

	void slotViewMenuBar();
	void slotViewStatusBar();

	void slotConfKeys();
	void slotConfToolbar();

	void slotViewMembersLeft();
	void slotViewMembersRight();
	void slotToggleViewMembers();
	const void slotEnableUpdateBg() { updateBg = true; }

	void slotSetCaption( bool );
	void slotUpdateCaptionIcons( const ChatView * );
	void slotChatClosed();
	void slotTabContextMenu( QWidget*, const QPoint & );
	void slotStopAnimation( ChatView* );
	void slotNickComplete();

protected:
	virtual void closeEvent( QCloseEvent *e );
	virtual bool queryExit( );
	virtual void windowActivationChange(bool);
	virtual bool eventFilter( QObject *o, QEvent *e );
};

#endif
