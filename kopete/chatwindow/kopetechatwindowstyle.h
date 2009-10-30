 /*
    kopetechatwindowstyle.h - A Chat Window Style.

    Copyright (c) 2005      by Michaël Larouche     <larouche@kde.org>

    Kopete    (c) 2002-2008 by the Kopete developers <kopete-devel@kde.org>

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


#include <QHash>

#include <kopete_export.h>

/**
 * This class represent a single chat window style.
 *
 * @author Michaël Larouche <larouche@kde.org>
 */
class KOPETECHATWINDOW_SHARED_EXPORT ChatWindowStyle : public QObject
{
public:
	/**
	 * StyleVariants is a typedef to a QHash
	 * key = Variant Name
	 * value = Path to variant CSS file.
	 * Path is relative to Resources directory.
	 */
	typedef QHash<QString,QString> StyleVariants;

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
	explicit ChatWindowStyle(const QString &styleName, StyleBuildMode styleBuildMode = StyleBuildNormal);
	ChatWindowStyle(const QString &styleName, const QString &variantPath, StyleBuildMode styleBuildMode = StyleBuildFast);
	~ChatWindowStyle();

	/**
	 * Checks if the style is valid
	 */
	bool isValid() const;

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
	QString getStyleName() const;

	/**
	 * Get the style resource directory.
	 * Resources directory is the base where all CSS, HTML and images are located.
	 *
	 * Adium(and now Kopete too) style directories are disposed like this:
	 * StyleName/
	 *          Contents/
	 *            Resources/
	 *
	 * @return the path to the resource directory.
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

	QString getFileTransferIncomingHtml() const;

	QString getVoiceClipIncomingHtml() const;

	QString getOutgoingStateSendingHtml() const;
	QString getOutgoingStateSentHtml() const;
	QString getOutgoingStateErrorHtml() const;
	QString getOutgoingStateUnknownHtml() const;

	/**
	 * Check if the style has the support for Kopete Action template (Kopete extension)
	 * @return true if the style has Action template.
	 */
	bool hasActionTemplate() const;

	/**
	 * Check if the supplied variant has a compact form
	 */
	bool hasCompact( const QString & variant ) const;

	/**
	 * Return the compact version of the given style variant.
	 * For the unmodified style, this returns "Variants/_compact_.css"
	 */
	QString compact( const QString & variant ) const;

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
	void init(const QString &styleName, StyleBuildMode styleBuildMode);

	/**
	 * List available variants for the current style.
	 */
	void listVariants();

private:
	class Private;
	Private * const d;
};

#endif
