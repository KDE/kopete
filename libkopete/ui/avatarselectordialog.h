/*
    avatarselectordialog.h - Dialog to manage and select user avatar

    Copyright (c) 2007      by Michaël Larouche      <larouche@kde.org>

    Kopete    (c) 2002-2007 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/
#ifndef KOPETE_AVATARSELECTORDIALOG_H
#define KOPETE_AVATARSELECTORDIALOG_H

// KDE includes
#include <kdialog.h>

// Kopete includes
#include <kopete_export.h>

namespace Kopete
{

namespace UI
{

/**
 * @brief Dialog to manage and select user avatar
 *
 * Using AvatarSelectorDialog is very simple, first create and show the dialog:
 * @code
Kopete::UI::AvatarSelectorDialog *avatarDialog = new Kopete::UI::AvatarSelectorDialog(parent);
connect(avatarDialog, SIGNAL(result(Kopete::UI::AvatarSelectorDialog*)), this, SLOT(avatarDialogResult(Kopete::UI::AvatarSelectorDialog*)));
avatarDialog->show();
 * @endcode
 *
 * then in the resulting slot, retrieve the path of the selected avatar:
 * @code
void SpamEgg::avatarDialogResult(Kopete::UI::AvatarSelectorDialog *dialog)
{
	// Set avatar to Myself metacontact
	Kopete::ContactList::self()->myself()->setPhoto( KUrl(dialog->selectedAvatarPath()) );
}
 * @endcode
 *
 * @author Michaël Larouche <larouche@kde.org>
 */
class KOPETE_EXPORT AvatarSelectorDialog : public KDialog
{
	Q_OBJECT
public:
	/**
	 * Create a new AvatarSelectorDialog
	 * @param parent Parent widget
	 */
	AvatarSelectorDialog(QWidget *parent = 0);
	/**
	 * Clean-up ressource of AvatarSelectorDialog
	 */
	virtual ~AvatarSelectorDialog();

	/**
	 * @brief Get the selected avatar in dialog
	 *
	 * This method return the path of the selected avatar in
	 * dialog. Call this method in the resulting slot of 
	 * signal result().
	 * @return the absolute path to the avatar
	 */
	QString selectedAvatarPath() const;

Q_SIGNALS:
	/**
	 * This signal is emitted when Apply has been clicked
	 * and the dialog has been closed.
	 *
	 * @param dialog referring AvatarSelectorDialog
	 */
	void result(Kopete::UI::AvatarSelectorDialog *dialog);

private Q_SLOTS:
	/**
	 * @internal
	 * Apply button has been clicked
	 */
	void buttonOkClicked();

private:
	class Private;
	Private *d;
};

} // namespace UI

} // namespace Kopete

#endif
