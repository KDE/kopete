/*
    pipesconfig.cpp

    Copyright (c) 2007      by Charles Connell <charles@connells.org>

    Kopete    (c) 2007      by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
#include "pipesconfig.h"

#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include <kglobal.h>

PipesConfig * PipesConfig::mSelf = 0;

PipesConfig * PipesConfig::self ()
{
	if (!mSelf)
		mSelf = new PipesConfig();
	return mSelf;
}

PipesPlugin::PipeOptionsList PipesConfig::pipes ()
{
	if (!mSelf)
		mSelf = new PipesConfig();
	return mSelf->mPipesList;
}

void PipesConfig::setPipes ( PipesPlugin::PipeOptionsList pipes )
{
	if (!mSelf)
		mSelf = new PipesConfig();
	mSelf->mPipesList = pipes;
}

void PipesConfig::save ()
{
	KConfigGroup config (KGlobal::config(), "PipesPlugin_Pipes");
	config.deleteGroup();
	
	QStringList uids;
	foreach (PipesPlugin::PipeOptions pipe, mPipesList){
		config.writeEntry ( QString(pipe.uid) + "enabled", pipe.enabled );
		config.writeEntry ( QString(pipe.uid) + "path", pipe.path );
		config.writeEntry ( QString(pipe.uid) + "direction", (int) pipe.direction );
		config.writeEntry ( QString(pipe.uid) + "pipeContents", (int) pipe.pipeContents);
		uids.append (pipe.uid.toString());
	}
	config.writeEntry ( "Pipes", uids);
}

void PipesConfig::load ()
{
	KConfigGroup config (KGlobal::config(), "PipesPlugin_Pipes");
	const QStringList uidList = config.readEntry ("Pipes", QStringList());
	
	PipesPlugin::PipeOptions pipeOptions;
	PipesPlugin::PipeOptionsList pipesList;
	mPipesList.clear();
	foreach (const QString& uid, uidList){
		pipeOptions.uid = uid;
		pipeOptions.enabled = config.readEntry ( uid + "enabled", true );
		pipeOptions.path = config.readEntry ( uid + "path", QString() );
		pipeOptions.direction = (PipesPlugin::PipeDirection)config.readEntry ( uid + "direction", 0 );
		pipeOptions.pipeContents = (PipesPlugin::PipeContents)config.readEntry ( uid + "pipeContents", 0 );
		mPipesList.append (pipeOptions);
	}
}

PipesConfig::PipesConfig()
{
	load();
}

