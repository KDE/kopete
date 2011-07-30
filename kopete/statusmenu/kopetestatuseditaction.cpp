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

StatusEditWidget::StatusEditWidget( QWidget *parent )
: QWidget( parent )
, ui( new Ui::KopeteStatusEditWidget )
{
	ui->setupUi( this );

	ui->statusTitle->setClearButtonShown( true );
	ui->buttonBox->addButton( KGuiItem( i18n( "C&lear" ), "edit-clear" ), QDialogButtonBox::DestructiveRole, this, SLOT(clearClicked()) );

	setFocusPolicy( Qt::StrongFocus );
	setFocusProxy( ui->statusTitle );

	connect( ui->buttonBox, SIGNAL(accepted()), this, SLOT(changeClicked()) );
}

StatusEditWidget::~StatusEditWidget()
{
	delete ui;
}

KDialogButtonBox *StatusEditWidget::buttonBox() const
{
	return ui->buttonBox;
}

Kopete::StatusMessage StatusEditWidget::statusMessage() const
{
	Kopete::StatusMessage statusMessage;
	statusMessage.setTitle( ui->statusTitle->text() );
	statusMessage.setMessage( ui->statusMessage->toPlainText() );
	return statusMessage;
}

void StatusEditWidget::setStatusMessage( const Kopete::StatusMessage& statusMessage )
{
	ui->statusTitle->setText( statusMessage.title() );
	ui->statusMessage->setPlainText( statusMessage.message() );
}

void StatusEditWidget::changeClicked()
{
	emit statusChanged( statusMessage() );
}

void StatusEditWidget::clearClicked()
{
	setStatusMessage( Kopete::StatusMessage() );
	emit statusChanged( statusMessage() );
}

// FIXME: This should probably be in the action, implemented as an event-filter
// Prevents menu closing on widget click
void StatusEditWidget::mouseReleaseEvent( QMouseEvent * )
{}

void StatusEditWidget::keyPressEvent( QKeyEvent* event )
{
	// Change status on enter key press
	if ( event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter )
	{
		changeClicked();
		event->accept();
		return;
	}

	QWidget::keyPressEvent( event );
}

StatusEditAction::StatusEditAction( QObject *parent )
: QWidgetAction( parent )
{
	mStatusEditWidget = new StatusEditWidget();
	setDefaultWidget( mStatusEditWidget );

	connect(mStatusEditWidget, SIGNAL(statusChanged(Kopete::StatusMessage)), SLOT(hideMenu()) );
	connect(mStatusEditWidget, SIGNAL(statusChanged(Kopete::StatusMessage)), SIGNAL(statusChanged(Kopete::StatusMessage)) );
}

Kopete::StatusMessage StatusEditAction::statusMessage() const
{
	return mStatusEditWidget->statusMessage();
}

void StatusEditAction::setStatusMessage( const Kopete::StatusMessage& statusMessage )
{
	mStatusEditWidget->setStatusMessage( statusMessage );
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

StatusEditDialog::StatusEditDialog( QWidget *parent )
: KDialog(parent)
, mStatusEditWidget( new StatusEditWidget )
{
	setMainWidget( mStatusEditWidget );
	setCaption( i18n("Edit Message") );

	// We use the buttonbox from the edit widget
	setButtons(KDialog::None);
	KDialogButtonBox *buttonBox = mStatusEditWidget->buttonBox();
	buttonBox->setStandardButtons( buttonBox->standardButtons() | QDialogButtonBox::Cancel );
	connect(buttonBox, SIGNAL(rejected()), SLOT(reject()) );

	connect(mStatusEditWidget, SIGNAL(statusChanged(Kopete::StatusMessage)), SLOT(accept()) );
}

Kopete::StatusMessage StatusEditDialog::statusMessage() const
{
	return mStatusEditWidget->statusMessage();
}

void StatusEditDialog::setStatusMessage( const Kopete::StatusMessage& statusMessage )
{
	mStatusEditWidget->setStatusMessage( statusMessage );
}


}

}

#include "kopetestatuseditaction.moc"
