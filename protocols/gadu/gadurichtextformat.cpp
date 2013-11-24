// -*- Mode: c++-mode; c-basic-offset: 2; indent-tabs-mode: t; tab-width: 2; -*-
//
// Copyright (C) 2004 Grzegorz Jaskiewicz <gj at pointblue.com.pl>
//
// gadurichtextformat.cpp
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
// 02110-1301, USA.

#include "gadurichtextformat.h"

#include <kdebug.h>
#include <kopetemessage.h>

#include "gadusession.h"

#include <QtCore/QString>
#include <QtCore/QRegExp>

GaduRichTextFormat::GaduRichTextFormat()
{
}

GaduRichTextFormat::~GaduRichTextFormat()
{
}

QString
GaduRichTextFormat::convertToHtml( const QString& msg, unsigned int formats, void* formatStructure)
{
	QString tmp, nb;
	gg_msg_richtext_format *format;
	char *pointer = (char*) formatStructure;

	unsigned int i,j;
	int r, g, b;
	r = g = b = 0;
	bool opened = false;

	if ( formatStructure == NULL || formats == 0 ) {
		tmp = msg;
		escapeBody( tmp );
		return tmp;
	}

	for ( i = 0, j = 0 ; i < formats ; ) {
		format = (gg_msg_richtext_format*) pointer;
		unsigned int position = format->position;
		char font = format->font;
		QString style;

		if ( position < j || position > (unsigned int)msg.length() ) {
			break;
		}

		if ( font & GG_FONT_IMAGE ) {
			i += sizeof( gg_msg_richtext_image );
			pointer += sizeof( gg_msg_richtext_image );
			tmp += "<b>[this should be a picture, not yet implemented]</b>";
		}
		else {
			nb = msg.mid( j, position - j );
			tmp += escapeBody( nb );

			j = position;

			// add message bit between formating
			if ( opened ) {
				tmp += formatClosingTag("span");
				opened = false;
			}
			// set font attributes
			if ( font & GG_FONT_BOLD ) {
				style += (" font-weight:bold; ");
			}
			if ( font & GG_FONT_ITALIC ) {
				style += (" font-style:italic; ");
			}
			if ( font & GG_FONT_UNDERLINE ) {
				style += (" text-decoration:underline; ");
			}
			// add color
			if ( font & GG_FONT_COLOR ) {
				pointer += sizeof( gg_msg_richtext_format );
				i += sizeof( gg_msg_richtext_format );
				gg_msg_richtext_color *color = (gg_msg_richtext_color*)( pointer );
				r = (int)color->red;
				g = (int)color->green;
				b = (int)color->blue;
			}
			style += QString(" color: rgb( %1, %2, %3 ); ").arg( r ).arg( g ).arg( b );

			tmp += formatOpeningTag( QString::fromLatin1("span"), QString::fromLatin1("style=\"%1\"").arg( style ) );
			opened = true;

		}

		// advance to next structure in row
		pointer += sizeof( gg_msg_richtext_format );
		i += sizeof( gg_msg_richtext_format );
	}

	nb = msg.mid( j, msg.length() );
	tmp += escapeBody( nb );
	if ( opened ) {
		tmp += formatClosingTag("span");
	}

	return tmp;
}

QString
GaduRichTextFormat::formatOpeningTag( const QString& tag, const QString& attributes )
{
	QString res = '<' + tag;
	if(!attributes.isEmpty())
		res.append(' ' + attributes);
	return res + '>';
}

QString
GaduRichTextFormat::formatClosingTag( const QString& tag )
{
	return "</" + tag + '>';
}

// the initial idea stolen from IRC plugin
KGaduMessage*
GaduRichTextFormat::convertToGaduMessage( const Kopete::Message& message )
{
	QString htmlString = message.escapedBody();
	KGaduMessage* output = new KGaduMessage;
	rtcs.blue  = rtcs.green = rtcs.red = 0;
	color = QColor();
	int position = 0;

	rtf.resize( sizeof( gg_msg_richtext) );
	output->rtf.resize(0);

	// test first if there is any HTML formating in it
	if( htmlString.indexOf( QString::fromLatin1("</span") ) > -1 ) {
		QRegExp findTags( QString::fromLatin1("<span style=\"(.*)\">(.*)</span>") );
		findTags.setMinimal( true );
		int pos = 0;
		int lastpos = 0;

		while ( pos >= 0 ){
			pos = findTags.indexIn( htmlString );
			rtfs.font = 0;
			if ( pos != lastpos ) {
				QString tmp;
				if ( pos < 0 ) {
					tmp = htmlString.mid( lastpos );
				}
				else {
					tmp = htmlString.mid( lastpos, pos - lastpos );
				}
				if ( !tmp.isEmpty() ) {
					color.setRgb( 0, 0, 0 );
					if ( insertRtf( position ) == false ) {
						delete output;
						return NULL;
					}
					tmp = unescapeGaduMessage( tmp );
					output->message += tmp;
					position += tmp.length();
				}
			}

			if ( pos > -1 ) {
				QString styleHTML = findTags.cap(1);
				QString replacement = findTags.cap(2);
				QStringList styleAttrs = styleHTML.split( ';', QString::SkipEmptyParts );
				rtfs.font = 0;

				lastpos = pos + replacement.length();

				for( QStringList::Iterator attrPair = styleAttrs.begin(); attrPair != styleAttrs.end(); ++attrPair ) {
					QString attribute = (*attrPair).section(':',0,0);
					QString value = (*attrPair).section(':',1);
					parseAttributes( attribute, value );
				}

				if ( insertRtf( position ) == false ) {
					delete output;
					return NULL;
				}

				QString rep = QString("<span style=\"%1\">%2</span>" ).arg( styleHTML ).arg( replacement );
				htmlString.replace( findTags.pos( 0 ), rep.length(), replacement );

				replacement = unescapeGaduMessage( replacement );
				output->message += replacement;
				position += replacement.length();
			}

		}
		output->rtf = rtf;
		// this is sick, but that's the way libgadu is designed
		// here I am adding network header !, should sit in libgadu IMO
		header = (gg_msg_richtext*) output->rtf.data();
		header->length = output->rtf.size() - sizeof( gg_msg_richtext );
		header->flag = 2;
	}
	else {
		output->message = message.escapedBody();
		output->message = unescapeGaduMessage( output->message );
	}

	return output;

}

void
GaduRichTextFormat::parseAttributes( const QString attribute, const QString value )
{
	if( attribute == QString::fromLatin1("color") ) {
		color.setNamedColor( value );
	}
	if( attribute == QString::fromLatin1("font-weight") && value == QString::fromLatin1("600") ) {
		rtfs.font |= GG_FONT_BOLD;
	}
	if( attribute == QString::fromLatin1("text-decoration")  && value == QString::fromLatin1("underline") ) {
		rtfs.font |= GG_FONT_UNDERLINE ;
	}
	if( attribute == QString::fromLatin1("font-style")  && value == QString::fromLatin1("italic") ) {
		rtfs.font |= GG_FONT_ITALIC;
	}
}

QString
GaduRichTextFormat::unescapeGaduMessage( QString& ns )
{
	QString s;
	s = Kopete::Message::unescape( ns );
	s.replace( QString::fromAscii( "\n" ), QString::fromAscii( "\r\n" ) );
	return s;
}

bool
GaduRichTextFormat::insertRtf( uint position)
{
	if ( color != QColor( rtcs.red, rtcs.green, rtcs.blue ) ) {
		rtcs.red   = color.red();
		rtcs.green = color.green();
		rtcs.blue  = color.blue();
		rtfs.font |= GG_FONT_COLOR;
	}

	if ( rtfs.font ) {
		// append font description
		rtfs.position = position;
		uint csize = rtf.size();
		rtf.resize( csize + sizeof( gg_msg_richtext_format ) );
		memcpy( rtf.data()  + csize, &rtfs, sizeof( rtfs ) );
		// append color description, if color has changed
		if ( rtfs.font & GG_FONT_COLOR ) {
			csize = rtf.size();
			rtf.resize( csize + sizeof( gg_msg_richtext_color ) );
			memcpy( rtf.data() + csize, &rtcs, sizeof( rtcs ) );
		}
	}
	return true;
}

QString
GaduRichTextFormat::escapeBody( QString& input )
{
	input.replace( '<', QString::fromLatin1("&lt;") );
	input.replace( '>', QString::fromLatin1("&gt;") );
	input.replace( '\n', QString::fromLatin1( "<br />" ) );
	input.replace( '\t', QString::fromLatin1( "&nbsp;&nbsp;&nbsp;&nbsp;" ) );
	input.replace( QRegExp( QString::fromLatin1( "\\s\\s" ) ), QString::fromLatin1( " &nbsp;" ) );
	return input;
}
