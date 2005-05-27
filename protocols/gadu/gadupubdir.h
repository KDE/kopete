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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.
//

#ifndef GADUPUBDIR_H
#define GADUPUBDIR_H

#include "gadusearch.h"
#include "gadusession.h"

#include <kdebug.h>
#include <kdialogbase.h>

class GaduAccount;
class GaduProtocol;
class GaduContact;
class GaduAccount;
class GaduPublicDirectory;
class QListViewItem;
class GaduContact;

class GaduPublicDir : public KDialogBase
{
Q_OBJECT

public:
	GaduPublicDir( GaduAccount* , QWidget *parent = 0, const char* name = "GaduPublicDir" );
	GaduPublicDir( GaduAccount* , int searchFor, QWidget* parent = 0, const char* name = "GaduPublicDir" );
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
	GaduPublicDirectory*	mMainWidget;

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
