/*
    kabckeyselector.cpp  -  description

    Copyright (c) 2007      by Charles Connell <charles@connells.org>

    Kopete    (c) 2007 by the Kopete developers  <kopete-devel@kde.org>

    ***************************************************************************
    *                                                                         *
    *   This program is free software; you can redistribute it and/or modify  *
    *   it under the terms of the GNU General Public License as published by  *
    *   the Free Software Foundation; either version 2 of the License, or     *
    *   (at your option) any later version.                                   *
    *                                                                         *
    ***************************************************************************
*/
#include "kabckeyselector.h"

#include <KDialog>
#include <KMessageBox>
#include <KIconLoader>

#include "ui_kabckeyselectorbase.h"

QString KabcKeySelector ( QString displayName, QString addresseeName, QStringList keys, QWidget *parent )
{
	// just a Yes/No about whether to accept the key
	if ( keys.count() == 1 ) {
		if ( KMessageBox::questionYesNo ( parent, i18n ( QString ("Cryptography plugin has found an encryption key for " + displayName + " (" + addresseeName + ")" + " in your KDE address book. Do you want to use key " + keys.first().right ( 8 ).prepend ( "0x" ) + " as this contact's public key?").toLocal8Bit() ), i18n ( "Public Key Found" ) ) == KMessageBox::Yes ) {
			return keys.first();
		}
	}
	// allow for selection of key out of many
	if ( keys.count() > 1 )
	{
		KDialog dialog (parent);
		QWidget w (&dialog);
		Ui::KabcKeySelectorUI ui;
		ui.setupUi (&w);
		dialog.setCaption ( i18n ("Public Keys Found") );
		dialog.setButtons ( KDialog::Ok | KDialog::Cancel );
		dialog.setMainWidget (&w);
		ui.label->setText ( i18n ( QString("Cryptography plugin has found multiple encryption keys for " + displayName + " (" + addresseeName + ")" + " in your KDE address book. To use one of these keys, select it and choose OK." ).toLocal8Bit() ) );
		for (int i = 0; i < keys.count(); i++)
			ui.keyList->addItem ( new QListWidgetItem ( KIconLoader::global()->loadIconSet ("kgpg-key1-kopete", K3Icon::Small), keys[i].right(8).prepend("0x"), ui.keyList) );
		ui.keyList->addItems (keys);
		if (dialog.exec())
			return ui.keyList->currentItem()->text();
	}
	return QString();
}
