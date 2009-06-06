/*
 Kopete Oscar Protocol
 infocombobox.cpp - ComboBox for user info

 Copyright (c) 2007 Roman Jarosz <kedgedev@centrum.cz>

 Kopete (c) 2007 by the Kopete developers <kopete-devel@kde.org>

 *************************************************************************
 *                                                                       *
 * This library is free software; you can redistribute it and/or         *
 * modify it under the terms of the GNU Lesser General Public            *
 * License as published by the Free Software Foundation; either          *
 * version 2 of the License, or (at your option) any later version.      *
 *                                                                       *
 *************************************************************************
*/

#include "infocombobox.h"

#include <QtGui/QLineEdit>

InfoComboBox::InfoComboBox( QWidget *parent )
 : QComboBox( parent ), mReadOnly( false )
{
}

void InfoComboBox::setReadOnly( bool readOnly )
{
	if ( mReadOnly != readOnly )
	{
		mReadOnly = readOnly;

		setInsertPolicy( ( readOnly ) ? QComboBox::NoInsert : QComboBox::InsertAtBottom );
		setEditable( readOnly );
		lineEdit()->setReadOnly( readOnly );
	}
}

bool InfoComboBox::isReadOnly() const
{
	return mReadOnly;
}

void InfoComboBox::showPopup()
{
	if ( mReadOnly )
		return;

	QComboBox::showPopup();
}



