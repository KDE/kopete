/*  This file is part of the KDE project
    Copyright (C) 2007 Will Stephenson <wstephenson@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2
    as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "tutorialpreferences.h"

#include <kgenericfactory.h>

typedef KGenericFactory<TutorialPreferences> TutorialPreferencesFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_kopete_tutorialplugin, TutorialPreferencesFactory( "kcm_kopete_tutorialplugin" ))

TutorialPreferences::TutorialPreferences(QWidget *parent, const QStringList &args)
    : KCModule(TutorialPreferencesFactory::componentData(), parent, args)
{
    ui.setupUi(this);
}

TutorialPreferences::~TutorialPreferences()
{

}

void TutorialPreferences::save()
{

}

void TutorialPreferences::load()
{
}

#include "tutorialpreferences.moc"
