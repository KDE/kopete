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
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
// 02110-1301, USA.

#ifndef GADUEDITACCOUNT_H
#define GADUEDITACCOUNT_H

#include "ui_gadueditaccountui.h"
#include "editaccountwidget.h"
#include "gaduregisteraccount.h"
#include "gadusession.h"

class GaduAccount;
class GaduProtocol;

namespace Kopete { class Account; }

class GaduEditAccount : public QWidget, private Ui::GaduAccountEditUI, public KopeteEditAccountWidget
{
    Q_OBJECT

public:
	GaduEditAccount( GaduProtocol*, Kopete::Account*, QWidget* parent = 0 );
	virtual bool validateData();
	Kopete::Account* apply();

private slots:
	void registerNewAccount();
	void newUin( unsigned int, QString  );
	void registrationFailed();
	void slotSearchResult( const SearchResult&, unsigned int );

private:
	void enableUserInfo( bool );
	void publishUserInfo();

	GaduProtocol*		protocol_;
	bool			reg_in_progress;
	bool			isSsl;
	RegisterCommand*	rcmd;
	GaduRegisterAccount*	regDialog;
	GaduAccount*		account_;
	unsigned int 		seqNr;
};

#endif
