// -*- Mode: c++-mode; c-basic-offset: 2; indent-tabs-mode: t; tab-width: 2; -*-
//
// Copyright (C) 2003 Grzegorz Jaskiewicz <gj at pointblue.com.pl>
// Copyright (C) 2002-2003 Zack Rusin <zack@kde.org>
//
// gaduaddconectpage.h
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

#ifndef GADUADDCONTACTPAGE_H
#define GADUADDCONTACTPAGE_H

#include "addcontactpage.h"
#include <qwidget.h>

class GaduAccount;
class gaduAddUI;
class QLabel;

class GaduAddContactPage : public AddContactPage
{
	Q_OBJECT

public:
	GaduAddContactPage( GaduAccount*, QWidget* parent = 0, const char* name = 0 );
	~GaduAddContactPage();
	virtual bool validateData();
	virtual bool apply( KopeteAccount* , KopeteMetaContact * );

private:
	GaduAccount*	account_;
	gaduAddUI*	addUI_;
	QLabel*		noaddMsg1_;
	QLabel*		noaddMsg2_;
	bool			canAdd_;
};

#endif
