/*
    exportkeys.cpp

    Copyright (c) 2007      by Charles Connell        <charles@connells.org>

    Kopete    (c) 2007 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
#ifndef EXPORTKEYS_H
#define EXPORTKEYS_H

#include <kdialog.h>

#include <kabc/addresseelist.h>

namespace Kopete { 	class MetaContact; }
namespace Ui { class ExportKeysUI; }

/**
Dialog that exports public keys from Kopete to KABC

	@author Charles Connell <charles@connells.org>
*/
class ExportKeys : public KDialog
{
		Q_OBJECT
	public:
		explicit ExportKeys ( QList<Kopete::MetaContact*> mcs, QWidget *parent = 0 );

		~ExportKeys();
		
	protected slots:
		void accept();

	private:
		Ui::ExportKeysUI * mUi;
		KABC::AddresseeList mAddressees;
		QList<Kopete::MetaContact*> mMetaContacts;
};

#endif
