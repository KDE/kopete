/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef JSPREFERENCES_H
#define JSPREFERENCES_H

#include <qptrdict.h>

#include "kcmodule.h"
#include "javascriptconfig.h"

typedef QValueList<KopeteAccount*> AccountList;

class JavaScriptPrefsBase;
class KTempFile;
class Script;

class JavaScriptPreferences : public KCModule
{
	Q_OBJECT

	public:
		JavaScriptPreferences( QWidget *parent = 0, const char *name = 0,
			const QStringList &args = QStringList() );

		virtual void save();
		virtual void load();

	private slots:
		void slotAddScript();
		void slotEditScript();
		void slotEmitChanged();
		void slotUpdateScriptList();
		void slotUpdateButtons();
		void slotFileDirty( const QString &file );
		void slotEnableScript( const QVariant &scriptItem );

	private:
		//void updateHighlight();
		JavaScriptPrefsBase *preferencesDialog;
		JavaScriptConfig *config;
		KTempFile *tempFile;
		Script *currentScript;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

