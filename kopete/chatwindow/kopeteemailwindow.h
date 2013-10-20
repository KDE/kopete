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

#ifndef KOPETEEMAILWINDOW_H
#define KOPETEEMAILWINDOW_H

#include "kopeteview.h"
#include "kopeteviewplugin.h"
#include <kxmlguiwindow.h>
#include <kparts/mainwindow.h>
#include <QCloseEvent>

class EmailWindowPlugin;

class KopeteEmailWindow : KParts::MainWindow, public KopeteView
{
	Q_OBJECT

public:
	enum WindowMode { Send, Read, Reply };

	KopeteEmailWindow( Kopete::ChatSession *, EmailWindowPlugin *parent, bool foreignMessage );
	~KopeteEmailWindow();

	virtual Kopete::Message currentMessage();
	virtual void setCurrentMessage( const Kopete::Message &newMessage );
	virtual void raise(bool activate=false);
	virtual void makeVisible();
	virtual bool closeView( bool force = false );
	virtual bool isVisible();
	virtual QWidget *mainWidget() { return this; }

public slots:
	virtual void sendMessage();
	virtual void appendMessage( Kopete::Message &message );
	virtual void messageSentSuccessfully();

signals:
	void shown();
	void messageSent( Kopete::Message &message );
	void closing( KopeteView *view );
	void activated( KopeteView *view );

protected:
	virtual void closeEvent( QCloseEvent *e );
	virtual void changeEvent( QEvent *e );

private slots:
	void slotReplySend();
	void slotUpdateReplySend();
	void slotReadNext();
	void slotReadPrev();
	void slotCloseView();

	void slotSmileyActivated( const QString & );
	void slotCopy();

	void slotViewMenuBar();

	void slotConfToolbar();

	void slotMarkMessageRead();

private:
	class Private;
	Private * const d;

	void toggleMode( WindowMode );
	void updateNextButton();
	void initActions();
	void writeMessage( Kopete::Message & );
};


/**
 * This is the class that makes the emailwindow a plugin
 */
class EmailWindowPlugin : public Kopete::ViewPlugin
{
    public:
	EmailWindowPlugin(QObject *parent, const QVariantList &args);
	KopeteView* createView( Kopete::ChatSession *manager );
};

#endif // __KOPETEEMAILWINDOW_H__

// vim: set noet ts=4 sts=4 sw=4:

