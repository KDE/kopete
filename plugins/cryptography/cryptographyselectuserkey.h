/*
    cryptographyselectuserkey.h  -  description

    Copyright (C) 2002      by Olivier Goffart <ogoffart@kde.org>

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
namespace Ui { class CryptographyUserKey_ui; }

/**
  *@author OlivierGoffart
  */

class CryptographySelectUserKey : public KDialog {
	Q_OBJECT
public:
	CryptographySelectUserKey(const QString &key, Kopete::MetaContact *mc);
	~CryptographySelectUserKey();


  QString publicKey() const;

private slots:
	void keySelected(const QString &);
	void slotSelectPressed();
  /** No descriptions */
  void slotRemovePressed();

private:
	Ui::CryptographyUserKey_ui *view;
	Kopete::MetaContact *m_metaContact;

};

#endif
