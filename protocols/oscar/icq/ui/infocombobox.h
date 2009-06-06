/*
 Kopete Oscar Protocol
 infocombobox.h - ComboBox for user info

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

#ifndef INFOCOMBOBOX_H
#define INFOCOMBOBOX_H

#include <QtGui/QComboBox>

class InfoComboBox : public QComboBox
{
	Q_OBJECT

public:
	InfoComboBox( QWidget *parent = 0 );

	/**
	 * Set read only mode for the combo box
	 *
	 * @note In read only mode the combo box shows QLineEdit and
	 *       the list of items can't be shown.
	 */
	void setReadOnly( bool readOnly );
	bool isReadOnly() const;

	virtual void showPopup();

private:
	bool mReadOnly;
};

#endif
