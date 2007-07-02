/*
    identitystatuswidget.cpp  -  Kopete identity status configuration widget

    Copyright (c) 2007      by Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>

    Kopete    (c) 2003-2007 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/


#include "identitystatuswidget.h"
#include "ui_identitystatusbase.h"

#include <KIcon>
#include <QTimeLine>
#include <kopeteidentity.h>
#include <kopeteaccount.h>
#include <kopetecontact.h>
#include <kopeteprotocol.h>
#include <KDebug>

class IdentityStatusWidget::Private
{
public:
	Kopete::Identity *identity;
	Ui::IdentityStatusBase ui;
	QTimeLine *timeline;
	QString photoPath;
};

IdentityStatusWidget::IdentityStatusWidget(Kopete::Identity *identity, QWidget *parent)
: QWidget(parent)
{
	d = new Private();
	d->identity = identity;
	
	// animation for showing/hiding
	d->timeline = new QTimeLine( 150, this );
	d->timeline->setCurveShape( QTimeLine::EaseInOutCurve );
	connect( d->timeline, SIGNAL(valueChanged(qreal)),
			 this, SLOT(slotAnimate(qreal)) );

	d->ui.setupUi(this);
	QWidget::setVisible( false );

	slotLoad();

	// user input signals
	connect( d->ui.nickName, SIGNAL(editingFinished()), this, SLOT(slotSave()) );
}

IdentityStatusWidget::~IdentityStatusWidget()
{
	delete d->timeline;
	delete d;
}

void IdentityStatusWidget::setIdentity(Kopete::Identity *identity)
{
	if (d->identity)
		slotSave();

	d->identity = identity;
	slotLoad();
}

Kopete::Identity *IdentityStatusWidget::identity() const
{
	return d->identity;
}

void IdentityStatusWidget::setVisible( bool visible )
{
	// animate the widget disappearing
	d->timeline->setDirection( visible ?  QTimeLine::Forward
										: QTimeLine::Backward );
	d->timeline->start();
}

void IdentityStatusWidget::slotAnimate(qreal amount)
{
	if (amount == 0)
	{
		QWidget::setVisible( false );
		return;
	}
	
	if (amount == 1)
	{
		setFixedHeight( sizeHint().height() );
		d->ui.nickName->setFocus();
		return;
	}

	if (!isVisible())
		QWidget::setVisible( true );

	setFixedHeight( sizeHint().height() * amount );
}

void IdentityStatusWidget::slotLoad()
{
	// clear
	d->ui.photo->setText(i18n("No Photo"));
	d->ui.nickName->clear();
	d->ui.identityStatus->clear();
	d->ui.identityName->clear();
	d->ui.accounts->clear();

	if (!d->identity)
		return;

	Kopete::Global::Properties *props = Kopete::Global::Properties::self();
	
	// photo
	if (d->identity->hasProperty(props->photo().key()))
	{
		d->photoPath = d->identity->getProperty(props->photo()).value().toString();
		d->ui.photo->setPixmap( QPixmap(d->photoPath) );
	}

	// nickname
	if (d->identity->hasProperty(props->nickName().key()))
		d->ui.nickName->setText( d->identity->getProperty(props->nickName()).value().toString() );

	d->ui.identityName->setText(d->identity->identityId());

	//acounts
	QString text("<qt>");
	foreach(Kopete::Account *a, d->identity->accounts())
	{
		Kopete::Contact *self = a->myself();
		QString onlineStatus = self ? self->onlineStatus().description() : i18n("Offline");
		text += i18nc( "Account tooltip information: <nobr>ICON <b>PROTOCOL:</b> NAME (<i>STATUS</i>)<br/>",
					 "<nobr><a href=\"accountmenu:%2:%3\"><img src=\"kopete-account-icon:%2:%3\"> %1 (<i>%4</i>)</a><br/>",
                     a->accountLabel(), QString(QUrl::toPercentEncoding( a->protocol()->pluginId() )),
                     QString(QUrl::toPercentEncoding( a->accountId() )), onlineStatus );
    }
    text += QLatin1String("</qt>");
	d->ui.accounts->setText( text );

	//TODO: online status
	
}

void IdentityStatusWidget::slotSave()
{
	if (!d->identity)
		return;

	Kopete::Global::Properties *props = Kopete::Global::Properties::self();

	// photo
	if (!d->identity->hasProperty(props->photo().key()) ||
		d->identity->getProperty(props->photo()).value().toString() != d->photoPath)
	{
		d->identity->setProperty(props->photo(), d->photoPath);
	}

	// nickname
	if (!d->identity->hasProperty(props->nickName().key()) ||
		d->identity->getProperty(props->photo()).value().toString() != d->ui.nickName->text())
	{
		d->identity->setProperty(props->nickName(), d->ui.nickName->text());
	}

	//TODO check what more to do

}

#include "identitystatuswidget.moc"
// vim: set noet ts=4 sts=4 sw=4:
