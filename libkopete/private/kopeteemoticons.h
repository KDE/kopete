/*
    kopeteemoticons.cpp - Kopete Preferences Container-Class

    Copyright (c) 2002 by Stefan Gehn            <sgehn@gmx.net>
    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef __kopeteemoticons_h__
#define __kopeteemoticons_h__

#include <qobject.h>
#include <qmap.h>
//class QMap;

typedef QMap<QString, QStringList> EmoticonMap;

/**
 * @author Stefan Gehn <sgehn@gmx.net> 
 *
 */

class KopeteEmoticons : public QObject
{
	Q_OBJECT

public:
	/**
	 * The emoticons container-class is a singleton object.
	 * Use this method to retrieve the instance.
	 */
	static KopeteEmoticons *emoticons();

	/**
	 * Use it to parse emoticons in a text.
	 * You dont need to use this for chat windows,
	 * There is a special class that abstract a chat view
	 * and uses emoticons parser.
	 * This function will use the selected emoticon theme.
	 */
	static QString parseEmoticons ( QString );


	~KopeteEmoticons();

	/**
	 * returns the path to an animation or pixmap
	 * that should be used to replace the given emoticon
	 * if the emoticon has no corresponding pixmap
	 * a null-QString is returned
	 **/
	QString emoticonToPicPath ( const QString& em );

	/**
	 *  returns a list of emoticons represented by the given pixmap-path
	 *  Usually used in conjunction with the QStringList
	 *  picList() provides.
	 **/
	QStringList picPathToEmoticon ( const QString& path );

	/**
	 * creates a list of all emoticons that can be
	 * mapped to an animation/pixmap
	 **/
	QStringList emoticonList ( void );

	/**
	 * creates a list of all animation/pixmap paths
	 * that are used to replace emoticons
	 **/
	QStringList picList ( void );

	QMap<QString, QString> emoticonAndPicList(void);


private:
	/**
	 * Private constructor: we are a singleton
	 **/
	KopeteEmoticons();
	
	/**
	 * Our instance
	 **/
	static KopeteEmoticons *s_instance;

	/**
	 * Our data, the heart of this class
	 **/
	EmoticonMap map;

	/**
	 * The current icon theme from KopetePrefs
	 */
	QString m_theme;
    
	/**
	 * add an emoticon to our mapping if
	 * an animation/pixmap has been found for it
	 **/
	void addIfPossible( const QString& filenameNoExt, QStringList emoticons );

private slots:

	/**
	 * Fills the map with paths and emoticons
	 * This needs to be done on every emoticon-theme change
	 **/
	void initEmoticons ( void );
};

#endif
/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

