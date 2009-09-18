/*
    wlmprotocol.h - Kopete Wlm Protocol

    Copyright (c) 2008      by Tiago Salem Herrmann <tiagosh@gmail.com>
    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU General Public                   *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef WLMPROTOCOL_H
#define WLMPROTOCOL_H

#include <kopeteprotocol.h>
#include "kopeteproperty.h"


/**
 * Encapsulates the generic actions associated with this protocol
 * @author Will Stephenson
 */
class WlmProtocol: public Kopete::Protocol
{
    Q_OBJECT
  public:
    WlmProtocol (QObject * parent, const QVariantList & args);
    ~WlmProtocol ();
    /**
	 * Convert the serialised data back into a WlmContact and add this
	 * to its Kopete::MetaContact
	 */
    virtual Kopete::Contact * deserializeContact (Kopete::MetaContact * metaContact,
                        const QMap < QString, QString > &serializedData,
                        const QMap < QString, QString > &addressBookData);
    /**
	 * Generate the widget needed to add WlmContacts
	 */
    virtual AddContactPage *
        createAddContactWidget (QWidget * parent, Kopete::Account * account);
    /**
	 * Generate the widget needed to add/edit accounts for this protocol
	 */
    virtual KopeteEditAccountWidget *
    createEditAccountWidget (Kopete::Account * account, QWidget * parent);
    /**
	 * Generate a WlmAccount
	 */
    virtual Kopete::Account *createNewAccount (const QString & accountId);
    /**
	 * Access the instance of this protocol
	 */
    static WlmProtocol * protocol ();

    static bool validContactId(const QString&);

    /**
	 * Represents contacts that are Online
	 */
    const Kopete::OnlineStatus wlmOnline;
    /**
	 * Represents contacts that are Away
	 */
    const Kopete::OnlineStatus wlmAway;
    /**
	 * Represents contacts that are Offline
	 */
    const Kopete::OnlineStatus wlmBusy;

    const Kopete::OnlineStatus wlmBeRightBack;

    const Kopete::OnlineStatus wlmOnThePhone;

    const Kopete::OnlineStatus wlmOutToLunch;

    const Kopete::OnlineStatus wlmInvisible;

    const Kopete::OnlineStatus wlmOffline;

    const Kopete::OnlineStatus wlmIdle;

    const Kopete::OnlineStatus wlmUnknown;

    const Kopete::OnlineStatus wlmConnecting;

    const Kopete::PropertyTmpl currentSong;
    
    const Kopete::PropertyTmpl contactCapabilities;

	const Kopete::PropertyTmpl displayPhotoSHA1;

  protected:
    static WlmProtocol * s_protocol;
};

#endif
