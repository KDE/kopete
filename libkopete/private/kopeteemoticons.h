/*
    kopeteemoticons.cpp - Kopete Preferences Container-Class

    Copyright (c) 2002-2003 by Stefan Gehn            <metz AT gehn.net>
    Kopete    (c) 2002-2004 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef kopeteemoticons_h__
#define kopeteemoticons_h__

#include <qobject.h>
#include <qvaluelist.h>
#include <qregexp.h>

#include <kopete_export.h>

namespace Kopete {

class KOPETE_EXPORT Emoticons : public QObject
{
	Q_OBJECT

public:
	/**
	 * Constructor: DON'T use it if you want to use the emoticon theme
	 * chosen by the user.
	 * For that one use the singleton stuff
	 **/
	Emoticons( const QString &theme = QString::null );

	~Emoticons(  );

	/**
	 * The emoticons container-class by default is a singleton object.
	 * Use this method to retrieve the instance.
	 */
	static Emoticons *self();

	/**
	 * Use it to parse emoticons in a text.
	 * You don't need to use this for chat windows,
	 * There is a special class that abstract a chat view
	 * and uses emoticons parser.
	 * This function will use the selected emoticon theme.
	 * If nicks is provided, they will not be parsed if they 
	 * exist in message.
	 */
	static QString parseEmoticons( const QString &message ) ;
	
	QString parse( const QString &message );
	
	/**
	 * Return all emoticons and the corresponding icon.
	 * (only one emoticon per image)
	 */
	QMap<QString, QString> emoticonAndPicList();


private:
	/**
	 * Our instance
	 **/
	static Emoticons *s_self;

	/**
	 * add an emoticon to our mapping if
	 * an animation/pixmap has been found for it
	 **/
	void addIfPossible( const QString& filenameNoExt, const QStringList &emoticons );

	struct Emoticon;
	class Private;
	Private *d;
private slots:

	/**
	 * Fills the map with paths and emoticons
	 * This needs to be done on every emoticon-theme change
	 **/
	void initEmoticons ( const QString &theme = QString::null );
};


} //END namespace Kopete

#endif
// vim: set noet ts=4 sts=4 sw=4:
