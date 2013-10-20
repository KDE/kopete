// -*- Mode: c++-mode; c-basic-offset: 2; indent-tabs-mode: t; tab-width: 2; -*-
//
// Copyright (C) 2003 Grzegorz Jaskiewicz 	<gj at pointblue.com.pl>
//
// gaduregisteraccount.h
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
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
// 02110-1301, USA.

#ifndef GADUREGISTERACCOUNT_H
#define GADUREGISTERACCOUNT_H

#include <kdialog.h>
#include <QPixmap>

class QString;
class QPixmap;
class RegisterCommand;
class QRegExp;
namespace Ui { class GaduRegisterAccountUI; }

class GaduRegisterAccount : public KDialog
{
    Q_OBJECT

public:
	GaduRegisterAccount( QWidget* = 0 );
	~GaduRegisterAccount( );

signals:
	void registeredNumber( unsigned int, QString  );

protected slots:
	void slotClose();
	void displayToken( QPixmap, QString );
	void registrationError(  const QString&, const QString& );
	void registrationDone(  const QString&,  const QString& );
	void inputChanged( const QString & );
	void doRegister();
	void updateStatus( const QString status );

private:
	void validateInput();

	Ui::GaduRegisterAccountUI*	ui;
	RegisterCommand*	cRegister;
	QRegExp*		emailRegexp; 
	QPixmap			hintPixmap;
};

#endif
