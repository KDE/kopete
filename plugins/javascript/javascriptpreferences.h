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

#include "ui_javascriptprefsbase.h"

#include <kcmodule.h>
#include <knewstuff/knewstuff.h>

class JavaScriptPreferences
	: public KCModule
	, public KNewStuff
	, Ui::JavaScriptPrefsBase
{
	Q_OBJECT

public:
	JavaScriptPreferences( QWidget *parent = 0, const QStringList &args = QStringList());
	~JavaScriptPreferences();

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
	struct Private;
	Private *d;
};

#endif

