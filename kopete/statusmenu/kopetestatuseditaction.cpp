/*
    kopetestatuseditaction.cpp - Kopete Status Edit Action

    Copyright (c) 2008      by Roman Jarosz          <kedgedev@centrum.cz>
    Kopete    (c) 2008      by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/
#include "kopetestatuseditaction.h"


#include <QKeyEvent>
#include <QMenu>

#include <kopetestatusmessage.h>

#include "ui_kopetestatuseditwidget_base.h"

namespace Kopete
{

namespace UI
{

class StatusEditWidget : public QWidget
{
public:
	StatusEditWidget( QWidget *parent = 0 ) : QWidget( parent )
	{
		ui.setupUi( this );

		ui.statusTitle->setClearButtonShown( true );
		ui.setButton->setGuiItem( KStandardGuiItem::Ok );
		ui.clearButton->setGuiItem( KGuiItem( i18n( "C&lear" ), "edit-clear" ) );

		setFocusPolicy( Qt::StrongFocus );
		setFocusProxy( ui.statusTitle );
	}

protected:
	// Prevents menu closing on widget click
	virtual void mouseReleaseEvent( QMouseEvent * ) {}

	virtual void keyPressEvent( QKeyEvent* event )
	{
		// Change status on enter key press
		if ( event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter )
		{
			ui.setButton->click();
			event->accept();
			return;
		}

		QWidget::keyPressEvent( event );
	}

public:
	Ui::KopeteStatusEditWidget ui;
};

StatusEditAction::StatusEditAction( QObject *parent )
: QWidgetAction( parent )
{
	mStatusEditWidget = new StatusEditWidget();
	setDefaultWidget( mStatusEditWidget );

	connect( mStatusEditWidget->ui.setButton, SIGNAL(clicked()), this, SLOT(changeClicked()) );
	connect( mStatusEditWidget->ui.clearButton, SIGNAL(clicked()), this, SLOT(clearClicked()) );
}

Kopete::StatusMessage StatusEditAction::statusMessage() const
{
	Kopete::StatusMessage statusMessage;
	statusMessage.setTitle( mStatusEditWidget->ui.statusTitle->text() );
	statusMessage.setMessage( mStatusEditWidget->ui.statusMessage->toPlainText() );
	return statusMessage;
}

void StatusEditAction::setStatusMessage( const Kopete::StatusMessage& statusMessage )
{
	mStatusEditWidget->ui.statusTitle->setText( statusMessage.title() );
	mStatusEditWidget->ui.statusMessage->setPlainText( statusMessage.message() );
}

void StatusEditAction::changeClicked()
{
	emit statusChanged( statusMessage() );
	hideMenu();
}

void StatusEditAction::clearClicked()
{
	emit statusChanged( Kopete::StatusMessage() );
	hideMenu();
}

void StatusEditAction::hideMenu()
{
	// Hack to hide menu
	QMenu* menu = qobject_cast<QMenu*>(mStatusEditWidget->parent());
	if ( menu )
	{
		menu->setActiveAction(this);
		mStatusEditWidget->parent()->event( new QKeyEvent(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier)  );
		mStatusEditWidget->parent()->event( new QKeyEvent(QEvent::KeyRelease, Qt::Key_Return, Qt::NoModifier)  );
	}
}

}

}

#include "kopetestatuseditaction.moc"
