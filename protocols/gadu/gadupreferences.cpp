//
// Current author and maintainer: Grzegorz Jaskiewicz
//				gj at pointblue.com.pl
//
// Kopete initial author:
// Copyright (C) 	2002-2003	 Zack Rusin <zack@kde.org>
//
// gaducommands.h - all basic, and not-session dependent commands
// (meaning you don't have to be logged in for any
//  of these). These delete themselves, meaning you don't
//  have to/can't delete them explicitely and have to create
//  them dynamically (via the 'new' call).
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.
//
// * 2003-06-29
//	(GJ) Due to API change, some properties are now on per account basis
//		and they will be transfered to gadueditaccount.cpp all others
//		will be implemented afterwards here

#include "gaduprefs.h"
#include "gadupreferences.h"
#include "gaducommands.h"

#include <kmessagebox.h>
#include <klineedit.h>
#include <kconfig.h>
#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>

#include <qpushbutton.h>
#include <qlayout.h>
#include <qcheckbox.h>

GaduPreferences::GaduPreferences( const QString& pixmap, QObject* parent )
	: ConfigModule( i18n("Gadu-Gadu Plugin"), i18n("Gadu Gadu"), pixmap, parent )
{
	uin_ = 0;
	( new QVBoxLayout( this ) )->setAutoAdd( true );
	prefDialog_ = new gaduPrefsUI( this );

	KGlobal::config()->setGroup("Gadu");
	prefDialog_->logAll_->setChecked( KGlobal::config()->readBoolEntry( "LogAll", false ) );
}

GaduPreferences::~GaduPreferences()
{
}

void
GaduPreferences::save()
{
	KConfig *config=KGlobal::config();


	config->setGroup("Gadu");
	config->writeEntry("LogAll", prefDialog_->logAll_->isChecked());
	config->sync();

	emit saved();
}

#include "gadupreferences.moc"
