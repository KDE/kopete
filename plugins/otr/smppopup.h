/*************************************************************************
 * Copyright <2007>  <Michael Zanetti> <michael_zanetti@gmx.net>         *
 *                                                                       *
 * This program is free software; you can redistribute it and/or         *
 * modify it under the terms of the GNU General Public License as        *
 * published by the Free Software Foundation; either version 2 of        *
 * the License or (at your option) version 3 or any later version        *
 * accepted by the membership of KDE e.V. (or its successor approved     *
 * by the membership of KDE e.V.), which shall act as a proxy            *
 * defined in Section 14 of version 3 of the license.                    *
 *                                                                       *
 * This program is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 * GNU General Public License for more details.                          *
 *                                                                       *
 * You should have received a copy of the GNU General Public License     *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *************************************************************************/ 

#ifndef SMPPOPUP_H
#define SMPPOPUP_H

/**
  * @author Michael Zanetti
  */

extern "C"{
#include "libotr/proto.h"
}

#include "kopetechatsession.h"

#include "otrlchatinterface.h"
#include "ui_smppopup.h"

class SMPPopup: public KDialog
{
	Q_OBJECT
public:
	explicit SMPPopup(QWidget *parent = 0, ConnContext *context = 0, Kopete::ChatSession *session = 0, bool initiate = true );
	~SMPPopup();

protected:
	virtual void closeEvent( QCloseEvent *event );

private slots:
	void	cancelSMP();
	void	respondSMP();
	void	manualAuth();

private:
	Ui::SMPPopup ui;
	ConnContext *context;
	Kopete::ChatSession *session;
	bool initiate;

};

#endif //SMPPOPUP_H
