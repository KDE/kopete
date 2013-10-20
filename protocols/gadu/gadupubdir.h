//
// Copyright (C) 2003	 Grzegorz Jaskiewicz <gj at pointblue.com.pl>
//
// gadupubdir.h
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
//

#ifndef GADUPUBDIR_H
#define GADUPUBDIR_H

#include "ui_gadusearch.h"
#include "gadusession.h"

#include <kdebug.h>
#include <kdialog.h>
#include <QPixmap>

class GaduAccount;
class GaduProtocol;
class GaduContact;
class GaduAccount;
namespace Ui { class GaduPublicDirectory; }
class GaduContact;

class GaduPublicDir : public KDialog
{
Q_OBJECT

public:
	explicit GaduPublicDir( GaduAccount* , QWidget *parent = 0 );
	GaduPublicDir( GaduAccount* , int searchFor, QWidget* parent = 0 );
	~GaduPublicDir();
	QPixmap iconForStatus( uint status );

private slots:
	void slotSearch();
	void slotNewSearch();
	void slotSearchResult( const SearchResult& result, unsigned int seq );
	void slotAddContact();
	void inputChanged( const QString& );
	void inputChanged( bool );
	void slotListSelected();


private:
	void getData();
	bool validateData();
	void createWidget();
	void initConnections();

	GaduProtocol*		p;
	GaduAccount*		mAccount;
	GaduContact*		mContact;
	Ui::GaduPublicDirectory*	mMainWidget;

// form data
	QString	fName;
	QString	fSurname;
	QString	fNick;
	QString	fCity;
	int		fUin;
	int		fGender;
	bool		fOnlyOnline;
	int		fAgeFrom;
	int		fAgeTo;
};
#endif
