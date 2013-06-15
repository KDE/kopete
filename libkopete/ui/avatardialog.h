/*
    avatardialog.h - Dialog to manage and select user avatar

    Copyright (c) 2007      by Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>
    Copyright (c) 2007      by Michaël Larouche       <larouche@kde.org>

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
#ifndef KOPETE_AVATARDIALOG_H
#define KOPETE_AVATARDIALOG_H

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
 * Using AvatarDialog is very simple, if you only want to retrieve an avatar, use 
 * the @ref AvatarDialog::getAvatar() static method.
 *
 * If you want the operation to be asynchronous, you can use it like the following 
 * example:
 * @code
Kopete::UI::AvatarDialog *avatarDialog = new Kopete::UI::AvatarDialog(parent);
connect(avatarDialog, SIGNAL(result(Kopete::UI::AvatarDialog*)), this, SLOT(avatarDialogResult(Kopete::UI::AvatarDialog*)));
avatarDialog->show();
 * @endcode
 *
 * then in the resulting slot, retrieve the path of the selected avatar:
 * @code
void SpamEgg::avatarDialogResult(Kopete::UI::AvatarDialog *dialog)
{
	// Set avatar to Myself metacontact
	Kopete::ContactList::self()->myself()->setPhoto( KUrl(dialog->selectedAvatarPath()) );
}
 * @endcode
 *
 * @author Michaël Larouche <larouche@kde.org>
 */
class KOPETE_EXPORT AvatarDialog : public KDialog
{
	Q_OBJECT
public:
	/**
	 * Create a new AvatarDialog
	 * @param parent Parent widget
	 */
	AvatarDialog(QWidget *parent = 0);
	/**
	 * Clean-up resource of AvatarDialog
	 */
	virtual ~AvatarDialog();

	/**
	 * @brief Get the selected avatar in dialog
	 *
	 * This method return the path of the selected avatar in
	 * dialog. Call this method in the resulting slot of 
	 * signal result().
	 * @return the absolute path to the avatar
	 */
	QString selectedAvatarPath() const;

	/**
	 * @brief Gets an avatar from the AvatarManager
	 *
	 * This method will open the avatar dialog for the user to choose
	 * an avatar.
	 * @param parent Parent widget
	 * @return The path of the selected avatar, or QString() if no avatar
	 * was chosen or if the Cancel button was pressed.
	 */
	static QString getAvatar(QWidget *parent = 0, const QString &currentAvatar = QString(), bool * ok = 0 );

Q_SIGNALS:
	/**
	 * This signal is emitted when Ok has been clicked
	 * before the dialog is closed
	 */
	void result();

protected Q_SLOTS:
	/**
	 * @internal
	 * A button has been clicked. Reimplemented from @ref KDialog::slotButtonClicked()
	 */
	virtual void slotButtonClicked(int button);

private:
	class Private;
	Private * const d;
};

} // namespace UI

} // namespace Kopete

#endif
