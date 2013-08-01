/*
    chatview.h - Chat View

    Copyright (c) 2008      by Benson Tsai           <btsai@vrwarp.com>
    Copyright (c) 2002-2004 by Olivier Goffart       <ogoffart@kde.org>

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
#include "kopeteviewplugin.h"
//#include <k3dockwidget.h>
#include <ktextedit.h> // for covariant return type of editWidget
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMap>
#include <kvbox.h>

#include <kopete_export.h>

class QTimer;

class ChatTextEditPart;
class ChatMessagePart;

class KopeteChatWindow;

class KopeteChatViewPrivate;
class ChatWindowPlugin;

namespace KParts
{
    class Part;
}

namespace Kopete
{
	class Contact;
	class ChatSession;
	class OnlineStatus;
	class PropertyContainer;
}

typedef QMap<const Kopete::Contact*,QTimer*> TypingMap;

/**
 * @author Olivier Goffart
 */
class ChatView : public KVBox, public KopeteView
{
	Q_OBJECT
public:
	ChatView( Kopete::ChatSession *manager, ChatWindowPlugin *parent );
	~ChatView();

	/** the state of our chat */
	enum KopeteTabState { Normal, Highlighted, Changed, Typing, Message, Undefined };

	ChatMessagePart *messagePart() const { return m_messagePart; }
	ChatTextEditPart *editPart() const { return m_editPart; }

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
	 * Tells this view it is the active view
	 */
	void setActive( bool value );

	/**
	 * save the chat settings (rich text, auto spelling)
	 */
	void saveChatSettings();

	/**
	 * read the chat settings (rich text, auto spelling)
	 */
	void loadChatSettings();

	/**
	 * Clears the chat buffer
	 *
	 * Reimplemented from KopeteView
	 */
	virtual void clear();

	/**
	 * Sets the text to be displayed on tab label and window caption
	 */
	void setCaption( const QString &text, bool modified );

	/**
	 * Changes the pointer to the chat window. Used to re-parent the view
	 * @param parent The new chat window
	 */
	void setMainWindow( KopeteChatWindow* parent );

	/**
	 * Returns the message currently in the edit area
	 * @return The Kopete::Message object for the message
	 *
	 * Reimplemented from KopeteView
	 */
	virtual Kopete::Message currentMessage();

	/**
	 * Sets the current message in the chat window
	 * @param parent The new chat window
	 *
	 * Reimplemented from KopeteView
	 */
	virtual void setCurrentMessage( const Kopete::Message &newMessage );

	/**
	 * Returns the chat window this view is in
	 * @return The chat window
	 */
	KopeteChatWindow *mainWindow() const { return m_mainWindow; }

	const QString &statusText();

	QString &caption() const;

	bool sendInProgress() const;

	/** Reimplemented from KopeteView **/
	virtual void raise( bool activate=false );

	/** Reimplemented from KopeteView **/
	virtual void makeVisible();

	/** Reimplemented from KopeteView **/
	virtual bool isVisible();

	/** Reimplemented from KopeteView **/
	virtual QWidget *mainWidget();

	KTextEdit *editWidget();

	bool canSend() const;
	bool canSendFile() const;
	
	/** Reimplemented from KopeteView **/
	virtual void registerContextMenuHandler( QObject *target, const char* slot );

	/** Reimplemented from KopeteView **/
	virtual void registerTooltipHandler( QObject *target, const char* slot );

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

	void nickComplete();

	/**
	 * Reset font and color of the edit area and outgoing messages.
	 */
	void resetFontAndColor();

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
	 * Send file (opens file dialog)
	 */
	void sendFile();

	/**
	 * Called when a typing event is received from a contact
	 * Updates the typing map and outputs the typing message into the status area
	 * @param contact The contact who is / isn't typing
	 * @param typing If the contact is typing now
	 */
	void remoteTyping( const Kopete::Contact *contact, bool typing );

	/**
	 * Sets the text to be displayed on the status label
	 * @param text The text to be displayed
	 */
	void setStatusText( const QString &text );

	/**
	 * Triggers text edit's size recalculation.
	 * Used for auto-sizing.
	 */
	void slotRecalculateSize(int difference);

	/** Reimplemented from KopeteView **/
	virtual void messageSentSuccessfully();

	virtual bool closeView( bool force = false );
	
	virtual void dropEvent ( QDropEvent * );
	bool isDragEventAccepted( const QDragMoveEvent * ) const;

	/** Retrieves the tab state. */
	KopeteTabState tabState() const;

signals:
	/**
	 * Emitted when a message is sent
	 * @param message The message sent
	 */
	void messageSent( Kopete::Message & );

	void messageSuccess( ChatView* );

	/**
	 * Emits when the chat view is shown
	 */
	void shown();

	void closing( KopeteView* );

	void activated( KopeteView* );

	void captionChanged( bool active );

	void updateStatusIcon( ChatView* );

	/** Emitted when a possible tab tooltip needs updating */
	void updateChatTooltip( ChatView*, const QString& );

	/** Emitted when the state of the chat changes */
	void updateChatState( ChatView*, int );

	/** Emitted when a possible tab label needs updating */
	void updateChatLabel( ChatView*, const QString& );

	/**
	 * Our send-button-enabled flag has changed
	 */
	void canSendChanged(bool);

	void canAcceptFilesChanged();

	/**
	 * Emitted when we re-parent ourselves with a new window
	 */
	void windowCreated();

	/**
	 * Emitted when the state of RTF has changed
	 */
	void rtfEnabled( ChatView*, bool );

	void autoSpellCheckEnabled( ChatView*, bool );

private slots:
	void slotRemoteTypingTimeout();

	/**
	 * Called when a contact is added to the chat session.
	 * Adds this contact to the typingMap and the contact list view
	 * @param c The contact that joined the chat
	 * @param suppress mean that no notifications are showed
	 */
	void slotContactAdded( const Kopete::Contact *c, bool suppress );

	/**
	 * Called when a contact is removed from the chat session. Updates the tab state and status icon,
	 * displays a notification message and performs some cleanup.
	 * @param c The contact left the chat
	 * @param reason is the reason the contact left
	 * @param format The format of the reason message
	 * @param suppressNotification mean that no notifications are showed
	 */
	void slotContactRemoved( const Kopete::Contact *c, const QString& reason, Qt::TextFormat format, bool suppressNotification=false );

	/**
	 * Called when a contact changes status, updates the display name, status icon and tab bar state.
	 * If the user isn't changing to/from an Unknown status, will also display a message in the chatwindow.
	 * @param contact The contact who changed status
	 * @param status The new status of the contact
	 * @param oldstatus The former status of the contact
	 */
	void slotContactStatusChanged( Kopete::Contact *contact, const Kopete::OnlineStatus &status, const Kopete::OnlineStatus &oldstatus );

	/**
	 * Called when a contact changes status message.
	 * @param contact The contact who changed status
	 */
	void slotStatusMessageChanged( Kopete::Contact *contact );

	/**
	 * Called when the chat's display name is changed
	 */
	void slotChatDisplayNameChanged();

	void slotMarkMessageRead();

	void slotToggleRtfToolbar( bool enabled );

	/**
	 * Show that a (meta)contact change his display name.
	 */
	void slotDisplayNameChanged(const QString &oldValue, const QString &newValue);

protected:
	virtual void dragEnterEvent ( QDragEnterEvent * );
	virtual void dragMoveEvent ( QDragMoveEvent * );

private:
	// widget stuff
	KopeteChatWindow *m_mainWindow;

//	K3DockWidget *viewDock;
	ChatMessagePart *m_messagePart;

//	K3DockWidget *editDock;
	ChatTextEditPart *m_editPart;

	KopeteTabState m_tabState;

	// miscellany
	TypingMap m_remoteTypingMap;
	QString unreadMessageFrom;
	QString m_status;

	void updateChatState( KopeteTabState state = Undefined );

	/**
	 * Read in saved options, such as splitter positions
	 */
	void readOptions();

	void sendInternalMessage( const QString &msg, Qt::TextFormat format = Qt::PlainText );
	
	KopeteTabState currentState() const;

	KopeteChatViewPrivate *d;
};

/**
 * This is the class that makes the chatwindow a plugin
 */
class ChatWindowPlugin : public Kopete::ViewPlugin
{
	public:
		ChatWindowPlugin(QObject *parent, const QVariantList &args);
		KopeteView* createView( Kopete::ChatSession *manager );
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

