 /*
    kopetechatwindowstyle.h - A Chat Window Style.

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
#ifndef KOPETECHATWINDOWSTYLE_H
#define KOPETECHATWINDOWSTYLE_H

#include <qstring.h>
#include <qmap.h>


/**
 * This class represent a single chat window style.
 *
 * @author Michaël Larouche <michael.larouche@kdemail.net>
 */
class ChatWindowStyle
{
public:
	/**
	 * StyleVariants is a typedef to a QMap
	 * key = Variant Name
	 * value = Path to variant CSS file.
	 * Path is relative to Ressources directory.
	 */
	typedef QMap<QString,QString> StyleVariants;

	/**
	 * This enum specifies the mode of the constructor
	 * - StyleBuildFast : Build the style the fatest possible
	 * - StyleBuildNormal : List all variants of this style. Require a async dir list.
	 */
	enum StyleBuildMode { StyleBuildFast, StyleBuildNormal};

	/**
	 * @brief Build a single chat window style.
	 * 
	 */
	ChatWindowStyle(const QString &stylePath, int styleBuildMode = StyleBuildNormal);
	ChatWindowStyle(const QString &stylePath, const QString &variantPath, int styleBuildMode = StyleBuildFast);
	~ChatWindowStyle();

	/**
	 * Get the list of all variants for this theme.
	 * If the variant aren't listed, it call the lister
	 * before returning the list of the Variants.
	 * If the variant are listed, it just return the cached
	 * variant list.
	 * @return the StyleVariants QMap.
	 */
	StyleVariants getVariants();

	/**
	 * Get the style path.
	 * The style path points to the directory where the style is located.
	 * ex: ~/.kde/share/apps/kopete/styles/StyleName/
	 *
	 * @return the style path based.
	 */
	QString getStylePath() const;

	/**
	 * Get the style ressource directory.
	 * Ressources directory is the base where all CSS, HTML and images are located.
	 *
	 * Adium(and now Kopete too) style directories are disposed like this:
	 * StyleName/
	 *          Contents/
	 *            Resources/
	 *
	 * @return the path to the the ressource directory.
	 */
	QString getStyleBaseHref() const;

	QString getHeaderHtml() const;
	QString getFooterHtml() const;
	QString getIncomingHtml() const;
	QString getNextIncomingHtml() const;
	QString getOutgoingHtml() const;
	QString getNextOutgoingHtml() const;
	QString getStatusHtml() const;

	QString getActionIncomingHtml() const;
	QString getActionOutgoingHtml() const;

	/**
	 * Check if the style has the support for Kopete Action template (Kopete extension)
	 * @return true if the style has Action template.
	 */
	bool hasActionTemplate() const;

	/**
	 * Reload style from disk.
	 */
	void reload();
private:
	/**
	 * Read style HTML files from disk
	 */
	void readStyleFiles();

	/**
	 * Init this class
	 */
	void init(const QString &stylePath, int styleBuildMode);

	/**
	 * List available variants for the current style.
	 */
	void listVariants();

private:
	class Private;
	Private *d;
};

#endif
