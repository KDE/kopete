/*
    kopeteawayconfigui.h  -  Kopete Away Config UI

    Copyright (c) 2002 by Chris TenHarmsel       <ctenha56@calvin.edu>

    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef __KOPETEAWAYCONFIGUI_H_
#define __KOPETEAWAYCONFIGUI_H_

#include "kopeteawayconfigbase.h"

#include <qwidget.h>

/**
 * @author Chris TenHarmsel <ctenha56@calvin.edu>
 *
 * @class KopeteAwayConfigUI kopeteawayconfigui.h
 *
 * This class represents the interface in the User Preference KCM
 * for configuring Away Messages
 */
class KopeteAwayConfigUI : public KopeteAwayConfigBaseUI
{
	Q_OBJECT

public:
	/**
	 * @brief Constructor for the class
	 * @param parent The parent object
	 */
	KopeteAwayConfigUI( QWidget *parent );

	/**
	 * @brief Updates the view from the values in global KopeteAway
	 */
	void updateView();

protected slots:
	/**
	 * @brief Slot that is called when the "Add..." button is clicked
	 */
	void btnAddAwayMsgClicked();

	/**
	 * @brief Slot that is called when the "Delete" button is clicked
	 */
	void btnDeleteAwayMsgClicked();

	/**
	 * @brief Slot that is called when the "Edit.." button is clicked
	 */
	void btnEditAwayMsgClicked();

	/**
	 * @brief Slot that is called when a new title is selected in the title list
	 */
	void titleSelected();
signals:
	/**
	 * @brief Signal emitted when the away messages change in any way
	 */
	void awayMessagesChanged(bool);
};

#endif
// vim: set noet ts=4 sts=4 sw=4:
