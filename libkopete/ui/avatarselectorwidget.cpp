/*
    avatarselectorwidget.cpp - Widget to manage and select user avatar

    Copyright (c) 2007      by Michaël Larouche      <larouche@kde.org>

    Kopete    (c) 2002-2007 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/
#include "avatarselectorwidget.h"

// Qt includes

// KDE includes
#include <kdebug.h>
#include <klocale.h>

// Kopete includes

#include "ui_avatarselectorwidget.h"

namespace Kopete
{
namespace UI
{

class AvatarSelectorWidget::Private
{
public:
	Ui::AvatarSelectorWidget mainWidget;
};

AvatarSelectorWidget::AvatarSelectorWidget(QWidget *parent)
 : QWidget(parent), d(new Private)
{
	d->mainWidget.setupUi(this);
}

AvatarSelectorWidget::~AvatarSelectorWidget()
{
	delete d;
}

void AvatarSelectorWidget::applyAvatar()
{
	//TODO
}

} // Namespace Kopete::UI

} // Namespace Kopete

#include "avatarselectorwidget.moc"
