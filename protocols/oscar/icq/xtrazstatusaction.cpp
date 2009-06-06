/*
    xtrazstatusaction.cpp  -  Xtraz Status Action

    Copyright (c) 2007 by Roman Jarosz <kedgedev@centrum.cz>
    Kopete    (c) 2007 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "xtrazstatusaction.h"

namespace Xtraz
{

StatusAction::StatusAction( const Xtraz::Status &status, QObject *parent )
: KAction( parent ), mStatus( status )
{
	this->setText( mStatus.description() );

	this->setIcon( KIcon( QString( "icq_xstatus%1" ).arg( mStatus.status() ) ) );
	this->setToolTip( mStatus.message() );
	
	QObject::connect( this, SIGNAL(triggered(bool)), this, SLOT(triggered()) );
}

void StatusAction::triggered()
{
	emit triggered( mStatus );
}

}

#include "xtrazstatusaction.moc"
