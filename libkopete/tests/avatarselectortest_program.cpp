/*
    Avatar Selector Widget test

    Copyright (c) 2007      by Michaël Larouche       <larouche@kde.org>

    Kopete    (c) 2002-2007 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
#include <kcmdlineargs.h>
#include <kaboutdata.h>

#include "avatarselectorwidget.h"

int main(int argc, char **argv)
{
	KAboutData aboutData( "avatarselectortest", "KopeteAvatarSelectorWidgetTest",
		"0.1.0", "Kopete Avatar Selector Widget Test", KAboutData::License_GPL,
		"Michaël Larouche", "larouche@kde.org", "http://kopete.kde.org");

	KCmdLineArgs::init( argc, argv, &aboutData );

	KApplication app;

	Kopete::UI::AvatarSelectorWidget testWidget;
	testWidget.show();

	return app.exec();
}
