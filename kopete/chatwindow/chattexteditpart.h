/*
    chattexteditpart.h - Chat Text Edit Part

    Copyright (c) 2004      by Richard Smith         <kde@metafoo.co.uk>

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

#ifndef CHATTEXTEDITPART_H
#define CHATTEXTEDITPART_H
#include <kparts/part.h>
#include <krichtextedit.h>
#include "kopeterichtextwidget.h"

#include <QtGui/QFont>
#include <QtGui/QColor>
#include <QtCore/QFlags>
#include <QtCore/QStringList>

// TODO: Use kdelibs export
#include <kopete_export.h>

class KAboutData;
class KTextEdit;
class KConfigGroup;
class KCompletion;
class QTimer;

namespace Sonnet {
	class Highlighter;
}

namespace Kopete
{
class Message;
class Contact;
class OnlineStatus;
class ChatSession;
class Protocol;
class PropertyContainer;
}

/**
 * @brief An instant message composition part
 * 
 * This class provides an input part suitable for the composition of instant messages.
 * It provides command history, nickname completion and typing notifications. It is
 * also able to determine whether the send button should be enabled.
 * 
 * @author Richard Smith
 */
class CHATTEXTEDITPART_EXPORT ChatTextEditPart : public KParts::ReadOnlyPart
{
	Q_OBJECT
public:
	ChatTextEditPart( Kopete::ChatSession *session, QWidget *parent);
	ChatTextEditPart(QWidget *parent, QObject*, const QStringList&);
	~ChatTextEditPart();
	
	/**
	 * @brief Get the text in the editor in the given format.
	 * By default if return the text using the most appropriate format.
	 *
	 * @param format A value in Qt::TextFormat enum.
	 *
	 * @return text using the given format
	 */
	QString text( Qt::TextFormat format = Qt::AutoText ) const;
	
	/**
	 * Enable or Disable the automatic spell checking
	 * @param enabled the state that auto spell checking should beee
	 */
	void setCheckSpellingEnabled( bool enabled );
	
	/**
	 * Get the state of auto spell checking
	 * @return true if auto spell checking is turned on, false otherwise
	 */
	bool checkSpellingEnabled() const;
	
	static KAboutData *createAboutData();
	
	/**
	 * @brief Disable file open, because it's not used by this part.
	 */
	virtual bool openFile() {
	    return false;
	}
	
	/**
	 * @brief Get the inside KTextEdit
	 * @return instance of KTextEdit
	 */
	KopeteRichTextWidget *textEdit();
	
	/**
	* @brief Is rich text is currently enabled
	*/
	bool isRichTextEnabled() const;
	
	
	/**
	 * Returns the message currently in the edit area
	 * @return The @ref Kopete::Message object for the message
	 */
	Kopete::Message contents();
	
	/**
	 * Sets the message in the edit field
	 * @param message The message to display
	 */
	void setContents( const Kopete::Message &message );

	/**
	 * Adds text into the edit area. Used when an emoticon is selected.
	 * @param text The text to be inserted
	 */
	void addText( const QString &text );
	
	/**
	 * Can we send messages now?
	 */
	bool canSend();

	/**
	 * Is the user typing right now?
	 */
	bool isTyping();
	
	void readConfig( KConfigGroup& config );
	void resetConfig( KConfigGroup& config );
	void writeConfig( KConfigGroup& config );

public slots:
	/**
	 * Go up an entry in the message history.
	 */	
	void historyUp();
	
	/**
	 * Go down an entry in the message history.
	 */
	void historyDown();
	
	/**
	 * Try to complete the word under the cursor.
	 */
	void complete();
	
	/**
	 * Sends the text currently entered into the edit area.
	 */
	void sendMessage();
	
	void checkToolbarEnabled();

signals:
	/**
	 * Emitted when a message is sent.
	 * @param message The message sent
	 */
	void messageSent( Kopete::Message &message );

	/**
	 * Emitted every 4 seconds while the user is typing.
	 * @param typing @c true if the user is typing, @c false otherwise
	 */
	void typing( bool typing );

	/**
	 * Our send-button-enabled flag might have changed
	 * @param canSend The return value of @ref canSend().
	 */
	void canSendChanged( bool canSend );
	
	void toolbarToggled(bool enabled);
	void richTextChanged();

private slots:
	/**
	 * Called when a contact is added to the chat session.
	 * Adds this contact to the nickname completion list.
	 * @param c The contact that joined the chat
	 */
	void slotContactAdded( const Kopete::Contact *c );

	/**
	 * Called when a contact is removed from the chat session.
	 * Removes this contact from the nickname completion list.
	 * @param c The contact left the chat
	 */
	void slotContactRemoved( const Kopete::Contact *c );

	/**
	 * Called when a contact changes status, may emit @ref canSendChanged.
	 * @param contact The contact who changed status
	 * @param status The new status of the contact
	 */
	void slotContactStatusChanged( Kopete::Contact *contact, const Kopete::OnlineStatus &status, const Kopete::OnlineStatus &oldstatus );

	/**
	 * Called when text is changed in the edit area
	 */
	void slotTextChanged();

	/**
	 * User is typing, so emit a @ref typing( @c true ) signal every 4 seconds.
	 * This is stupid. Why not just emit it once?
	 */
	void slotRepeatTypingTimer();
	
	/**
	 * Emits a @ref typing( @c false ) signal 4.5 seconds after the user stops typing.
	 */
	void slotStoppedTypingTimer();
	
	/**
	 * Update completion to follow changes in users' nicknames
	 */
	void slotDisplayNameChanged( const QString &oldName, const QString &newName );

	/**
	 * Some appearance settings has changed.
	 */
	void slotAppearanceChanged();

	void slotRichTextSupportChanged();

private:
	void init( Kopete::ChatSession *session, QWidget *parent);

	Kopete::ChatSession *m_session;
	
	/**
	 * The history buffer conceptually works like this:
	 * We have a list of messages (historyList), with indices from -1 to n.
	 * historyPos is our current position in this list; historyList[historyPos]
	 * is conceptually the message we are editing. The exception to this is that
	 * index -1 is treated specially; when it is modified, changes are saved to
	 * a new message placed at index 0.
	 */
	QStringList historyList;
	int historyPos;
	
	KCompletion *mComplete;
	QString m_lastMatch;
	
	QTimer *m_typingRepeatTimer;
	QTimer *m_typingStopTimer;
	
	KopeteRichTextWidget *editor;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:
