/*************************************************************************
 *   Copyright (C) 2007 by Michael Zanetti 
 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

/**
  * @author Michael Zanetti
  */

#include "privkeypopup.h"

PrivKeyPopup::PrivKeyPopup( QWidget *parent ):KDialog( parent ){
	QWidget *widget = new QWidget( this, Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint );
	ui.setupUi( widget );
	setMainWidget( widget );
	setCaption( i18n( "Please wait" ) );
	setButtons( KDialog::None );

	KIcon *icon = new KIcon( "dialog-password" );
	ui.lIcon->setPixmap( icon->pixmap( 48, 48 ) );
}

PrivKeyPopup::~PrivKeyPopup(){
}


void PrivKeyPopup::setCloseLock( bool locked ){
	closeLock = locked;
}

void PrivKeyPopup::closeEvent( QCloseEvent *e ){
	if( closeLock ){
		e->ignore();
	} else {
		e->accept();
	}
}

#include "privkeypopup.moc"