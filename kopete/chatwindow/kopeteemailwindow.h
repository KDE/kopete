/*
    kopeteemailwindow.h - Kopete "email window" for single-shot messages

    Copyright (c) 2002      by Daniel Stone          <dstone@kde.org>
    Copyright (c) 2003      by Jason Keirstead       <jason@keirstead.org>
    Copyright (c) 2003      by Martijn Klingens      <klingens@kde.org>

    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef __KOPETEEMAILWINDOW_H__
#define __KOPETEEMAILWINDOW_H__

#include <kmainwindow.h>
#include <kparts/mainwindow.h>

#include "kopeteview.h"

class KURL;

namespace KParts { struct URLArgs; }

class KopeteEmailWindowPrivate;

class KopeteEmailWindow : KParts::MainWindow, public KopeteView
{
	Q_OBJECT

public:
	enum WindowMode { Send, Read, Reply };

	/**
	 * Constructs a KopeteEmailWindowBase as a child of 'parent', with the
	 * name 'name' and widget flags set to 'f'.
	 */
	KopeteEmailWindow( KopeteMessageManager *, bool foreignMessage );

	/**
	 * Destroys the object and frees any allocated resources
	 */
	~KopeteEmailWindow();

	virtual void setCurrentMessage( const KopeteMessage &newMessage );

	virtual void raise(bool activate=false);

	virtual void makeVisible();

	virtual bool closeView( bool force = false );

	virtual bool isVisible();

	virtual KopeteMessage currentMessage();

	virtual QTextEdit *editWidget();
	virtual QWidget *mainWidget() { return this; }

public slots:
	virtual void sendMessage();
	virtual void appendMessage( KopeteMessage &message );
	virtual void messageSentSuccessfully();

signals:
	virtual void shown();
	virtual void messageSent( KopeteMessage &message );
	virtual void closing( KopeteView *view );
	virtual void activated( KopeteView *view );

protected:
	virtual bool queryExit();
	virtual void windowActivationChange( bool activated );
	virtual bool KopeteEmailWindow::eventFilter( QObject *o, QEvent *e );

private slots:
	void slotReplySendClicked();
	void slotReadNext();
	void slotReadPrev();
	void slotOpenURLRequest( const KURL &url, const KParts::URLArgs &args );
	void slotTextChanged();
	void slotCloseView();

	void slotSmileyActivated( const QString & );
	void slotCopy();
	void slotSetBgColor( const QColor & = QColor() );
	void slotSetFgColor( const QColor & = QColor() );
	void slotSetFont( const QFont & );
	void slotSetFont();

	void slotViewMenuBar();
	void slotViewToolBar();

	void slotConfKeys();
	void slotConfToolbar();

	void slotMarkMessageRead();
	void slotRefreshAppearance();

private:
	KopeteEmailWindowPrivate *d;

	void toggleMode( WindowMode );
	void updateNextButton();
	void initActions();
	void writeMessage( KopeteMessage & );
};

#endif // __KOPETEEMAILWINDOW_H__

// vim: set noet ts=4 sts=4 sw=4:

