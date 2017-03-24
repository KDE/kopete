/*
 * xmpp_xdata.cpp - a class for jabber:x:data forms
 * Copyright (C) 2003-2004  Michail Pishchagin
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301  USA
 *
 */

#include "xmpp_xdata.h"
#include "xmpp_xmlcommon.h"
#include "xmpp/jid/jid.h"

#include <QList>
#include <QSharedDataPointer>

using namespace XMPP;
using namespace XMLHelper;

// TODO: report, item

//----------------------------------------------------------------------------
// XData::Field
//----------------------------------------------------------------------------
XData::Field::Field() :
    _required(false)
{
}

XData::Field::~Field()
{
}

QString XData::Field::desc() const
{
	return _desc;
}

void XData::Field::setDesc(const QString &d)
{
	_desc = d;
}

XData::Field::OptionList XData::Field::options() const
{
	return _options;
}

void XData::Field::setOptions(XData::Field::OptionList o)
{
	_options = o;
}

XData::Field::MediaElement XData::Field::mediaElement() const
{
	return _mediaElement;
}

void XData::Field::setMediaElement(const XData::Field::MediaElement &el)
{
	_mediaElement = el;
}

bool XData::Field::required() const
{
	return _required;
}

void XData::Field::setRequired(bool r)
{
	_required = r;
}

QString XData::Field::label() const
{
	return _label;
}

void XData::Field::setLabel(const QString &l)
{
	_label = l;
}

QString XData::Field::var() const
{
	return _var;
}

void XData::Field::setVar(const QString &v)
{
	_var = v;
}

QStringList XData::Field::value() const
{
	return _value;
}

void XData::Field::setValue(const QStringList &v)
{
	_value = v;
}

XData::Field::Type XData::Field::type() const
{
	return _type;
}

void XData::Field::setType(XData::Field::Type t)
{
	_type = t;
}

bool XData::Field::isValid() const
{
	if ( _required && _value.isEmpty() )
		return false;

	if ( _type == Field_Hidden || _type == Field_Fixed) {
		return true;
	}
	if ( _type == Field_Boolean ) {
		if ( _value.count() != 1 )
			return false;

		QString str = _value.first();
		if ( str == QLatin1String("0") || str == QLatin1String("1") || str == QLatin1String("true") || str == QLatin1String("false") || str == QLatin1String("yes") || str == QLatin1String("no") )
			return true;
	}
	if ( _type == Field_TextSingle || _type == Field_TextPrivate ) {
		if ( _value.count() == 1 )
			return true;
	}
	if ( _type == Field_TextMulti ) {
		//no particular test. empty/required case already caught (see above)
		return true;
	}
	if ( _type == Field_ListSingle || _type == Field_ListMulti ) {
		//no particular test. empty/required case already caught (see above)
		return true;
	}
	if ( _type == Field_JidSingle ) {
		if ( _value.count() != 1 )
			return false;

		Jid j( _value.first() );
		return j.isValid();
	}
	if ( _type == Field_JidMulti ) {
		QStringList::ConstIterator it = _value.begin();
		bool allValid = true;
		for ( ; it != _value.end(); ++it) {
			Jid j(*it);
			if ( !j.isValid() ) {
				allValid = false;
				break;
			}
		}
		return allValid;
	}

	return false;
}

void XData::Field::fromXml(const QDomElement &e)
{
	if ( e.tagName() != QLatin1String("field") )
		return;

	_var = e.attribute(QStringLiteral("var"));
	_label = e.attribute(QStringLiteral("label"));

	QString type = e.attribute(QStringLiteral("type"));
	if ( type == QLatin1String("boolean") )
		_type = Field_Boolean;
	else if ( type == QLatin1String("fixed") )
		_type = Field_Fixed;
	else if ( type == QLatin1String("hidden") )
		_type = Field_Hidden;
	else if ( type == QLatin1String("jid-multi") )
		_type = Field_JidMulti;
	else if ( type == QLatin1String("jid-single") )
		_type = Field_JidSingle;
	else if ( type == QLatin1String("list-multi") )
		_type = Field_ListMulti;
	else if ( type == QLatin1String("list-single") )
		_type = Field_ListSingle;
	else if ( type == QLatin1String("text-multi") )
		_type = Field_TextMulti;
	else if ( type == QLatin1String("text-private") )
		_type = Field_TextPrivate;
	else
		_type = Field_TextSingle;

	_required = false;
	_desc     = QString::null;
	_options.clear();
	_value.clear();

	QDomNode n = e.firstChild();
	for ( ; !n.isNull(); n = n.nextSibling() ) {
		QDomElement i = n.toElement();
		if ( i.isNull() )
			continue;

		QString tag = i.tagName();
		if ( tag == QLatin1String("required") )
			_required = true;
		else if ( tag == QLatin1String("desc") )
			_desc = i.text().trimmed();
		else if ( tag == QLatin1String("option") ) {
			Option o;
			o.label = i.attribute(QStringLiteral("label"));
			o.value = subTagText(i, QStringLiteral("value"));
			_options.append(o);
		}
		else if ( tag == QLatin1String("value") ) {
			_value.append(i.text());
		}
		else if (tag == QLatin1String("media") && (i.namespaceURI() == QLatin1String("urn:xmpp:media-element")
				 || i.attribute(QStringLiteral("xmlns")) == QLatin1String("urn:xmpp:media-element"))) { // allow only one media element
			QSize s;
			if (i.hasAttribute(QStringLiteral("width"))) {
				s.setWidth(i.attribute(QStringLiteral("width")).toInt());
			}
			if (i.hasAttribute(QStringLiteral("height"))) {
				s.setHeight(i.attribute(QStringLiteral("height")).toInt());
			}
			_mediaElement.setMediaSize(s);
			for(QDomNode un = i.firstChild(); !un.isNull(); un = un.nextSibling()) {
				QDomElement uel = un.toElement();
				if(uel.isNull() || uel.tagName() != QLatin1String("uri")) {
					continue;
				}
				QStringList type = uel.attribute(QStringLiteral("type")).split(';');
				QHash<QString,QString> params;
				for (int i = 1; i < type.size(); ++i) {
					QStringList p = type.value(i).split('=');
					QString key = p[0].trimmed();
					if (!key.isEmpty()) {
						params[key] = p.value(1).trimmed();
					}
				}
				_mediaElement.append(type.value(0).trimmed(), uel.text(), params);
			}
		}
	}
}

QDomElement XData::Field::toXml(QDomDocument *doc, bool submitForm) const
{
	QDomElement f = doc->createElement(QStringLiteral("field"));

	// setting attributes...
	if ( !_var.isEmpty() )
		f.setAttribute(QStringLiteral("var"), _var);
	if ( !submitForm && !_label.isEmpty() )
		f.setAttribute(QStringLiteral("label"), _label);

	// now we're gonna get the 'type'
	QString type = QStringLiteral("text-single");
	if ( _type == Field_Boolean )
		type = QStringLiteral("boolean");
	else if ( _type == Field_Fixed )
		type = QStringLiteral("fixed");
	else if ( _type == Field_Hidden )
		type = QStringLiteral("hidden");
	else if ( _type == Field_JidMulti )
		type = QStringLiteral("jid-multi");
	else if ( _type == Field_JidSingle )
		type = QStringLiteral("jid-single");
	else if ( _type == Field_ListMulti )
		type = QStringLiteral("list-multi");
	else if ( _type == Field_ListSingle )
		type = QStringLiteral("list-single");
	else if ( _type == Field_TextMulti )
		type = QStringLiteral("text-multi");
	else if ( _type == Field_TextPrivate )
		type = QStringLiteral("text-private");

	f.setAttribute(QStringLiteral("type"), type);

	// now, setting nested tags...
	if ( !submitForm && _required )
		f.appendChild( emptyTag(doc, QStringLiteral("required")) );

	if ( !submitForm && !_desc.isEmpty() )
		f.appendChild( textTag(doc, QStringLiteral("desc"), _desc) );

	if ( !submitForm && !_options.isEmpty() ) {
		OptionList::ConstIterator it = _options.begin();
		for ( ; it != _options.end(); ++it) {
			QDomElement o = doc->createElement(QStringLiteral("option"));
			o.appendChild(textTag(doc, QStringLiteral("value"), (*it).value));
			if ( !(*it).label.isEmpty() )
				o.setAttribute(QStringLiteral("label"), (*it).label);
			f.appendChild(o);
		}
	}

	if ( !_value.isEmpty() ) {
		QStringList::ConstIterator it = _value.begin();
		for ( ; it != _value.end(); ++it)
			f.appendChild( textTag(doc, QStringLiteral("value"), *it) );
	}

	if ( !_mediaElement.isEmpty() ) {
		QDomElement media = doc->createElementNS(QStringLiteral("urn:xmpp:media-element"), QStringLiteral("media"));
		QSize s = _mediaElement.mediaSize();
		if (!s.isEmpty()) {
			media.setAttribute(QStringLiteral("width"), s.width());
			media.setAttribute(QStringLiteral("height"), s.height());
		}
		foreach(const MediaUri &uri, _mediaElement) {
			QDomElement uriEl = doc->createElement(QStringLiteral("uri"));
			QString type = uri.type;
			foreach (const QString &k, uri.params.keys()) {
				type += ";" + k + "=" + uri.params[k];
			}
			uriEl.setAttribute(QStringLiteral("type"), type);
			uriEl.appendChild(doc->createTextNode(uri.uri));
			media.appendChild(uriEl);
		}
		f.appendChild(media);
	}

	return f;
}

//----------------------------------------------------------------------------
// MediaElement
//----------------------------------------------------------------------------

void XData::Field::MediaElement::append(const QString &type, const QString &uri,
										QHash<QString,QString> params)
{
	XData::Field::MediaUri u;
	u.type = type;
	u.uri = uri;
	u.params = params;
	QList<XData::Field::MediaUri>::append(u);
}

void XData::Field::MediaElement::setMediaSize(const QSize &size)
{
	_size = size;
}

QSize XData::Field::MediaElement::mediaSize() const
{
	return _size;
}

bool XData::Field::MediaElement::checkSupport(const QStringList &wildcards)
{
	foreach (const XData::Field::MediaUri &uri, *this) {
		foreach (const QString &wildcard, wildcards) {
			if (QRegExp(wildcard, Qt::CaseSensitive, QRegExp::Wildcard)
					.exactMatch(uri.type)) {
				return true;
			}
		}
	}
	return false;
}

//----------------------------------------------------------------------------
// XData
//----------------------------------------------------------------------------

XData::XData()
{
	d = new Private;
	d->type = Data_Form;
}

QString XData::title() const
{
	return d->title;
}

void XData::setTitle(const QString &t)
{
	d->title = t;
}

QString XData::instructions() const
{
	return d->instructions;
}

void XData::setInstructions(const QString &i)
{
	d->instructions = i;
}

XData::Type XData::type() const
{
	return d->type;
}

void XData::setType(Type t)
{
	d->type = t;
}

QString XData::registrarType() const
{
	return d->registrarType;
}

XData::FieldList XData::fields() const
{
	return d->fields;
}

XData::Field XData::getField(const QString &var) const
{
	if ( !d->fields.isEmpty() ) {
		FieldList::ConstIterator it = d->fields.begin();
		for ( ; it != d->fields.end(); ++it) {
			Field f = *it;
			if (f.isValid() && f.var() == var)
				return f;
		}
	}
	return Field();
}

XData::Field &XData::fieldRef(const QString &var)
{
	FieldList::Iterator it = d->fields.begin();
	for ( ; it != d->fields.end(); ++it) {
		if (it->isValid() && it->var() == var)
			break;
	}
	return *it;
}

void XData::setFields(const FieldList &fl)
{
	d->fields = fl;
	foreach (const Field &f, fl) {
		if (f.type() == Field::Field_Hidden && f.var() == QLatin1String("FORM_TYPE")) {
			d->registrarType = f.value().value(0);
		}
	}
}

void XData::fromXml(const QDomElement &e)
{
	if ( (e.attribute(QStringLiteral("xmlns")) != QLatin1String("jabber:x:data")) && (e.namespaceURI() != QLatin1String("jabber:x:data")) )
		return;

	QString type = e.attribute(QStringLiteral("type"));
	if ( type == QLatin1String("result") )
		d->type = Data_Result;
	else if ( type == QLatin1String("submit") )
		d->type = Data_Submit;
	else if ( type == QLatin1String("cancel") )
		d->type = Data_Cancel;
	else
		d->type = Data_Form;

	d->title        = subTagText(e, QStringLiteral("title"));
	d->instructions = subTagText(e, QStringLiteral("instructions"));

	d->fields.clear();

	QDomNode n = e.firstChild();
	for ( ; !n.isNull(); n = n.nextSibling() ) {
		QDomElement i = n.toElement();
		if ( i.isNull() )
			continue;

		if ( i.tagName() == QLatin1String("field") ) {
			Field f;
			f.fromXml(i);
			d->fields.append(f);
			if (f.type() == Field::Field_Hidden && f.var() == QLatin1String("FORM_TYPE")) {
				d->registrarType = f.value().value(0);
			}
		}
		else if ( i.tagName() == QLatin1String("reported") ) {
			d->report.clear();
			d->reportItems.clear();

			QDomNode nn = i.firstChild();
			for ( ; !nn.isNull(); nn = nn.nextSibling() ) {
				QDomElement ii = nn.toElement();
				if ( ii.isNull() )
					continue;

				if ( ii.tagName() == QLatin1String("field") ) {
					d->report.append( ReportField( ii.attribute(QStringLiteral("label")), ii.attribute(QStringLiteral("var")) ) );
				}
			}
		}
		else if ( i.tagName() == QLatin1String("item") ) {
			ReportItem item;

			QDomNode nn = i.firstChild();
			for ( ; !nn.isNull(); nn = nn.nextSibling() ) {
				QDomElement ii = nn.toElement();
				if ( ii.isNull() )
					continue;

				if ( ii.tagName() == QLatin1String("field") ) {
					item.insert(ii.attribute(QStringLiteral("var")), subTagText(ii, QStringLiteral("value")));
				}
			}

			d->reportItems.append( item );
		}
	}
}

QDomElement XData::toXml(QDomDocument *doc, bool submitForm) const
{
	QDomElement x = doc->createElementNS(QStringLiteral("jabber:x:data"), QStringLiteral("x"));
	x.setAttribute(QStringLiteral("xmlns"), QStringLiteral("jabber:x:data"));

	QString type = QStringLiteral("form");
	if ( d->type == Data_Result )
		type = QStringLiteral("result");
	else if ( d->type == Data_Submit )
		type = QStringLiteral("submit");
	else if ( d->type == Data_Cancel )
		type = QStringLiteral("cancel");

	x.setAttribute(QStringLiteral("type"), type);

	if ( !submitForm && !d->title.isEmpty() )
		x.appendChild( textTag(doc, QStringLiteral("title"), d->title) );
	if ( !submitForm && !d->instructions.isEmpty() )
		x.appendChild( textTag(doc, QStringLiteral("instructions"), d->instructions) );

	if ( !d->fields.isEmpty() ) {
		FieldList::ConstIterator it = d->fields.begin();
		for ( ; it != d->fields.end(); ++it) {
			Field f = *it;
			if ( !(submitForm && f.var().isEmpty()) )
				x.appendChild( f.toXml(doc, submitForm) );
		}
	}

	return x;
}

const QList<XData::ReportField> &XData::report() const
{
	return d->report;
}

const QList<XData::ReportItem> &XData::reportItems() const
{
	return d->reportItems;
}

bool XData::isValid() const
{
	foreach(Field f, d->fields) {
		if (!f.isValid())
			return false;
	}
	return true;
}
