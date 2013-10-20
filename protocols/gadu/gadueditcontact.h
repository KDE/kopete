// -*- Mode: c++-mode; c-basic-offset: 2; indent-tabs-mode: t; tab-width: 2; -*-
//
// Copyright (C) 2003 Grzegorz Jaskiewicz 	<gj at pointblue.com.pl>
// Copyright (C) 2002-2003 Zack Rusin	<zack@kde.org>
//
// gadueditcontact.h
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

#ifndef GADUEDITCONTACT_H
#define GADUEDITCONTACT_H

#include "gaducontactlist.h"

#include <kdialog.h>
#include <QLabel>

class GaduAccount;
namespace Ui { class GaduAddUI; }
class QLabel;
class QWidget;
class GaduContact;
class Q3ListViewItem;

class GaduEditContact : public KDialog
{
	Q_OBJECT

public:
	GaduEditContact( GaduAccount*, GaduContact*,
		    QWidget* parent = 0 );
	GaduEditContact( GaduAccount*,  GaduContactsList::ContactLine*,
		    QWidget* parent = 0 );
	~GaduEditContact();
protected slots:
	void slotApply();
	void listClicked( Q3ListViewItem* );
private:

	void init();
	void fillIn();
	void fillGroups();
	GaduAccount*	account_;
	GaduContact*	contact_;
	Ui::GaduAddUI*	ui_;
	GaduContactsList::ContactLine* cl_;
};

#endif
