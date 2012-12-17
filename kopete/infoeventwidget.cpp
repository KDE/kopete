/*
    infoeventwidget.cpp - Info Event Widget

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
#include "infoeventwidget.h"
#include "ui_infoeventbase.h"

#include <QPointer>
#include <QHash>
#include <QTimeLine>
#include <QLayout>
#include <QTextDocument>

#include <knotification.h>

#include <kopeteinfoeventmanager.h>
#include <kopeteinfoevent.h>
#include <kopetestatusmanager.h>
#include <kopeteonlinestatusmanager.h>

class InfoEventWidget::Private
{
public:
	QPointer<Kopete::InfoEvent> currentEvent;
	int currentEventIndex;
	QTimeLine *timeline;
	Ui::InfoEventBase ui;
	bool enableUpdates;
	QHash<KNotification*, QPointer<Kopete::InfoEvent> > notifyEventHash;
};

InfoEventWidget::InfoEventWidget(QWidget *parent)
: QWidget(parent), d( new Private() )
{
	d->timeline = new QTimeLine( 150, this );
	d->timeline->setCurveShape( QTimeLine::EaseInOutCurve );
	connect( d->timeline, SIGNAL(valueChanged(qreal)),
	         this, SLOT(slotAnimate(qreal)) );

	d->ui.setupUi(this);
	static_cast<KSqueezedTextLabel*>(d->ui.lblTitle)->setTextElideMode( Qt::ElideRight );
	d->ui.buttonPrev->setIcon( KIcon( "arrow-left" ) );
	d->ui.buttonNext->setIcon( KIcon( "arrow-right" ) );
	d->ui.buttonClose->setIcon( KIcon( "window-close" ) );
	d->ui.lblInfo->setTextInteractionFlags(Qt::TextSelectableByMouse);
	QWidget::setVisible( false );

	d->currentEventIndex = 0;
	d->enableUpdates = false;
	connect( Kopete::InfoEventManager::self(), SIGNAL(changed()), this, SLOT(updateInfo()) );
	connect( Kopete::InfoEventManager::self(), SIGNAL(eventAdded(Kopete::InfoEvent*)), this, SLOT(eventAdded(Kopete::InfoEvent*)) );
	connect( d->ui.lblActions, SIGNAL(linkActivated(QString)), this, SLOT(linkClicked(QString)) );
	connect( d->ui.buttonPrev, SIGNAL(clicked(bool)), this, SLOT(prevInfoEvent()) );
	connect( d->ui.buttonNext, SIGNAL(clicked(bool)), this, SLOT(nextInfoEvent()) );
	connect( d->ui.buttonClose, SIGNAL(clicked(bool)), this, SLOT(closeInfoEvent()) );
}


InfoEventWidget::~InfoEventWidget()
{
	delete d;
}

void InfoEventWidget::setVisible( bool visible )
{
	if ( visible == isVisible() )
		return;

	d->enableUpdates = visible;
	if ( visible )
		updateInfo();

	// animate the widget disappearing
	d->timeline->setDirection( visible ?  QTimeLine::Forward
	                           : QTimeLine::Backward );
	d->timeline->start();
}

void InfoEventWidget::prevInfoEvent()
{
	if ( d->currentEventIndex > 0 )
	{
		d->currentEventIndex--;
		updateInfo();
	}
}

void InfoEventWidget::nextInfoEvent()
{
	if ( d->currentEventIndex + 1 < Kopete::InfoEventManager::self()->eventCount() )
	{
		d->currentEventIndex++;
		updateInfo();
	}
}

void InfoEventWidget::closeInfoEvent()
{
	Kopete::InfoEventManager* ie = Kopete::InfoEventManager::self();
	if ( ie->eventCount() > 0 )
	{
		Kopete::InfoEvent* event = ie->event( d->currentEventIndex );

		Q_ASSERT( event );
		event->close();
	}

	if ( ie->eventCount() == 0 )
		setVisible( false );
}

void InfoEventWidget::slotAnimate( qreal amount )
{
	if ( amount == 0 )
	{
		QWidget::setVisible( false );
		return;
	}
	
	if ( amount == 1.0 )
	{
		layout()->setSizeConstraint( QLayout::SetDefaultConstraint );
		setFixedHeight( sizeHintHeight() );
		return;
	}
	
	setFixedHeight( sizeHintHeight() * amount );
	
	if ( !isVisible() )
		QWidget::setVisible( true );
}

void InfoEventWidget::linkClicked( const QString& link )
{
	Kopete::InfoEvent* event = Kopete::InfoEventManager::self()->event( d->currentEventIndex );
	Q_ASSERT( event );

	event->activate( link.toInt() );
}

void InfoEventWidget::updateInfo()
{
	if ( !d->enableUpdates )
		return;

	Kopete::InfoEventManager* ie = Kopete::InfoEventManager::self();

	if ( ie->eventCount() == 0 )
	{
		d->currentEventIndex = 0;
		d->ui.lblInfo->clear();
		d->ui.lblActions->clear();
		// Can't use clear
		static_cast<KSqueezedTextLabel*>(d->ui.lblTitle)->setText(QString());
		d->ui.lblEvent->setText( "0/0" );
		d->ui.buttonPrev->setEnabled( false );
		d->ui.buttonNext->setEnabled( false );
		return;
	}

	if ( d->currentEventIndex >= ie->eventCount() )
		d->currentEventIndex = ie->eventCount() - 1;

	d->ui.buttonPrev->setEnabled( (d->currentEventIndex > 0) );
	d->ui.buttonNext->setEnabled( (d->currentEventIndex + 1 < ie->eventCount()) );

	Kopete::InfoEvent* event = ie->event( d->currentEventIndex );
	Q_ASSERT( event );

	if ( d->currentEvent != event )
	{
		if ( !d->currentEvent )
			disconnect( d->currentEvent, SIGNAL(changed()), this , SLOT(updateInfo()) );

		d->currentEvent = event;
		connect( d->currentEvent, SIGNAL(changed()), this , SLOT(updateInfo()) );
	}

	static_cast<KSqueezedTextLabel*>(d->ui.lblTitle)->setText( Qt::escape( event->title() ) );
	d->ui.lblEvent->setText( QString("%1/%2").arg( d->currentEventIndex + 1 ).arg( ie->eventCount() ) );

	d->ui.lblInfo->setVisible( !event->text().isEmpty() );
	if ( !event->text().isEmpty() )
	{
		QString text = QString( "<p>%1</p>" ).arg( event->text() );
		if ( !event->additionalText().isEmpty() )
			text += QString( "<p>%1</p>" ).arg( event->additionalText() );

		d->ui.lblInfo->setText( text );
	}
	else
	{
		d->ui.lblInfo->clear();
	}

	d->ui.lblActions->setVisible( !event->actions().isEmpty() );
	if ( !event->actions().isEmpty() )
	{
		QString linkCode = QString::fromLatin1( "<p align=\"right\">" );

		QMap<uint, QString> actions = event->actions();
		QMapIterator<uint, QString> it(actions);
		while ( it.hasNext() )
		{
			it.next();
			linkCode += QString::fromLatin1( "<a href=\"%1\">%2</a> " ).arg( it.key() ).arg( Qt::escape(it.value()) );
		}

		d->ui.lblActions->setText( linkCode );
	}

	// Redo the layout otherwise sizeHint() won't be correct.
	layout()->activate();
	if ( sizeHintHeight() > height() )
		setFixedHeight( sizeHintHeight() );
}

void InfoEventWidget::eventAdded( Kopete::InfoEvent* event )
{
	if ( Kopete::StatusManager::self()->globalStatusCategory() != Kopete::OnlineStatusManager::Busy )
	{
		KNotification *notify = new KNotification( QString("kopete_info_event") , 0l );
		notify->setActions( QStringList( i18n( "View" ) ) );
		notify->setText( event->text() );

		d->notifyEventHash.insert( notify, event );

		connect( notify, SIGNAL(activated(uint)), this, SLOT(notificationActivated()) );
		connect( notify, SIGNAL(closed()), this, SLOT(notificationClosed()) );

		notify->sendEvent();
	}

	if ( event->showOnSend() )
	{
		int index = Kopete::InfoEventManager::self()->events().indexOf( event );
		if ( index != -1 )
		{
			d->currentEventIndex = index;
			updateInfo();
		}
		emit showRequest();
	}
}

void InfoEventWidget::notificationActivated()
{
	KNotification *notify = dynamic_cast<KNotification *>(sender());

	Kopete::InfoEvent* event = d->notifyEventHash.value( notify, 0 );
	if ( !event )
		return;

	int index = Kopete::InfoEventManager::self()->events().indexOf( event );
	if ( index != -1 )
	{
		d->currentEventIndex = index;
		updateInfo();
	}
	emit showRequest();
}

void InfoEventWidget::notificationClosed()
{
	KNotification *notify = dynamic_cast<KNotification *>(sender());
	d->notifyEventHash.remove( notify );
}

int InfoEventWidget::sizeHintHeight() const
{
	return qMin( sizeHint().height(), 250 );
}

#include "infoeventwidget.moc"
