/*
    cryptographypreferences.h

    Copyright (c) 2002      by Olivier Goffart <ogoffart@kde.org>
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

#ifndef CRYPTOGRAPHYPREFERENCES_H
#define CRYPTOGRAPHYPREFERENCES_H

#include "kcmodule.h"

#include <QVariantList>

/**
 * Preference widget for the Cryptography plugin
 * @author Olivier Goffart
 * @author Charles Connell
 */

namespace Ui { class CryptographyPrefsUI; }
class CryptographyConfig;
class Kleo::EncryptionKeyRequester;
class QCheckBox;

class CryptographyPreferences : public KCModule
{

		Q_OBJECT

	public:
		explicit CryptographyPreferences ( QWidget *parent = 0, const QVariantList &args = QVariantList() );
		virtual ~CryptographyPreferences();
		virtual void save();
		virtual void load();
		virtual void defaults();

	private:
		Kleo::EncryptionKeyRequester * key;
		QCheckBox * mAskPassphraseOnStartup;
		Ui::CryptographyPrefsUI *mPreferencesDialog;
		CryptographyConfig *mConfig;

	private slots:
		void slotModified();
		void slotAskOnStartupPressed (bool b);
};

#endif

// vim: set noet ts=4 sts=4 sw=4:
