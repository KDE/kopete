 /*
    Copyright (c) 2008 by Igor Janssen  <alaves17@gmail.com>

    Kopete    (c) 2008 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
 */

#include "jt_xsearch.h"

#include "xmpp_xmlcommon.h"

JT_XSearch::JT_XSearch(Task *parent):
JT_Search(parent)
{
}

bool JT_XSearch::take(const QDomElement &x)
{
	_iq = x;
	return JT_Search::take( x );
}

QDomElement JT_XSearch::iq() const
{
	return _iq;
}

void JT_XSearch::setForm(const Form &frm, const XData &_form)
{
	JT_Search::set( frm );

	_iq = createIQ(doc(), "set", frm.jid().full(), id());
	QDomElement query = doc()->createElement("query");
	query.setAttribute("xmlns", "jabber:iq:search");
	_iq.appendChild(query);

	XData form( _form );
	form.setType( XData::Data_Submit );
	query.appendChild( form.toXml( doc() ) );
}

void JT_XSearch::onGo()
{
	if ( !_iq.isNull() )
		send( _iq );
	else
		JT_Search::onGo();
}

#include "jt_xsearch.moc"
