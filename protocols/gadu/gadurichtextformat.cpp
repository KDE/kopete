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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.

#include <kopetenotifyclient.h>
#include <kdebug.h>
#include "gadurichtextformat.h"

#include <qstring.h>

#include <libgadu.h>

GaduRichTextFormat::GaduRichTextFormat()
{
}

GaduRichTextFormat::~GaduRichTextFormat()
{
}

QString
GaduRichTextFormat::convertToHtml( const QString& msg, unsigned int formats, void* formatStructure)
{
	if ( formatStructure == NULL ) {
		return msg;
	}
	if ( formats == 0 ) {
		return msg;
	}

	QString tmp;
	gg_msg_richtext_format *format;
	char *pointer = (char*) formatStructure;

	unsigned int i,j;
	int r, g, b;
	r = g = b = 0;
	bool opened = false;
	for ( i = 0, j = 0 ; i < formats ; i++ ) {
		format = (gg_msg_richtext_format*) pointer;
		unsigned int position = format->position;
		char font = format->font;
		QString style;

		// FIXME: there is something wrong with libgadu, besides richtext formating information, there is something attached at the end as well
		if ( position < j || position > msg.length() ) {
			break;
		}

		if ( font & GG_FONT_IMAGE ) {
			pointer += sizeof( gg_msg_richtext_image );
			tmp += "<b>[this should be a picture, not yet implemented]</b>";
		}
		else {
			tmp += msg.mid( j, position - j );
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
				gg_msg_richtext_color *color = (gg_msg_richtext_color*)( pointer );
				r = (int)color->red;
				g = (int)color->green;
				b = (int)color->blue;
			}
			style += QString::fromLatin1(" color: rgb( %1, %2, %3 ); ").arg( r ).arg( g ).arg( b );

			if ( !style.isEmpty() ) {
				opened = true;
				tmp += formatOpeningTag(QString::fromLatin1("span"), QString::fromLatin1("style=\"%1\"").arg( style ) );
			}

		}

		// advance to next structure in row
		pointer += sizeof( gg_msg_richtext_format );
	}

	tmp += msg.mid( j, msg.length() );
	if ( opened ) {
		tmp += formatClosingTag("span");
	}

	tmp.replace( '\n', "<br />" );
	tmp.replace( '\t', "&nbsp;&nbsp;&nbsp;&nbsp;" );

	return tmp;
}

QString
GaduRichTextFormat::formatOpeningTag( const QString& tag, const QString& attributes )
{
	QString res = "<" + tag;
	if(!attributes.isEmpty())
		res.append(" " + attributes);
	return res + ">";
}

QString
GaduRichTextFormat::formatClosingTag( const QString& tag )
{
	return "</" + tag + ">";
}
