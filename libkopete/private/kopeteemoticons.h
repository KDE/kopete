/*
    kopeteemoticons.cpp - Kopete Preferences Container-Class

    Copyright (c) 2002-2003 by Stefan Gehn            <metz AT gehn.net>
    Kopete    (c) 2002-2004 by the Kopete developers  <kopete-devel@kde.org>
    Copyright (c) 2005      by Engin AYDOGAN	      <engin @ bzzzt.biz>

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

#include "kopete_export.h"

namespace Kopete {

class KOPETE_EXPORT Emoticons : public QObject
{
	Q_OBJECT
public:
	/**
	 * Constructor: DON'T use it if you want to use the emoticon theme
	 * chosen by the user.
	 * Instead, use @ref Kopete::Emoticons::self()
	 **/
	Emoticons( const QString &theme = QString::null );

	~Emoticons(  );

	/**
	 * The emoticons container-class by default is a singleton object.
	 * Use this method to retrieve the instance.
	 */
	static Emoticons *self();

	/**
	 * The possible parse modes
	 */
	enum ParseMode {  DefaultParseMode = 0x0 ,  /**  Use strict or relaxed according the config  */
			StrictParse = 0x1,			/** Strict parsing requires a space between each emoticon */
			RelaxedParse = 0x4,         /** Parse mode where all possible emoticon matches are allowed */
			SkipHTML = 0x2				/** Skip emoticons within HTML */
		 };

	/**
	 * Use it to parse emoticons in a text.
	 * You don't need to use this for chat windows,
	 * There is a special class that abstract a chat view
	 * and uses emoticons parser.
	 * This function will use the selected emoticon theme.
	 * If nicks is provided, they will not be parsed if they 
	 * exist in message.
	 */
	static QString parseEmoticons( const QString &message, ParseMode = SkipHTML ) ;

	
	QString parse( const QString &message, ParseMode = SkipHTML );

	/**
	 * TokenType, a token might be an image ( emoticon ) or text.
	 */
	enum TokenType { Undefined, /** Undefined, for completeness only */
					 Image, 	/** Token contains a path to an image */
					 Text 		/** Token contains test */
				   };
	
	/**
	 * A token consists of a QString text which is either a regular text
	 * or a path to image depending on the type.
	 * If type is Image the text refers to an image path.
	 * If type is Text the text refers to a regular text.
	 */
	struct Token {
		Token() : type( Undefined ) {}
		Token( TokenType t, const QString &m ) : type( t ), text(m) {}
		Token( TokenType t, const QString &m, const QString &p, const QString &html )
		 : type( t ), text( m ), picPath( p ), picHTMLCode( html ) {}
		TokenType	type;
		QString		text;
		QString		picPath;
		QString		picHTMLCode;
	};
	

	/**
	 * Static function which will call tokenize
	 * @see tokenize( const QString& )
	 */
	static QValueList<Token> tokenizeEmoticons( const QString &message, ParseMode mode = DefaultParseMode );

	/**
	 * Tokenizes an message.
	 * For example;
	 * Assume :], (H), :-x are three emoticons.
	 * A text "(H)(H) foo bar john :] :-x" would be tokenized as follows (not strict):
	 * 1- /path/to/shades.png
	 * 2- /path/to/shades.png
	 * 3- " foo bar john "
	 * 4- /path/to/bat.png
	 * 5- " "
	 * 6- /path/to/kiss.png
	 *
	 * Strict tokenization (require spaces around emoticons):
	 * 1- "(H)(H) foo bar john "
	 * 2- /path/to/bat.png
	 * 3- " "
	 * 4- /path/to/kiss.png
	 * Note: quotation marks are used to emphasize white spaces.
	 * @param message is the message to tokenize
	 * @param mode is a bitmask of ParseMode enum
	 * @return a QValueList which consiste of ordered tokens of the text.
	 * @author Engin AYDOGAN < engin@bzzzt.biz >
	 * @since 23-03-05
	 */
	QValueList<Token> tokenize( const QString &message, uint mode = DefaultParseMode );
	
	/**
	 * Return all emoticons and the corresponding icon.
	 * (only one emoticon per image)
	 */
	QMap<QString, QStringList> emoticonAndPicList();


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
	
	/**
	 * uses the kopete's emoticons.xml  for the theme
	 * @see initEmoticons
	 */
	void initEmoticon_emoticonsxml( const QString & filename);
	
	/**
	 * uses the JEP-0038 xml description for the theme
	 * @see initEmoticons
	 */
	void initEmoticon_JEP0038( const QString & filename);
	
	/**
	 * sorts emoticons for convenient parsing, which yields greedy matching on
	 * matchText
	 */
	void sortEmoticons();


	struct Emoticon;
	struct EmoticonNode;
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
