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

#include "jt_xregister.h"

#include "xmpp_xmlcommon.h"

JT_XRegister::JT_XRegister(Task *parent):
JT_Register(parent)
{
}

bool JT_XRegister::take(const QDomElement &x)
{
	_iq = x;
	return JT_Register::take( x );
}

QDomElement JT_XRegister::iq() const
{
	return _iq;
}

void JT_XRegister::setXForm(const Form &frm, const XData &_form)
{
	JT_Register::setForm( frm );

	_iq = createIQ(doc(), "set", frm.jid().full(), id());
	QDomElement query = doc()->createElement("query");
	query.setAttribute("xmlns", "jabber:iq:register");
	_iq.appendChild(query);

	XData form( _form );
	form.setType( XData::Data_Submit );
	query.appendChild( form.toXml( doc() ) );
}

void JT_XRegister::onGo()
{
	if ( !_iq.isNull() )
		send( _iq );
	else
		JT_Register::onGo();
}

QDomElement JT_XRegister::xdataElement() const
{
	QDomNode n = queryTag(iq()).firstChild();
	for (; !n.isNull(); n = n.nextSibling()) {
		QDomElement i = n.toElement();
		if (i.isNull())
			continue;

		if (i.attribute("xmlns") == "jabber:x:data")
			return i;
	}

	return QDomElement();
}

#include "jt_xregister.moc"
