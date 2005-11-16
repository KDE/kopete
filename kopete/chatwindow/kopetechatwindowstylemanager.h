 /*
    kopetechatwindowstylemanager.h - Manager all chat window styles

    Copyright (c) 2005      by Michaël Larouche     <michael.larouche@kdemail.net>

    Kopete    (c) 2002-2005 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef KOPETECHATWINDOWSTYLEMANAGER_H
#define KOPETECHATWINDOWSTYLEMANAGER_H

#include <qobject.h>
#include <kfileitem.h>
#include <kopete_export.h>

/**
 * Sigleton class that create all the ChatWindowStyle objects.
 * This class list all the available styles in $KDEDATADIR/kopete/styles
 *
 * Each style is in own subdirectory
 * 
 * @author Michaël Larouche <michael.larouche@kdemail.net>
 */
class KOPETE_EXPORT ChatWindowStyleManager : public QObject
{
	Q_OBJECT
public:
	~ChatWindowStyleManager();

	/**
	 * Singleton access to this class.
	 * @return the single instance of this class.
	 */
	static ChatWindowStyleManager *self();

	/**
	 * Init this class.
	 * Create the latest current chat window style.
	 */
	void load();

	/**
	 * List all availables styles.
	 */
	void loadStyles();

private slots:
	void slotNewStyles(const KFileItemList &dirList);
	void slotDirectoryFinished();

private:
	ChatWindowStyleManager(QObject *parent = 0, const char *name = 0);

	static ChatWindowStyleManager *s_self;

	class Private;
	Private *d;
};

#endif
