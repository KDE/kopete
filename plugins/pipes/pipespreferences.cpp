/*
    pipespreferences.cpp

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
#include "pipespreferences.h"

#include <QHeaderView>
#include <QCheckBox>
#include <QComboBox>

#include <kpluginfactory.h>
#include <kfiledialog.h>

#include "ui_pipesprefsbase.h"
#include "pipesmodel.h"
#include "pipesdelegate.h"
#include "pipesplugin.h"
#include "pipesconfig.h"

K_PLUGIN_FACTORY ( PipesPreferencesFactory, registerPlugin<PipesPreferences>(); )
K_EXPORT_PLUGIN ( PipesPreferencesFactory ( "kcm_kopete_pipes" ) )

PipesPreferences::PipesPreferences ( QWidget *parent, const QVariantList &args )
	: KCModule ( PipesPreferencesFactory::componentData(), parent, args )
{
	mPrefs = new Ui::PipesPrefsUI;
	mPrefs->setupUi (this);

	mPrefs->pipesList->setSortingEnabled (false);

	mModel = new PipesModel (this);
	PipesDelegate * delegate = new PipesDelegate (this);

	mPrefs->pipesList->setModel (mModel);
	mPrefs->pipesList->setItemDelegate (delegate);

	mPrefs->pipesList->horizontalHeader()->setStretchLastSection (true);
	mPrefs->pipesList->verticalHeader()->hide();

	connect (mPrefs->addButton, SIGNAL (clicked()), this, SLOT(slotAdd()));
	connect (mPrefs->removeButton, SIGNAL (clicked()), this, SLOT(slotRemove()));
	connect (mModel, SIGNAL (dataChanged(QModelIndex,QModelIndex)), this, SLOT(slotListChanged()));
	connect (mModel, SIGNAL (modelReset()), this, SLOT (slotListChanged()));
	
	slotListChanged();
}

PipesPreferences::~PipesPreferences()
{
	delete mPrefs;
}

void PipesPreferences::load()
{
	PipesConfig::self()->load();
	mModel->setPipes (PipesConfig::pipes());

	emit KCModule::changed(false);
}

void PipesPreferences::save()
{
	PipesConfig::setPipes (mModel->pipes());
	PipesConfig::self()->save();

	emit KCModule::changed(false);
}

// We get a filename, then fill out a PipeOptions class with some default data,
// then pass it to the file.
void PipesPreferences::slotAdd()
{
	const QString filePath = KFileDialog::getOpenFileName( KUrl ("kfiledialog:///pipesplugin"), QString(), this, i18n ("Select Program or Script to Pipe Messages Through"));
	if (filePath.isEmpty())
		return;
	PipesPlugin::PipeOptions pipe;
	pipe.uid = QUuid::createUuid();
	pipe.path = filePath;
	pipe.direction = PipesPlugin::BothDirections;
	pipe.pipeContents = PipesPlugin::HtmlBody;
	pipe.enabled = true;
	mModel->addPipe (pipe);
}

void PipesPreferences::slotRemove()
{
	mModel->removeRow( mPrefs->pipesList->currentIndex().row(), QModelIndex() );
}

// when the data changes, we tell the view to resize itself
void PipesPreferences::slotListChanged()
{
	mPrefs->pipesList->resizeColumnToContents(PipesDelegate::EnabledColumn);
	mPrefs->pipesList->resizeColumnToContents(PipesDelegate::DirectionColumn);
	mPrefs->pipesList->resizeColumnToContents(PipesDelegate::ContentsColumn);
	changed();
}

