/***************************************************************************
                          cryptographyselectuserkey.h  -  description
                             -------------------
    begin                : dim nov 17 2002
    copyright            : (C) 2002 by Olivier Goffart
    email                : ogoffart @ kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CRYPTOGRAPHYSELECTUSERKEY_H
#define CRYPTOGRAPHYSELECTUSERKEY_H

#include <kdialogbase.h>

namespace Kopete { class MetaContact; }
class CryptographyUserKey_ui;

/**
  *@author OlivierGoffart
  */

class CryptographySelectUserKey : public KDialogBase {
	Q_OBJECT
public:
	CryptographySelectUserKey(const QString &key, Kopete::MetaContact *mc);
	~CryptographySelectUserKey();


  QString publicKey() const;

private slots:
	void keySelected(QString &);
	void slotSelectPressed();
  /** No descriptions */
  void slotRemovePressed();

private:
	CryptographyUserKey_ui *view;
	Kopete::MetaContact *m_metaContact;

};

#endif
