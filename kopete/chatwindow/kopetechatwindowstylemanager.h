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
#include <qmap.h>
#include <kfileitem.h>
#include <kopete_export.h>

class ChatWindowStyle;
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
	/**
	 * StyleList typedef (a QMap)
	 * key = Name of the style (currently the directory name)
	 * value = Path to the style
	 */
	typedef QMap<QString, QString> StyleList;

	/**
	 * The StyleInstallStatus enum. It gives better return value for installStyle().
	 * - StyleInstallOk : The install went fine.
	 * - StyleNotValid : The archive didn't contain a valid Chat Window style.
	 * - StyleNoDirectoryValid : It didn't find a suitable directory to install the theme.
	 * - StyleCannotOpen : The archive couldn't be openned.
	 * - StyleUnknow : Unknow error.
	 */
	enum StyleInstallStatus { StyleInstallOk = 0, StyleNotValid, StyleNoDirectoryValid, StyleCannotOpen, StyleUnknow };

	~ChatWindowStyleManager();

	/**
	 * Singleton access to this class.
	 * @return the single instance of this class.
	 */
	static ChatWindowStyleManager *self();

	/**
	 * List all availables styles.
	 */
	void loadStyles();

	/**
	 * Get all available styles.
	 */
	StyleList getAvailableStyles();

public slots:
	/**
	 * Install a new style into user style directory
	 * Note that you must pass a path to a archive.
	 *
	 * @param styleBundlePath Path to the container file to install.
	 * @return a value contained in the StyleInstallStatus.
	 */
	int installStyle(const QString &styleBundlePath);

	/**
	 * Remove a style from user style directory
	 *
	 * @param styleName the name(not the path) of the style to remove
	 */
	bool removeStyle(const QString &styleName);
	
	/**
	 * Get a instance of a ChatWindowStyle from the pool.
	 * If they are no instance for the specified style, it gets created.
	 * DO NOT DELETE the resulting pointer, it is handled by this class.
	 *
	 * @param stylePath Path for the specified style. Name can be ambigous.
	 * @return the instance of ChatWindow for the specified style. DO NOT DELETE IT.
	 */
	ChatWindowStyle *getStyleFromPool(const QString &stylePath);

signals:
	/**
	 * This signal is emitted when all styles finished to list.
	 * Used to inform and/or update GUI.
	 */
	void loadStylesFinished();

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
