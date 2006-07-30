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

#include <sys/types.h>

#include <qptrdict.h>

#include <knewstuff/knewstuff.h>
#include <kcmodule.h>
#include "javascriptconfig.h"

typedef QValueList<Kopete::Account*> AccountList;

class JavaScriptPrefsBase;
class KTempFile;
class Script;
class KDialogBase;
class JavaScriptDialog;

class JavaScriptPreferences : public KCModule, public KNewStuff
{
	Q_OBJECT

	public:
		JavaScriptPreferences( QWidget *parent = 0, const char *name = 0,
			const QStringList &args = QStringList() );

		virtual void save();
		virtual void load();

		//KNewStuff
		virtual bool install( const QString &fileName );
		virtual bool createUploadFile( const QString &fileName );

	signals:
		void installPackage( const QString &fileName, bool &retVal );

	private slots:
		void slotAddScript();
		void slotAddComplete();
		void slotAddDone();
		void slotEditScript();
		void slotEditDone();
		void slotEmitChanged();
		void slotUpdateScriptList();
		void slotDownloadScript();
		void slotUpdateButtons();
		void slotWaitForEdit();
		void slotFileDirty( const QString &file );
		void slotEnableScript( const QVariant &scriptItem );

	private:
		//void updateHighlight();
		JavaScriptPrefsBase *preferencesDialog;
		JavaScriptConfig *config;
		KTempFile *tempFile;
		Script *currentScript;
		KDialogBase *addScriptDialog;
		JavaScriptDialog *nameDialog;
		pid_t editProcess;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

