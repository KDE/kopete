// -*- Mode: c++-mode; c-basic-offset: 2; indent-tabs-mode: t; tab-width: 2; -*-
//
// Copyright (C) 2003 Grzegorz Jaskiewicz 	<gj at pointblue.com.pl>
//
// gadueditaccount.h
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

#ifndef GADUEDITACCOUNT_H
#define GADUEDITACCOUNT_H

#include "gadueditaccountui.h"
#include "editaccountwidget.h"
#include "gaduregisteraccount.h"

class GaduAccount;
class GaduProtocol;
class KopeteAccount;

class GaduEditAccount : public GaduAccountEditUI, public KopeteEditAccountWidget
{
    Q_OBJECT

public:
	GaduEditAccount( GaduProtocol*, KopeteAccount*, QWidget* parent = 0, const char* name = 0 );
	virtual bool validateData();
	KopeteAccount* apply();

private slots:
	void registerNewAccount();
	void newUin( unsigned int, QString  );
	void registrationFailed();

private:
	GaduProtocol*		protocol_;
	bool				reg_in_progress;
	bool				isSsl;
	RegisterCommand*	rcmd;
	GaduRegisterAccount* regDialog;
};

#endif
