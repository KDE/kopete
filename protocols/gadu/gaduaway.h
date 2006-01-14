// -*- Mode: c++-mode; c-basic-offset: 2; indent-tabs-mode: t; tab-width: 2; -*-
//
// Copyright (C) 2003 Grzegorz Jaskiewicz 	<gj at pointblue.com.pl>
// Copyright (C) 2002-2003 Zack Rusin	<zack@kde.org>
//
// gaduaway.h
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

#ifndef GADUAWAY_H
#define GADUAWAY_H

#include <kdialogbase.h>
#include <qstring.h>

class GaduAccount;
class GaduAwayUI;

class GaduAway : public KDialogBase
{
	Q_OBJECT

public:
	GaduAway( GaduAccount*, QWidget* parent = 0, const char* name = 0 );
	int status() const;
	QString awayText() const;

protected slots:
	void slotApply();

private:
	GaduAccount*	account_;
	GaduAwayUI*	ui_;
};

#endif
