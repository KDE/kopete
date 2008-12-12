/*
    kopeteinfodialog.h - A dialog to display and configure information 

    Copyright (c) 2007      by Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>

    Kopete    (c) 2002-2007 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef KOPETEINFODIALOG_H
#define KOPETEINFODIALOG_H

#include <KDE/KDialog>
#include "kopete_export.h"

namespace Kopete
{

namespace UI
{

/**
 * \brief a dialog for displaying and configuring information
 *
 * @author Gustavo Pichorim Boiko <gustavo.boiko AT kdemail.net>
 */
class KOPETE_EXPORT InfoDialog : public KDialog
{
Q_OBJECT

public:
	/**
	 * Constructor.
	 *
	 * @param parent the parent of this widget
	 * @param title the title to be shown in the dialog
	 * @param icon the name of the icon to be used
	 */
	InfoDialog(QWidget *parent, 
			   const QString &title = QString(), const QString &icon = QString());
	/**
	 * Constructor.
	 *
	 * @param parent the parent of this widget
	 * @param title the title to be shown in the dialog
	 * @param icon the icon to be used
	 */
	InfoDialog(QWidget *parent, 
			   const QString &title, const KIcon &icon);

	/**
	 * Destructor.
	 */
	~InfoDialog();

	void setTitle(const QString &title);
	void setIcon(const QString &icon);
	void setIcon(const KIcon &icon);
	
	void addWidget(QWidget *w, const QString &caption);

protected slots:
	/**
	 * This should be reimplemented in derived dialogs to enable saving info
	 */
	virtual void slotSave();

private:
	class Private;
	Private * const d;

	void initialize();

};

} // namespace UI
} // namespace Kopete
#endif
// vim: set noet ts=4 sts=4 sw=4:
