/*
    chatview.h - Chat View

    Copyright (c) 2002-2004 by Olivier Goffart       <ogoffart@tiscalinet.be>

    Kopete    (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef CHATVIEW_H
#define CHATVIEW_H

#include "kopeteview.h"

#include <ktextedit.h>
#include <kdockwidget.h>

#include <qptrdict.h>

class QTimer;

class ChatMembersListWidget;
class ChatMessagePart;

class KopeteChatWindow;
class KopeteRichTextEditPart;

class KTextEdit;
class KTabWidget;
class KCompletion;

class KopeteChatViewPrivate;

namespace Kopete
{
class Contact;
class ChatSession;
}

namespace KParts
{
class Part;
}

/**
 * @author Olivier Goffart
 */
class ChatView : public KDockMainWindow, public KopeteView
{
	friend class KopeteChatViewTip;

	Q_OBJECT
public:
	ChatView( Kopete::ChatSession *manager, const char *name = 0 );
	~ChatView();
	
	ChatMembersListWidget *membersList() { return m_membersList; }
	ChatMessagePart *messagePart() { return m_messagePart; }

	/**
	 * Adds text into the edit area. Used when you select an emoticon
	 * @param text The text to be inserted
	 */
	void addText( const QString &text );

	/**
	 * Saves window settings such as splitter positions
	 */
	void saveOptions();

	/**
	 * Sets the text to be displayed on the status label
	 * @param text The text to be displayed
	 */
	void setStatusText( const QString &text );

	/**
	 * Tells this view it is the active view
	 */
	void setActive( bool value );

	/**
	 * Clears the chat buffer
	 *
	 * Reimplemented from KopeteView
	 */
	virtual void clear();

	/**
	 * Sets the text to be displayed on tab label and window caption
	 */
	virtual void setCaption( const QString &text, bool modified );

	/**
	 * Changes the pointer to the chat widnow. Used to re-parent the view
	 * @param parent The new chat window
	 */
	void setMainWindow( KopeteChatWindow* parent );

	/**
	 * Sets the current message in the chat window
	 * @param parent The new chat window
	 */
	virtual void setCurrentMessage( const Kopete::Message &newMessage );

	void setTabBar( KTabWidget *tabBar );

	/**
	 * Sets the placement of the chat members list.
	 * DockLeft, DockRight, or DockNone.
	 * @param dp The dock position of the list
	 */
	void placeMembersList( KDockWidget::DockPosition dp = KDockWidget::DockRight );

	/**
	 * Shows or hides the chat members list
	 */
	void toggleMembersVisibility();

	/**
	 * Returns the message currently in the edit area
	 * Reimplemented from KopeteView
	 * @return The Kopete::Message object for the message
	 */
	virtual Kopete::Message currentMessage();

	/**
	 * Returns the chat window this view is in
	 * @return The chat window
	 */
	KopeteChatWindow *mainWindow() const { return m_mainWindow; }

	/**
	 * Returns the current position of the chat member slist
	 * @return The position of the chat members list
	 */
	const KDockWidget::DockPosition membersListPosition()  { return membersDockPosition; }

	/**
	 * Returns whether or not the chat member list is visible
	 * @return Is the chat member list visible?
	 */
	bool visibleMembersList();

	const QString &statusText();

	bool docked() { return ( m_tabBar != 0L ); }

	QString &caption() const;

	bool sendInProgress();

	void historyUp();

	void historyDown();

	/** Reimplemented from KopeteView **/
	virtual void raise( bool activate=false );

	/** Reimplemented from KopeteView **/
	virtual void makeVisible();

	/** Reimplemented from KopeteView **/
	virtual bool isVisible();

	/** Reimplemented from KopeteView **/
	virtual KTextEdit *editWidget() { return m_edit; }
	
	/** Reimplemented from KopeteView **/
	virtual QWidget *mainWidget() { return this; }

	void nickComplete();

	/**
	 * Can we send messages now?
	 */
	bool canSend();

	/**
	 * Is the user typing right now?
	 */
	bool isTyping();
	
public slots:
	/**
	 * Initiates a cut action on the edit area of the chat view
	 */
	void cut();

	/**
	 * Initiates a copy action
	 * If there is text selected in the HTML view, that text is copied
	 * Otherwise, the entire edit area is copied.
	 */
	void copy();

	/**
	 * Initiates a paste action into the edit area of the chat view
	 */
	void paste();

	/**
	 * Sets the foreground color of the entry area, and outgoing messages
	 * @param newColor The new foreground color. If this is QColor(), then
	 * a color chooser dialog is opened
	 */
	void setFgColor( const QColor &newColor = QColor() );

	/**
	 * Sets the font of the edit area and outgoing messages to the specified value.
	 * @param newFont The new font to use.
	 */
	void setFont( const QFont &newFont );

	/**
	 * show a Font dialog and set the font selected by the user
	 */
	void setFont();

	/**
	 * Sets the background color of the entry area, and outgoing messages
	 * @param newColor The new background color. If this is QColor(), then
	 * a color chooser dialog is opened
	 */
	void setBgColor( const QColor &newColor = QColor() );

	/**
	 * Sends the text currently entered into the edit area
	 */
	virtual void sendMessage();

	/**
	 * Called when a message is received from someone
	 * @param message The message received
	 */
	virtual void appendMessage( Kopete::Message &message );

	/**
	 * Called when a typing event is received from a contact
	 * Updates the typing map and outputs the typing message into the status area
	 * @param contact The contact who is / isn't typing
	 * @param typing If the contact is typing now
	 */
	void remoteTyping( const Kopete::Contact *contact, bool typing );

	/** Reimplemented from KopeteView **/
	virtual void messageSentSuccessfully();

	virtual bool closeView( bool force = false );

	KParts::Part *part() const { return (KParts::Part*)( editpart ); }

signals:
	/**
	 * Emits when a message is sent
	 * @param message The message sent
	 */
	void messageSent( Kopete::Message & );

	/**
	 * Emits every 4 seconds while the user is typing
	 * @param bool if the user is typing right now
	 */
	void typing( bool );

	void messageSuccess( ChatView* );

	/**
	 * Emits when the chat view is shown
	 */
	void shown();

	void closing( KopeteView* );

	void activated( KopeteView* );

	void captionChanged( bool active );

	void updateStatusIcon( const ChatView* );

	/**
	 * Our send-button-enabled flag has changed
	 */
	void canSendChanged();

	/**
	 * Emitted when we re-parent ourselves with a new window
	 */
	void windowCreated();

private slots:
	void slotRepeatTimer();
	void slotRemoteTypingTimeout();
	void slotPropertyChanged( Kopete::Contact *contact, const QString &key, const QVariant &oldValue, const QVariant &newValue  );

	/**
	 * Called when a contact is added to the KMM instance (A new person joins the chat).
	 * Adds this contact to the typingMap and the contact list view
	 * @param c The contact that joined the chat
	 * @param suppress mean that no notifications are showed
	 */
	void slotContactAdded( const Kopete::Contact *c, bool suppress );

	/**
	 * Called when a contact is removed from the KMM instance (A person left the chat).
	 * Removes this contact from typingMap and the contact list view
	 * @param c The contact left the chat
	 * @param reason is the reason the contact left
	 * @param format The format of the reason message
	 * @param suppressNotification mean that no notifications are showed
	 */
	void slotContactRemoved( const Kopete::Contact *c, const QString& reason, Kopete::Message::MessageFormat format, bool suppressNotification=false );

	/**
	 * Called when a contact changes status, updates the contact list view and
	 * @param contact The contact who changed status
	 * @param status The new status of the contact
	 */
	void slotContactStatusChanged( Kopete::Contact *contact, const Kopete::OnlineStatus &status, const Kopete::OnlineStatus &oldstatus );

	/**
	 * Called when the chat's display name is changed
	 */
	void slotChatDisplayNameChanged();

	/**
	 * Called when text is changed in the edit area
	 */
	void slotTextChanged();

	void slotStopTimer();

	void slotMarkMessageRead();

	void slotToggleRtfToolbar( bool enabled );
	
protected:
	virtual void dragEnterEvent ( QDragEnterEvent * );
	virtual void dropEvent ( QDropEvent * );

private:
	enum KopeteTabState { Normal , Highlighted , Changed , Typing , Message , Undefined };
	enum MembersListPolicy { Smart = 0, Visible = 1, Hidden = 2 };

	QPtrDict<QTimer> m_remoteTypingMap;
	ChatMessagePart *m_messagePart; //move to d-pointer
	
	MembersListPolicy membersStatus;
	QStringList historyList;
	int historyPos;
	QString unreadMessageFrom;
	KTextEdit* m_edit;
	ChatMembersListWidget *m_membersList; //move to d-pointer
	
	QString m_lastMatch;
	QString m_status;
	KCompletion *mComplete;

	KopeteTabState m_tabState;
	KDockWidget *viewDock;
	KDockWidget *membersDock;
	KDockWidget *editDock;
	KTabWidget *m_tabBar;
	KopeteRichTextEditPart *editpart;

	KAction *copyAction;

	// These control the position and visibility of the chat member list
	KDockWidget::DockPosition membersDockPosition;

	KopeteChatWindow *m_mainWindow;

	QTimer *m_typingRepeatTimer;
	QTimer *m_typingStopTimer;

	void setTabState( KopeteTabState state = Undefined );

	/**
	 * Creates the members list widget
	 */
	void createMembersList();

	/**
	 * Read in saved options, such as splitter positions
	 */
	void readOptions();

	void sendInternalMessage( const QString &msg, Kopete::Message::MessageFormat format = Kopete::Message::PlainText );

	KopeteChatViewPrivate *d;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

