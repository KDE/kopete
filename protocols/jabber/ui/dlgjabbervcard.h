
/***************************************************************************
                          dlgjabbervcard.h  -  vCard dialog
                             -------------------
    begin                : Thu Aug 08 2002
    copyright            : (C) 2002-2003 by Till Gerken <till@tantalo.net>
                           (C) 2005      by Michaël Larouche <michael.larouche@kdemail.net>
    email                : kopete-devel@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DLGJABBERVCARD_H
#define DLGJABBERVCARD_H

#include <kdialogbase.h>
#include "xmpp_vcard.h"

class JabberAccount;
class JabberContact;
class QString;
class dlgVCard;

/**
 * @brief Show the information of a Jabber contact.
 *
 * This dialog shows the information of a Jabber contact from
 * the contact properties(from Kopete). 
 * Also it is used to edit the information of the Account myself contact.
 *
 * First it fetch a new version of the vcard then it display the
 * information. User can force the update using the "Update vCard" button.
 * 
 * @author Till Gerken <till@tantolo.net>
 * @author Michaël Larouche <michael.larouche@kdemail.net>
 */
class dlgJabberVCard : public KDialogBase
{
	Q_OBJECT

public:
	/**
	 * Create the information(vcard) dialog.
	 *
	 * @param account the current Jabber account 
	 * @param contact the contact to display or edit information.
	 * @param widget Parent widget.
	 * @param name widget name.
	 */
	dlgJabberVCard (JabberAccount *account, JabberContact *contact, QWidget * parent = 0, const char *name = 0);
	~dlgJabberVCard ();

signals:
	/**
	 * This signal is emitted when information is changed.
	 */
	void informationChanged();

private slots:
	/**
	 * Show the KFileDialog for image to select a photo for the contact.
	 */
	void slotSelectPhoto();
	/**
	 * Remove(clear) the photo. 
	 * Maybe the user doesn't want to export a photo anymore.
	 */
	void slotClearPhoto();
	/**
	 * Put back the information from the dialog into the contact properties.
	 * After it emit the informationChanged() signal.
	 */
	void slotSaveVCard();
	/**
	 * Close the dialog.
	 */
	void slotClose();
	/**
	 * Open a link. (ex: the homepage link or the email address)
	 */
	void slotOpenURL(const QString &url);

private:
	JabberAccount *m_account;
	JabberContact *m_contact;
	dlgVCard *m_mainWidget;
	QString m_photoPath;

	void assignContactProperties();
	void setReadOnly(bool state);

};

#endif // DLGJABBERVCARD_H

/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
