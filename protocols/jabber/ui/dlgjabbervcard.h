
/***************************************************************************
                          dlgjabbervcard.h  -  vCard dialog
                             -------------------
    begin                : Thu Aug 08 2002
    copyright            : (C) 2002-2003 by Till Gerken <till@tantalo.net>
                           (C) 2005      by Michaël Larouche <larouche@kde.org>
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

#include <kdialog.h>
#include "xmpp_vcard.h"

class JabberAccount;
class JabberBaseContact;
class QString;
namespace Ui { class dlgVCard; }

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
 * @author Michaël Larouche <larouche@kde.org>
 */
class dlgJabberVCard : public KDialog
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
	dlgJabberVCard (JabberAccount *account, JabberBaseContact *contact, QWidget * parent = 0);
	~dlgJabberVCard ();

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
	 * Send vCard to the server.
	 */
	void slotSaveVCard();
	/**
	 * Put back the information from the dialog into the contact properties
	 */
	void slotVCardSaved();
	/**
	 * Close the dialog.
	 */
	void slotClose();
	/**
	 * Open a link. (ex: the homepage link or the email address)
	 */
	void slotOpenURL(const QString &url);

	/**
	 * Retrieve vCard information for the current contact.
	 */
	void slotGetVCard();
	/**
	 * vCard was successfully fetched, update contact properties
	 * and enable display.
	 */
	void slotGotVCard();

private:
	JabberAccount *m_account;
	JabberBaseContact *m_contact;
	Ui::dlgVCard *m_mainWidget;
	QString m_photoPath;

	void assignContactProperties();
	void setReadOnly(bool state);
	void setEnabled(bool state);
	
};

#endif // DLGJABBERVCARD_H

/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
