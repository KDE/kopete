/*
    chatmessagepart.h - Chat Message KPart

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

#ifndef CHATMESSAGEPART_H
#define CHATMESSAGEPART_H

#include <khtml_part.h>

#include <dom/html_element.h>

#include <qptrdict.h>
#include <qvaluelist.h>
#include <qpair.h>

#include "kopeteview.h"

class KTempFile;
class KRootPixmap;
namespace DOM { class Node; }
namespace Kopete { class ChatSession; }

/**
 * @author Richard Smith
 */
class ChatMessagePart : public KHTMLPart
{
	Q_OBJECT
public:
	ChatMessagePart( Kopete::ChatSession *manager, QWidget *parent, const char *name = 0 );
	~ChatMessagePart();

	/**
	 * Clear the message window
	 */
	void clear();

	void setStylesheet( const QString &style  );

	/**
	 * Immediately scroll the chat to the bottom, as long as it has not been intentionally scrolled away from the bottom
	 * use 
	 */
	void keepScrolledDown();

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
	 */
	void appendMessage( Kopete::Message &message );

signals:
	/**
	 * Emits before the context menu is about to show
	 */
	void contextMenuEvent( Kopete::Message &message, const QString &textUnderMouse, KPopupMenu *popupMenu );

	/**
	 * Emits before the tooltip is about to show
	 */
	void tooltipEvent( Kopete::Message &message, const QString &textUnderMouse, QString &toolTip );

private slots:
	void slotOpenURLRequest( const KURL &url, const KParts::URLArgs &args );
	void slotScrollView();
	void slotAppearanceChanged();

	/**
	 * Called when KopetePrefs are saved
	 */
	void slotTransparencyChanged();

	/**
	 * Sets the background of the widget
	 * @param pm The new background image
	 */
	void slotUpdateBackground( const QPixmap &pm );

	void slotScrollingTo( int x, int y );

	void slotRefreshNodes();

	void slotRefreshView();

	void slotTransformComplete( const QVariant &result );

	void slotRightClick( const QString &, const QPoint &point );

	void slotCopyURL();

	void slotCloseView( bool force = false );

protected:
	virtual void khtmlDrawContentsEvent( khtml::DrawContentsEvent * );
	
private:
	Kopete::ChatSession *m_manager;

	unsigned long messageId;
	typedef QMap<unsigned long,Kopete::Message> MessageMap;
	MessageMap messageMap;

	bool scrollPressed;
	bool bgChanged;

	DOM::HTMLElement activeElement;

	// FIXME: share
	KTempFile *backgroundFile;
	KRootPixmap *root;

	KAction *copyAction;
	KAction *saveAction;
	KAction *printAction;
	KAction *closeAction;
	KAction *copyURLAction;

	void readOverrides();

	const QString styleHTML() const;

	const QString addNickLinks( const QString &html ) const;

	Kopete::Contact *contactFromNode( const DOM::Node &n ) const;

	/**
	 * Emits before the tooltip is about to show
	 */
	void emitTooltipEvent( Kopete::Message &message, const QString &textUnderMouse, QString &toolTipString );

	/**
	 * Returns the text currently under the mouse
	 */
	QString textUnderMouse();

	class ToolTip;
	friend class ToolTip;

	class Private;
	Private *d;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

