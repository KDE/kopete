// -*- Mode: c++-mode; c-basic-offset: 2; indent-tabs-mode: t; tab-width: 2; -*-
//
// Copyright (C) 2004 Grzegorz Jaskiewicz <gj at pointblue.com.pl>
//
// gadurichtextformat.h
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


#ifndef GADURTF_H
#define GADURTF_H

#include "libgadu/libgadu.h"

class Qstring;
class KopeteMessage;
class KGaduMessage;

class GaduRichTextFormat {
public:
	GaduRichTextFormat();
	~GaduRichTextFormat();
	QString convertToHtml( const QString&, unsigned int, void* );
	KGaduMessage* convertToGaduMessage( const KopeteMessage& );

private:
	QString formatOpeningTag( const QString& , const QString& = QString::null );
	QString formatClosingTag( const QString& );
	bool insertRtf( uint );
	QString unescapeGaduMessage( QString& );
	void parseAttributes( QString, QString );

	QColor 			color;
	gg_msg_richtext_format	rtfs;
	gg_msg_richtext_color	rtcs;
	gg_msg_richtext*	header;
	QByteArray		rtf;

};
#endif
