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
 * Sigleton class that handle Chat Window styles. 
 * It use style absolute path to avoid unexpected behavior that could happen when using style name.
 *
 * It can install, delete styles. The styles are managed in a pool, they are only retrieved on demand.
 *
 * Use getStyleFromPool to retrieve a ChatWindowStyle instance. Do not delete the returned instance, it
 * is handled by this class.
 *
 * When called the first time, it list all the available styles in $KDEDATADIR/kopete/styles and
 * KDirWatch (via KDirLister) watch for new styles. 
 *
 * If you want to keep a trace of avaiable styles, connect to loadStylesFinished() signal. 
 * It is called when KDirLister finish a job(ex: on new directory).
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

	/**
	 * Destructor.
	 */
	~ChatWindowStyleManager();

	/**
	 * Singleton access to this class.
	 * @return the single instance of this class.
	 */
	static ChatWindowStyleManager *self();

	/**
	 * List all availables styles.
	 * Init KDirLister and thus KDirWatch that watch for new styles.
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
	 * @return A status code from StyleInstallStatus enum.
	 */
	int installStyle(const QString &styleBundlePath);

	/**
	 * Remove a style from user style directory
	 *
	 * @param stylePath the path of the style to remove.
	 * @return true if the deletion went without problems.
	 */
	bool removeStyle(const QString &stylePath);
	
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
	/**
	 * KDirLister found new files.
	 * @param dirList new files found.
	 */
	void slotNewStyles(const KFileItemList &dirList);
	/**
	 * KDirLister finished a job.
	 * Emit loadStylesFinished() if they are no directory left in the stack.
	 */
	void slotDirectoryFinished();

private:
	/**
	 * Private constructor(it's a singleton class)
	 * Call loadStyles() to list all avaiable styles.
	 */
	ChatWindowStyleManager(QObject *parent = 0, const char *name = 0);

	static ChatWindowStyleManager *s_self;

	class Private;
	Private *d;
};

#endif
