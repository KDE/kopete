/*
    cryptographyselectuserkey.h  -  description

    Copyright (C) 2002      by Olivier Goffart <ogoffart@kde.org>
    Copyright (c) 2007      by Charles Connell <charles@connells.org>

    Kopete    (c) 2002-2007 by the Kopete developers <kopete-devel@kde.org>

    ***************************************************************************
    *                                                                         *
    *   This program is free software; you can redistribute it and/or modify  *
    *   it under the terms of the GNU General Public License as published by  *
    *   the Free Software Foundation; either version 2 of the License, or     *
    *   (at your option) any later version.                                   *
    *                                                                         *
    ***************************************************************************
*/

#ifndef CRYPTOGRAPHYSELECTUSERKEY_H
#define CRYPTOGRAPHYSELECTUSERKEY_H

#include <kdialog.h>

namespace Kopete { class MetaContact; }
namespace Kleo { class EncryptionKeyRequester; }

/**
  *@author Olivier Goffart
  * Dialog to select the public key to associate with a metacontact
  */

class CryptographySelectUserKey : public KDialog
{
		Q_OBJECT
	public:
		CryptographySelectUserKey ( const QString &key, Kopete::MetaContact *mc );
		~CryptographySelectUserKey();

		QString publicKey() const;

	private:
		Kleo::EncryptionKeyRequester *m_KeyEdit;
		Kopete::MetaContact *m_metaContact;

};

#endif
