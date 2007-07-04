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

#ifndef TUTORIAL_PREFERENCES_H
#define TUTORIAL_PREFERENCES_H

#include <KCModule>

#include "ui_tutorialprefs.h"

class TutorialPreferences : public KCModule
{
Q_OBJECT
public:
    explicit TutorialPreferences(QWidget *parent = 0, const QStringList &args = QStringList());
    ~TutorialPreferences();
    virtual void save();
    virtual void load();

private:
    Ui_TutorialPrefsUI ui;
};

#endif
