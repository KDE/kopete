////////////////////////////////////////////////////////////////////////////////
// gaduprefs.h                                                                //
//                                                                            //
// Copyright (C)  2002  Zack Rusin <zack@kde.org>                             //
//                                                                            //
// This library is free software; you can redistribute it and/or              //
// modify it under the terms of the GNU Lesser General Public                 //
// License as published by the Free Software Foundation; either               //
// version 2.1 of the License, or (at your option) any later version.         //
//                                                                            //
// This library is distributed in the hope that it will be useful,            //
// but WITHOUT ANY WARRANTY; without even the implied warranty of             //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU          //
// Lesser General Public License for more details.                            //
//                                                                            //
// You should have received a copy of the GNU Lesser General Public           //
// License along with this library; if not, write to the Free Software        //
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA                   //
// 02111-1307  USA                                                            //
////////////////////////////////////////////////////////////////////////////////
#ifndef GADUPREFERENCES_H
#define GADUPREFERENCES_H

#include <qwidget.h>
#include <configmodule.h>

class gaduPrefsUI;

class GaduPreferences : public ConfigModule
{
	Q_OBJECT
public:
	GaduPreferences( const QString &pixmap, QObject *parent=0 );
	~GaduPreferences();

	virtual void save();
	const QString password() { return password_; };
	const Q_UINT32 uin() { return uin_; };
signals:
	void saved();

private:
	gaduPrefsUI *prefDialog_;
	QString password_;
	Q_UINT32 uin_;
};


#endif
