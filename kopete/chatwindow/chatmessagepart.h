/*
    chatmessagepart.h - Chat Message KPart

    Copyright (c) 2004      by Richard Smith         <kde@metafoo.co.uk>
    Copyright (c) 2005      by Michaël Larouche      <larouche@kde.org>
    Copyright (c) 2009      by Fabian Rami      <fabian.rami@wowcompany.com>

    Kopete    (c) 2002-2008 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef CHATMESSAGEPART_H
#define CHATMESSAGEPART_H

#include <khtml_part.h>
#include <dom/html_element.h>
#include <dom/dom2_events.h>

#include <kmenu.h>
#include <kopetemessage.h>

#include <kopete_export.h>

namespace Kopete
{
	class Message;
	class ChatSession;
	class Contact;
}
class KMenu;
class ChatWindowStyle;

/**
 * @author Richard Smith
 */
class KOPETECHATWINDOW_SHARED_EXPORT ChatMessagePart : public KHTMLPart
{
	Q_OBJECT
public:
	/**
	 * Create a new ChatMessage Part.
	 */
	ChatMessagePart( Kopete::ChatSession *manager, QWidget *parent);
	~ChatMessagePart();

	/**
	 * Clear the message window
	 */
	void clear();

	/**
	 * Immediately scroll the chat to the bottom, as long as it has not been intentionally scrolled away from the bottom
	 * use
	 */
	void keepScrolledDown();

	/**
	 * Format contact's nickname/displayname according to preferences.
	 *
	 * @param contact Contact to format.
	 * @param format text format, only PlainText and RichText are supported
	 * @return the formatted name.
	 */
	QString formatName( const Kopete::Contact* contact, Qt::TextFormat format ) const;

	/**
	 * Format a nickname/displayname according to preferences.
	 *
	 * @param sourceName Source name to format.
	 * @param format text format, only PlainText and RichText are supported
	 * @return the formatted name.
	 */
	QString formatName( const QString &sourceName, Qt::TextFormat format ) const;

public slots:
	/**
	 * Initiates a copy action
	 * If there is text selected in the HTML view, that text is copied
	 * Otherwise if @p justselection is false, the entire edit area is copied.
	 *
	 * @param justselection If this is true, then the text will be only copied to the selection buffer only.
	 *                      In this mode, if nothing is selected, then nothing is copied.
	 */
	void copy(bool justselection = false);

	/**
	 * Print out the contents of the chatwindow
	 */
	void print();

	/**
	 * Save the contents of the chat to a file
	 */
	void save();

	/**
	 * Scroll the view up a page
	 */
	void pageUp();

	/**
	 * Scroll the view down a page
	 */
	void pageDown();

	/**
	 * Appends a message to the messave view
	 * @param message The message to be appended
	 * @param restoring This flag is used to not re-append message when changing style. By default false.
	 */
	void appendMessage( Kopete::Message &message, bool restoring = false);

	/**
	 * Change the current style.
	 * This method override is used when preferences change.
	 * This method create a new ChatWindowStyle object.
	 *
	 * Need to rebuild all the XHTML content.
	 *
	 * @param styleName name of the style.
	 */
	void setStyle( const QString &styleName );

	/**
	 * Change the current style
	 * This method override is used on preview and unit tests.
	 * Use a already existing ChatWindowStyle object.
	 *
	 * Need to rebuild all the XHTML content.
	 *
	 * @param chatWindowStyle ChatWindowStyle object.
	 */
	void setStyle( ChatWindowStyle *style );

	/**
	 * Change the current variant for the current style
	 * @param variantPath relative path to the style variant.
	 */
	void setStyleVariant( const QString &variantPath );

	void messageStateChanged( uint messageId, Kopete::Message::MessageState state );

	/**
	 * @deprecated
	 * does nothing
	 */
	KDE_DEPRECATED void slotToggleGraphicOverride(bool enable);

signals:
	/**
	 * Emits before the context menu is about to show
	 */
	void contextMenuEvent(  const QString &textUnderMouse, KMenu *popupMenu );

	/**
	 * Emits before the tooltip is about to show
	 */
	void tooltipEvent(  const QString &textUnderMouse, QString &toolTip );

private slots:
	void slotOpenURLRequest( const KUrl &url, const KParts::OpenUrlArguments &, const KParts::BrowserArguments & );
	void slotFileTransferIncomingDone( unsigned int id );
	void slotScrollView();
	void slotRenderingFinished();
	void slotAppearanceChanged();

	void slotScrollingTo( int y );

	void slotRefreshView();

	void slotRightClick( const QString &, const QPoint &point );

	void slotCopyURL();

	void slotCloseView( bool force = false );

	/**
	 * Do the actual style change.
	 */
	void changeStyle();

	/**
	 * Clears style when style is deleted.
	 */
	void clearStyle();

	/**
	 * Update the display in the header template if any.
	 */
	void slotUpdateHeaderDisplayName();
	/**
	 * Upda the photo in the header.
	 */
	void slotUpdateHeaderPhoto();
	
	void resendMessage( uint messageId );

	void playVoiceClip( uint messageId );

	void saveVoiceClip( uint messageId );

protected:
	virtual void khtmlDrawContentsEvent( khtml::DrawContentsEvent * );

private:
	void readOverrides();

	const QString styleHTML() const;

	Kopete::Contact *contactFromNode( const DOM::Node &n ) const;

	/**
	 * Emits before the tooltip is about to show
	 */
	void emitTooltipEvent(  const QString &textUnderMouse, QString &toolTipString );

	/**
	 * Returns the text currently under the mouse
	 */
	QString textUnderMouse();

//	class ToolTip;
//	friend class ToolTip;

	/**
	 * Format(replace) style keywords for messages (incoming, outgoing, internal)
	 * Use formatStyleKeywords(const QString &sourceHTML) for header and footer.
	 *
	 * @param sourceHTML the source html which contains the keywords
	 * @param message the current Message.
	 *
	 * @return the resulting HTML with replaced keywords.
	 */
	QString formatStyleKeywords( const QString &sourceHTML, const Kopete::Message &message );
	/**
	 * Format(replace) style keywords for header and footers.
	 * For messages, use formatStyleKeywords(const QString &sourceHTML, Kopete::Message &message)  instead.
	 *
	 * @param sourceHTML HTML source needed to be replaced.
	 *
	 * @return the resulting HTML with replaced keywords.
	 */
	QString formatStyleKeywords( const QString &sourceHTML );

	/**
	 * Helper function to parse time in correct format.
	 * Use glibc strftime function.
	 *
	 * @param timeFormat the time format to parse.
	 * @param dateTime the QDateTime which contains the datetime to format.
	 * @return the formatted time string.
	 */
	QString formatTime(const QString &timeFormat, const QDateTime &dateTime);

	/**
	 * Format a message body according to the style included
	 * in the message.
	 *
	 * @param message Kopete::Message to format.
	 * @return a span tag with a style attribute.
	 */
	QString formatMessageBody( const Kopete::Message &message );

	/**
	 * Write the template file to KHTMLPart
	 */
	void writeTemplate();

	/**
	 * Adjust style variant to chat session type.  This allows the chat window to show a compact
	 * variant of the style for chatrooms and a full-sized version for 1:small chats
	 * @param styleVariant the chosen style variant as a relative path
	 * @return the munged path, eg variant_compact.css
	 */
	QString adjustStyleVariantForChatSession( const QString & styleVariant ) const;

	/**
	 * Find photo for given contact.
	 * @return the photo path or base64 photo data
	 */
	QString photoForContact( const Kopete::Contact *contact ) const;

	void addFileTransferButtonsEventListener( unsigned int id );

	void disableFileTransferButtons( unsigned int id );

	void addVoiceClipsButtonsEventListener( unsigned int id );

	void changeMessageStateElement( uint id, Kopete::Message::MessageState state );

	void registerClickEventListener( DOM::HTMLElement element );

	void readChatFont();

	class Private;
	Private * const d;
};

class HTMLEventListener: public QObject, public DOM::EventListener
{
	Q_OBJECT
public:
	virtual void handleEvent( DOM::Event &event );
Q_SIGNALS:
	void resendMessage( uint messageId );
	void playVoiceClip( uint messageId );
	void saveVoiceClip( uint messageId );
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

