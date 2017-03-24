/*
 * xmpp_vcard.cpp - classes for handling vCards
 * Copyright (C) 2003  Michail Pishchagin
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 */

#include "xmpp_vcard.h"

#include <qdom.h>
#include <QDateTime>
#include <QImage> // needed for image format recognition
#include <QBuffer>
#include <QImageReader>
#include <QImageWriter>
#include <QtCrypto>
#include <QtDebug>

#include "xmpp_xmlcommon.h"

using namespace XMLHelper;

//----------------------------------------------------------------------------
// VCard
//----------------------------------------------------------------------------
QString image2type(const QByteArray &ba)
{
	QBuffer buf;
	buf.setData(ba);
	buf.open(QIODevice::ReadOnly);
	QString format = QImageReader::imageFormat( &buf );

	// TODO: add more formats
	if ( format.toUpper() == QLatin1String("PNG") || format == QLatin1String("PsiPNG") )
		return QStringLiteral("image/png");
	if ( format.toUpper() == QLatin1String("MNG") )
		return QStringLiteral("video/x-mng");
	if ( format.toUpper() == QLatin1String("GIF") )
		return QStringLiteral("image/gif");
	if ( format.toUpper() == QLatin1String("BMP") )
		return QStringLiteral("image/bmp");
	if ( format.toUpper() == QLatin1String("XPM") )
		return QStringLiteral("image/x-xpm");
	if ( format.toUpper() == QLatin1String("SVG") )
		return QStringLiteral("image/svg+xml");
	if ( format.toUpper() == QLatin1String("JPEG") )
		return QStringLiteral("image/jpeg");

	qWarning() << QStringLiteral("WARNING! VCard::image2type: unknown format = '%1'").arg(format.isNull() ? QStringLiteral("UNKNOWN") : format);

	return QStringLiteral("image/unknown");
}

namespace XMPP {

// Long lines of encoded binary data SHOULD BE folded to 75 characters using the folding method defined in [MIME-DIR].
static QString foldString(const QString &s)
{
	QString ret;

	for (int i = 0; i < (int)s.length(); i++) {
		if ( !(i % 75) )
			ret += '\n';
		ret += s[i];
	}

	return ret;
}

class VCardPrivate : public QSharedData
{
public:
	VCardPrivate();
	~VCardPrivate();

	// do we need copy constructor?
	//VCardPrivate(const VCardPrivate &other) :
	//    QSharedData(other), version(other.version), fullName(other.fullName) { qDebug("Copy VCardPrivate"); }

	QString version;
	QString fullName;
	QString familyName, givenName, middleName, prefixName, suffixName;
	QString nickName;

	QByteArray photo;
	QString photoURI;

	QString bday;
	VCard::AddressList addressList;
	VCard::LabelList labelList;
	VCard::PhoneList phoneList;
	VCard::EmailList emailList;
	QString jid;
	QString mailer;
	QString timezone;
	VCard::Geo geo;
	QString title;
	QString role;

	QByteArray logo;
	QString logoURI;

	QSharedPointer<VCard> agent;
	QString agentURI;

	VCard::Org org;
	QStringList categories;
	QString note;
	QString prodId;
	QString rev;
	QString sortString;

	QByteArray sound;
	QString soundURI, soundPhonetic;

	QString uid;
	QString url;
	QString desc;
	VCard::PrivacyClass privacyClass;
	QByteArray key;

	bool isEmpty() const;
};

VCardPrivate::VCardPrivate()
{
	privacyClass = VCard::pcNone;
}

VCardPrivate::~VCardPrivate()
{

}

bool VCardPrivate::isEmpty() const
{
	if (	!version.isEmpty() ||
		!fullName.isEmpty() ||
		!familyName.isEmpty() || !givenName.isEmpty() || !middleName.isEmpty() || !prefixName.isEmpty() || !suffixName.isEmpty() ||
		!nickName.isEmpty() ||
		!photo.isEmpty() || !photoURI.isEmpty() ||
		!bday.isEmpty() ||
		!addressList.isEmpty() ||
		!labelList.isEmpty() ||
		!phoneList.isEmpty() ||
		!emailList.isEmpty() ||
		!jid.isEmpty() ||
		!mailer.isEmpty() ||
		!timezone.isEmpty() ||
		!geo.lat.isEmpty() || !geo.lon.isEmpty() ||
		!title.isEmpty() ||
		!role.isEmpty() ||
		!logo.isEmpty() || !logoURI.isEmpty() ||
		(agent && !agent->isEmpty()) || !agentURI.isEmpty() ||
		!org.name.isEmpty() || !org.unit.isEmpty() ||
		!categories.isEmpty() ||
		!note.isEmpty() ||
		!prodId.isEmpty() ||
		!rev.isEmpty() ||
		!sortString.isEmpty() ||
		!sound.isEmpty() || !soundURI.isEmpty() || !soundPhonetic.isEmpty() ||
		!uid.isEmpty() ||
		!url.isEmpty() ||
		!desc.isEmpty() ||
		(privacyClass != VCard::pcNone) ||
		!key.isEmpty() )
	{
		return false;
	}
	return true;
}

VCard::VCard()
{
}

VCard::VCard(const VCard &from) :
    d(from.d)
{
}

VCard & VCard::operator=(const VCard &from)
{
	d = from.d;
	return *this;
}

VCard::~VCard()
{
}

QDomElement VCard::toXml(QDomDocument *doc) const
{
	QDomElement v = doc->createElement(QStringLiteral("vCard"));
	v.setAttribute(QStringLiteral("xmlns"), QStringLiteral("vcard-temp"));

	if ( !d->version.isEmpty() )
		v.appendChild( textTag(doc, QStringLiteral("VERSION"),	d->version) );
	if ( !d->fullName.isEmpty() )
		v.appendChild( textTag(doc, QStringLiteral("FN"),	d->fullName) );

	if ( !d->familyName.isEmpty() || !d->givenName.isEmpty() || !d->middleName.isEmpty() ||
	     !d->prefixName.isEmpty() || !d->suffixName.isEmpty() ) {
		QDomElement w = doc->createElement(QStringLiteral("N"));

		if ( !d->familyName.isEmpty() )
			w.appendChild( textTag(doc, QStringLiteral("FAMILY"),	d->familyName) );
		if ( !d->givenName.isEmpty() )
			w.appendChild( textTag(doc, QStringLiteral("GIVEN"),	d->givenName) );
		if ( !d->middleName.isEmpty() )
			w.appendChild( textTag(doc, QStringLiteral("MIDDLE"),	d->middleName) );
		if ( !d->prefixName.isEmpty() )
			w.appendChild( textTag(doc, QStringLiteral("PREFIX"),	d->prefixName) );
		if ( !d->suffixName.isEmpty() )
			w.appendChild( textTag(doc, QStringLiteral("SUFFIX"),	d->suffixName) );

		v.appendChild(w);
	}

	if ( !d->nickName.isEmpty() )
		v.appendChild( textTag(doc, QStringLiteral("NICKNAME"),	d->nickName) );

	if ( !d->photo.isEmpty() || !d->photoURI.isEmpty() ) {
		QDomElement w = doc->createElement(QStringLiteral("PHOTO"));

		if ( !d->photo.isEmpty() ) {
			w.appendChild( textTag(doc, QStringLiteral("TYPE"),	image2type(d->photo)) );
			w.appendChild( textTag(doc, QStringLiteral("BINVAL"),	foldString( QCA::Base64().arrayToString(d->photo)) ) );
		}
		else if ( !d->photoURI.isEmpty() )
			w.appendChild( textTag(doc, QStringLiteral("EXTVAL"),	d->photoURI) );

		v.appendChild(w);
	}

	if ( !d->bday.isEmpty() )
		v.appendChild( textTag(doc, QStringLiteral("BDAY"),	d->bday) );

	if ( !d->addressList.isEmpty() ) {
		AddressList::ConstIterator it = d->addressList.constBegin();
		for ( ; it != d->addressList.end(); ++it ) {
			QDomElement w = doc->createElement(QStringLiteral("ADR"));
			const Address &a = *it;

			if ( a.home )
				w.appendChild( emptyTag(doc, QStringLiteral("HOME")) );
			if ( a.work )
				w.appendChild( emptyTag(doc, QStringLiteral("WORK")) );
			if ( a.postal )
				w.appendChild( emptyTag(doc, QStringLiteral("POSTAL")) );
			if ( a.parcel )
				w.appendChild( emptyTag(doc, QStringLiteral("PARCEL")) );
			if ( a.dom )
				w.appendChild( emptyTag(doc, QStringLiteral("DOM")) );
			if ( a.intl )
				w.appendChild( emptyTag(doc, QStringLiteral("INTL")) );
			if ( a.pref )
				w.appendChild( emptyTag(doc, QStringLiteral("PREF")) );

			if ( !a.pobox.isEmpty() )
				w.appendChild( textTag(doc, QStringLiteral("POBOX"),	a.pobox) );
			if ( !a.extaddr.isEmpty() )
				w.appendChild( textTag(doc, QStringLiteral("EXTADR"),	a.extaddr) );
			if ( !a.street.isEmpty() )
				w.appendChild( textTag(doc, QStringLiteral("STREET"),	a.street) );
			if ( !a.locality.isEmpty() )
				w.appendChild( textTag(doc, QStringLiteral("LOCALITY"),	a.locality) );
			if ( !a.region.isEmpty() )
				w.appendChild( textTag(doc, QStringLiteral("REGION"),	a.region) );
			if ( !a.pcode.isEmpty() )
				w.appendChild( textTag(doc, QStringLiteral("PCODE"),	a.pcode) );
			if ( !a.country.isEmpty() )
				w.appendChild( textTag(doc, QStringLiteral("CTRY"),	a.country) );

			v.appendChild(w);
		}
	}

	if ( !d->labelList.isEmpty() ) {
		LabelList::ConstIterator it = d->labelList.constBegin();
		for ( ; it != d->labelList.end(); ++it ) {
			QDomElement w = doc->createElement(QStringLiteral("LABEL"));
			const Label &l = *it;

			if ( l.home )
				w.appendChild( emptyTag(doc, QStringLiteral("HOME")) );
			if ( l.work )
				w.appendChild( emptyTag(doc, QStringLiteral("WORK")) );
			if ( l.postal )
				w.appendChild( emptyTag(doc, QStringLiteral("POSTAL")) );
			if ( l.parcel )
				w.appendChild( emptyTag(doc, QStringLiteral("PARCEL")) );
			if ( l.dom )
				w.appendChild( emptyTag(doc, QStringLiteral("DOM")) );
			if ( l.intl )
				w.appendChild( emptyTag(doc, QStringLiteral("INTL")) );
			if ( l.pref )
				w.appendChild( emptyTag(doc, QStringLiteral("PREF")) );

			if ( !l.lines.isEmpty() ) {
				QStringList::ConstIterator it = l.lines.constBegin();
				for ( ; it != l.lines.end(); ++it )
					w.appendChild( textTag(doc, QStringLiteral("LINE"), *it) );
			}

			v.appendChild(w);
		}
	}

	if ( !d->phoneList.isEmpty() ) {
		PhoneList::ConstIterator it = d->phoneList.constBegin();
		for ( ; it != d->phoneList.end(); ++it ) {
			QDomElement w = doc->createElement(QStringLiteral("TEL"));
			const Phone &p = *it;

			if ( p.home )
				w.appendChild( emptyTag(doc, QStringLiteral("HOME")) );
			if ( p.work )
				w.appendChild( emptyTag(doc, QStringLiteral("WORK")) );
			if ( p.voice )
				w.appendChild( emptyTag(doc, QStringLiteral("VOICE")) );
			if ( p.fax )
				w.appendChild( emptyTag(doc, QStringLiteral("FAX")) );
			if ( p.pager )
				w.appendChild( emptyTag(doc, QStringLiteral("PAGER")) );
			if ( p.msg )
				w.appendChild( emptyTag(doc, QStringLiteral("MSG")) );
			if ( p.cell )
				w.appendChild( emptyTag(doc, QStringLiteral("CELL")) );
			if ( p.video )
				w.appendChild( emptyTag(doc, QStringLiteral("VIDEO")) );
			if ( p.bbs )
				w.appendChild( emptyTag(doc, QStringLiteral("BBS")) );
			if ( p.modem )
				w.appendChild( emptyTag(doc, QStringLiteral("MODEM")) );
			if ( p.isdn )
				w.appendChild( emptyTag(doc, QStringLiteral("ISDN")) );
			if ( p.pcs )
				w.appendChild( emptyTag(doc, QStringLiteral("PCS")) );
			if ( p.pref )
				w.appendChild( emptyTag(doc, QStringLiteral("PREF")) );

			if ( !p.number.isEmpty() )
				w.appendChild( textTag(doc, QStringLiteral("NUMBER"),	p.number) );

			v.appendChild(w);
		}
	}

	if ( !d->emailList.isEmpty() ) {
		EmailList::ConstIterator it = d->emailList.constBegin();
		for ( ; it != d->emailList.end(); ++it ) {
			QDomElement w = doc->createElement(QStringLiteral("EMAIL"));
			const Email &e = *it;

			if ( e.home )
				w.appendChild( emptyTag(doc, QStringLiteral("HOME")) );
			if ( e.work )
				w.appendChild( emptyTag(doc, QStringLiteral("WORK")) );
			if ( e.internet )
				w.appendChild( emptyTag(doc, QStringLiteral("INTERNET")) );
			if ( e.x400 )
				w.appendChild( emptyTag(doc, QStringLiteral("X400")) );

			if ( !e.userid.isEmpty() )
				w.appendChild( textTag(doc, QStringLiteral("USERID"),	e.userid) );

			v.appendChild(w);
		}
	}

	if ( !d->jid.isEmpty() )
		v.appendChild( textTag(doc, QStringLiteral("JABBERID"),	d->jid) );
	if ( !d->mailer.isEmpty() )
		v.appendChild( textTag(doc, QStringLiteral("MAILER"),	d->mailer) );
	if ( !d->timezone.isEmpty() )
		v.appendChild( textTag(doc, QStringLiteral("TZ"),	d->timezone) );

	if ( !d->geo.lat.isEmpty() || !d->geo.lon.isEmpty() ) {
		QDomElement w = doc->createElement(QStringLiteral("GEO"));

		if ( !d->geo.lat.isEmpty() )
			w.appendChild( textTag(doc, QStringLiteral("LAT"),	d->geo.lat) );
		if ( !d->geo.lon.isEmpty() )
			w.appendChild( textTag(doc, QStringLiteral("LON"),	d->geo.lon));

		v.appendChild(w);
	}

	if ( !d->title.isEmpty() )
		v.appendChild( textTag(doc, QStringLiteral("TITLE"),	d->title) );
	if ( !d->role.isEmpty() )
		v.appendChild( textTag(doc, QStringLiteral("ROLE"),	d->role) );

	if ( !d->logo.isEmpty() || !d->logoURI.isEmpty() ) {
		QDomElement w = doc->createElement(QStringLiteral("LOGO"));

		if ( !d->logo.isEmpty() ) {
			w.appendChild( textTag(doc, QStringLiteral("TYPE"),	image2type(d->logo)) );
			w.appendChild( textTag(doc, QStringLiteral("BINVAL"),	foldString( QCA::Base64().arrayToString(d->logo)) ) );
		}
		else if ( !d->logoURI.isEmpty() )
			w.appendChild( textTag(doc, QStringLiteral("EXTVAL"),	d->logoURI) );

		v.appendChild(w);
	}

	if ( !d->agentURI.isEmpty() || (d->agent && d->agent->isEmpty()) ) {
		QDomElement w = doc->createElement(QStringLiteral("AGENT"));

		if ( d->agent && !d->agent->isEmpty() )
			w.appendChild( d->agent->toXml(doc) );
		else if ( !d->agentURI.isEmpty() )
			w.appendChild( textTag(doc, QStringLiteral("EXTVAL"),	d->agentURI) );

		v.appendChild(w);
	}

	if ( !d->org.name.isEmpty() || !d->org.unit.isEmpty() ) {
		QDomElement w = doc->createElement(QStringLiteral("ORG"));

		if ( !d->org.name.isEmpty() )
			w.appendChild( textTag(doc, QStringLiteral("ORGNAME"),	d->org.name) );

		if ( !d->org.unit.isEmpty() ) {
			QStringList::ConstIterator it = d->org.unit.constBegin();
			for ( ; it != d->org.unit.end(); ++it )
				w.appendChild( textTag(doc, QStringLiteral("ORGUNIT"),	*it) );
		}

		v.appendChild(w);
	}

	if ( !d->categories.isEmpty() ) {
		QDomElement w = doc->createElement(QStringLiteral("CATEGORIES"));

		QStringList::ConstIterator it = d->categories.constBegin();
		for ( ; it != d->categories.end(); ++it )
			w.appendChild( textTag(doc, QStringLiteral("KEYWORD"), *it) );

		v.appendChild(w);
	}

	if ( !d->note.isEmpty() )
		v.appendChild( textTag(doc, QStringLiteral("NOTE"),	d->note) );
	if ( !d->prodId.isEmpty() )
		v.appendChild( textTag(doc, QStringLiteral("PRODID"),	d->prodId) );
	if ( !d->rev.isEmpty() )
		v.appendChild( textTag(doc, QStringLiteral("REV"),	d->rev) );
	if ( !d->sortString.isEmpty() )
		v.appendChild( textTag(doc, QStringLiteral("SORT-STRING"),	d->sortString) );

	if ( !d->sound.isEmpty() || !d->soundURI.isEmpty() || !d->soundPhonetic.isEmpty() ) {
		QDomElement w = doc->createElement(QStringLiteral("SOUND"));

		if ( !d->sound.isEmpty() )
			w.appendChild( textTag(doc, QStringLiteral("BINVAL"),	foldString( QCA::Base64().arrayToString(d->sound)) ) );
		else if ( !d->soundURI.isEmpty() )
			w.appendChild( textTag(doc, QStringLiteral("EXTVAL"),	d->soundURI) );
		else if ( !d->soundPhonetic.isEmpty() )
			w.appendChild( textTag(doc, QStringLiteral("PHONETIC"),	d->soundPhonetic) );

		v.appendChild(w);
	}

	if ( !d->uid.isEmpty() )
		v.appendChild( textTag(doc, QStringLiteral("UID"),	d->uid) );
	if ( !d->url.isEmpty() )
		v.appendChild( textTag(doc, QStringLiteral("URL"),	d->url) );
	if ( !d->desc.isEmpty() )
		v.appendChild( textTag(doc, QStringLiteral("DESC"),	d->desc) );

	if ( d->privacyClass != pcNone ) {
		QDomElement w = doc->createElement(QStringLiteral("CLASS"));

		if ( d->privacyClass == pcPublic )
			w.appendChild( emptyTag(doc, QStringLiteral("PUBLIC")) );
		else if ( d->privacyClass == pcPrivate )
			w.appendChild( emptyTag(doc, QStringLiteral("PRIVATE")) );
		else if ( d->privacyClass == pcConfidential )
			w.appendChild( emptyTag(doc, QStringLiteral("CONFIDENTIAL")) );

		v.appendChild(w);
	}

	if ( !d->key.isEmpty() ) {
		QDomElement w = doc->createElement(QStringLiteral("KEY"));

		// TODO: Justin, please check out this code
		w.appendChild( textTag(doc, QStringLiteral("TYPE"), QStringLiteral("text/plain"))); // FIXME
		w.appendChild( textTag(doc, QStringLiteral("CRED"), QString::fromUtf8(d->key)) ); // FIXME

		v.appendChild(w);
	}

	return v;
}

VCard VCard::fromXml(const QDomElement &q)
{
	if ( q.tagName().toUpper() != QLatin1String("VCARD") )
		return VCard();

	VCard v;
	v.d = new VCardPrivate;

	QDomNode n = q.firstChild();
	for ( ; !n.isNull(); n = n.nextSibling() ) {
		QDomElement i = n.toElement();
		if ( i.isNull() )
			continue;

		QString tag = i.tagName().toUpper();

		QDomElement e;

		if ( tag == QLatin1String("VERSION") )
			v.d->version = i.text().trimmed();
		else if ( tag == QLatin1String("FN") )
			v.d->fullName = i.text().trimmed();
		else if ( tag == QLatin1String("N") ) {
			v.d->familyName = subTagText(i, QStringLiteral("FAMILY"));
			v.d->givenName  = subTagText(i, QStringLiteral("GIVEN"));
			v.d->middleName = subTagText(i, QStringLiteral("MIDDLE"));
			v.d->prefixName = subTagText(i, QStringLiteral("PREFIX"));
			v.d->suffixName = subTagText(i, QStringLiteral("SUFFIX"));
		}
		else if ( tag == QLatin1String("NICKNAME") )
			v.d->nickName = i.text().trimmed();
		else if ( tag == QLatin1String("PHOTO") ) {
			v.d->photo = QCA::Base64().stringToArray(subTagText(i, QStringLiteral("BINVAL")).replace(QRegExp("[\r\n]+"),QLatin1String(""))).toByteArray();
			v.d->photoURI = subTagText(i, QStringLiteral("EXTVAL"));
		}
		else if ( tag == QLatin1String("BDAY") )
			v.d->bday = i.text().trimmed();
		else if ( tag == QLatin1String("ADR") ) {
			Address a;

			a.home   = hasSubTag(i, QStringLiteral("HOME"));
			a.work   = hasSubTag(i, QStringLiteral("WORK"));
			a.postal = hasSubTag(i, QStringLiteral("POSTAL"));
			a.parcel = hasSubTag(i, QStringLiteral("PARCEL"));
			a.dom    = hasSubTag(i, QStringLiteral("DOM"));
			a.intl   = hasSubTag(i, QStringLiteral("INTL"));
			a.pref   = hasSubTag(i, QStringLiteral("PREF"));

			a.pobox    = subTagText(i, QStringLiteral("POBOX"));
			a.extaddr  = subTagText(i, QStringLiteral("EXTADR"));
			a.street   = subTagText(i, QStringLiteral("STREET"));
			a.locality = subTagText(i, QStringLiteral("LOCALITY"));
			a.region   = subTagText(i, QStringLiteral("REGION"));
			a.pcode    = subTagText(i, QStringLiteral("PCODE"));
			a.country  = subTagText(i, QStringLiteral("CTRY"));

			if ( a.country.isEmpty() ) // FIXME: Workaround for Psi prior to 0.9
				if ( hasSubTag(i, QStringLiteral("COUNTRY")) )
					a.country = subTagText(i, QStringLiteral("COUNTRY"));

			if ( a.extaddr.isEmpty() ) // FIXME: Workaround for Psi prior to 0.9
				if ( hasSubTag(i, QStringLiteral("EXTADD")) )
					a.extaddr = subTagText(i, QStringLiteral("EXTADD"));

			v.d->addressList.append ( a );
		}
		else if ( tag == QLatin1String("LABEL") ) {
			Label l;

			l.home   = hasSubTag(i, QStringLiteral("HOME"));
			l.work   = hasSubTag(i, QStringLiteral("WORK"));
			l.postal = hasSubTag(i, QStringLiteral("POSTAL"));
			l.parcel = hasSubTag(i, QStringLiteral("PARCEL"));
			l.dom    = hasSubTag(i, QStringLiteral("DOM"));
			l.intl   = hasSubTag(i, QStringLiteral("INTL"));
			l.pref   = hasSubTag(i, QStringLiteral("PREF"));

			QDomNode nn = i.firstChild();
			for ( ; !nn.isNull(); nn = nn.nextSibling() ) {
				QDomElement ii = nn.toElement();
				if ( ii.isNull() )
					continue;

				if ( ii.tagName().toUpper() == QLatin1String("LINE") )
					l.lines.append ( ii.text().trimmed() );
			}

			v.d->labelList.append ( l );
		}
		else if ( tag == QLatin1String("TEL") ) {
			Phone p;

			p.home  = hasSubTag(i, QStringLiteral("HOME"));
			p.work  = hasSubTag(i, QStringLiteral("WORK"));
			p.voice = hasSubTag(i, QStringLiteral("VOICE"));
			p.fax   = hasSubTag(i, QStringLiteral("FAX"));
			p.pager = hasSubTag(i, QStringLiteral("PAGER"));
			p.msg   = hasSubTag(i, QStringLiteral("MSG"));
			p.cell  = hasSubTag(i, QStringLiteral("CELL"));
			p.video = hasSubTag(i, QStringLiteral("VIDEO"));
			p.bbs   = hasSubTag(i, QStringLiteral("BBS"));
			p.modem = hasSubTag(i, QStringLiteral("MODEM"));
			p.isdn  = hasSubTag(i, QStringLiteral("ISDN"));
			p.pcs   = hasSubTag(i, QStringLiteral("PCS"));
			p.pref  = hasSubTag(i, QStringLiteral("PREF"));

			p.number = subTagText(i, QStringLiteral("NUMBER"));

			if ( p.number.isEmpty() ) // FIXME: Workaround for Psi prior to 0.9
				if ( hasSubTag(i, QStringLiteral("VOICE")) )
					p.number = subTagText(i, QStringLiteral("VOICE"));

			v.d->phoneList.append ( p );
		}
		else if ( tag == QLatin1String("EMAIL") ) {
			Email m;

			m.home     = hasSubTag(i, QStringLiteral("HOME"));
			m.work     = hasSubTag(i, QStringLiteral("WORK"));
			m.internet = hasSubTag(i, QStringLiteral("INTERNET"));
			m.x400     = hasSubTag(i, QStringLiteral("X400"));

			m.userid = subTagText(i, QStringLiteral("USERID"));

			if ( m.userid.isEmpty() ) // FIXME: Workaround for Psi prior to 0.9
				if ( !i.text().isEmpty() )
					m.userid = i.text().trimmed();

			v.d->emailList.append ( m );
		}
		else if ( tag == QLatin1String("JABBERID") )
			v.d->jid = i.text().trimmed();
		else if ( tag == QLatin1String("MAILER") )
			v.d->mailer = i.text().trimmed();
		else if ( tag == QLatin1String("TZ") )
			v.d->timezone = i.text().trimmed();
		else if ( tag == QLatin1String("GEO") ) {
			v.d->geo.lat = subTagText(i, QStringLiteral("LAT"));
			v.d->geo.lon = subTagText(i, QStringLiteral("LON"));
		}
		else if ( tag == QLatin1String("TITLE") )
			v.d->title = i.text().trimmed();
		else if ( tag == QLatin1String("ROLE") )
			v.d->role = i.text().trimmed();
		else if ( tag == QLatin1String("LOGO") ) {
			v.d->logo = QCA::Base64().stringToArray( subTagText(i, QStringLiteral("BINVAL")).replace(QLatin1String("\n"),QLatin1String("")) ).toByteArray();
			v.d->logoURI = subTagText(i, QStringLiteral("EXTVAL"));
		}
		else if ( tag == QLatin1String("AGENT") ) {
			e = i.firstChildElement(QStringLiteral("VCARD"));
			if ( !e.isNull() ) {
				VCard a;
				if ( a.fromXml(e) ) {
					if ( !v.d->agent )
						v.d->agent = QSharedPointer<VCard>(new VCard);
					*(v.d->agent) = a;
				}
			}

			v.d->agentURI = subTagText(i, QStringLiteral("EXTVAL"));
		}
		else if ( tag == QLatin1String("ORG") ) {
			v.d->org.name = subTagText(i, QStringLiteral("ORGNAME"));

			QDomNode nn = i.firstChild();
			for ( ; !nn.isNull(); nn = nn.nextSibling() ) {
				QDomElement ii = nn.toElement();
				if ( ii.isNull() )
					continue;

				if ( ii.tagName().toUpper() == QLatin1String("ORGUNIT") )
					v.d->org.unit.append( ii.text().trimmed() );
			}
		}
		else if ( tag == QLatin1String("CATEGORIES")) {
			QDomNode nn = i.firstChild();
			for ( ; !nn.isNull(); nn = nn.nextSibling() ) {
				QDomElement ee = nn.toElement();
				if ( ee.isNull() )
					continue;

				if ( ee.tagName().toUpper() == QLatin1String("KEYWORD") )
					v.d->categories << ee.text().trimmed();
			}
		}
		else if ( tag == QLatin1String("NOTE") )
			v.d->note = i.text().trimmed();
		else if ( tag == QLatin1String("PRODID") )
			v.d->prodId = i.text().trimmed();
		else if ( tag == QLatin1String("REV") )
			v.d->rev = i.text().trimmed();
		else if ( tag == QLatin1String("SORT-STRING") )
			v.d->sortString = i.text().trimmed();
		else if ( tag == QLatin1String("SOUND") ) {
			v.d->sound = QCA::Base64().stringToArray( subTagText(i, QStringLiteral("BINVAL")).replace(QLatin1String("\n"),QLatin1String("")) ).toByteArray();
			v.d->soundURI      = subTagText(i, QStringLiteral("EXTVAL"));
			v.d->soundPhonetic = subTagText(i, QStringLiteral("PHONETIC"));
		}
		else if ( tag == QLatin1String("UID") )
			v.d->uid = i.text().trimmed();
		else if ( tag == QLatin1String("URL"))
			v.d->url = i.text().trimmed();
		else if ( tag == QLatin1String("DESC") )
			v.d->desc = i.text().trimmed();
		else if ( tag == QLatin1String("CLASS") ) {
			if ( hasSubTag(i, QStringLiteral("PUBLIC")) )
				v.d->privacyClass = pcPublic;
			else if ( hasSubTag(i, QStringLiteral("PRIVATE")) )
				v.d->privacyClass = pcPrivate;
			else if ( hasSubTag(i, QStringLiteral("CONFIDENTIAL")) )
				v.d->privacyClass = pcConfidential;
		}
		else if ( tag == QLatin1String("KEY") ) {
			// TODO: Justin, please check out this code
			e = i.firstChildElement(QStringLiteral("TYPE"));
			QString type = QStringLiteral("text/plain");
			if ( !e.isNull() )
				type = e.text().trimmed();

			e = i.firstChildElement(QStringLiteral("CRED"));
			if ( e.isNull() )
				e = i.firstChildElement(QStringLiteral("BINVAL")); // case for very clever clients ;-)

			if ( !e.isNull() )
				v.d->key = e.text().toUtf8(); // FIXME
		}
	}

	return v;
}

bool VCard::isEmpty() const
{
	return !d || d->isEmpty();
}

VCard VCard::makeEmpty()
{
	VCard vcard;
	vcard.d = new VCardPrivate;
	return vcard;
}

// Some constructors

VCard::Address::Address()
{
	home = work = postal = parcel = dom = intl = pref = false;
}

VCard::Label::Label()
{
	home = work = postal = parcel = dom = intl = pref = false;
}

VCard::Phone::Phone()
{
	home = work = voice = fax = pager = msg = cell = video = bbs = modem = isdn = pcs = pref = false;
}

VCard::Email::Email()
{
	home = work = internet = x400 = false;
}

VCard::Geo::Geo()
{
}

VCard::Org::Org()
{
}

// vCard properties...

const QString &VCard::version() const
{
	return d->version;
}

void VCard::setVersion(const QString &v)
{
	d->version = v;
}

const QString &VCard::fullName() const
{
	return d->fullName;
}

void VCard::setFullName(const QString &n)
{
	d->fullName = n;
}

const QString &VCard::familyName() const
{
	return d->familyName;
}

void VCard::setFamilyName(const QString &n)
{
	d->familyName = n;
}

const QString &VCard::givenName() const
{
	return d->givenName;
}

void VCard::setGivenName(const QString &n)
{
	d->givenName = n;
}

const QString &VCard::middleName() const
{
	return d->middleName;
}

void VCard::setMiddleName(const QString &n)
{
	d->middleName = n;
}

const QString &VCard::prefixName() const
{
	return d->prefixName;
}

void VCard::setPrefixName(const QString &p)
{
	d->prefixName = p;
}

const QString &VCard::suffixName() const
{
	return d->suffixName;
}

void VCard::setSuffixName(const QString &s)
{
	d->suffixName = s;
}

const QString &VCard::nickName() const
{
	return d->nickName;
}

void VCard::setNickName(const QString &n)
{
	d->nickName = n;
}

const QByteArray &VCard::photo() const
{
	return d->photo;
}

void VCard::setPhoto(const QByteArray &i)
{
	d->photo = i;
}

const QString &VCard::photoURI() const
{
	return d->photoURI;
}

void VCard::setPhotoURI(const QString &p)
{
	d->photoURI = p;
}

const QDate VCard::bday() const
{
	return QDate::fromString(d->bday);
}

void VCard::setBday(const QDate &date)
{
	d->bday = date.toString();
}

const QString &VCard::bdayStr() const
{
	return d->bday;
}

void VCard::setBdayStr(const QString &date)
{
	d->bday = date;
}

const VCard::AddressList &VCard::addressList() const
{
	return d->addressList;
}

void VCard::setAddressList(const VCard::AddressList &a)
{
	d->addressList = a;
}

const VCard::LabelList &VCard::labelList() const
{
	return d->labelList;
}

void VCard::setLabelList(const VCard::LabelList &l)
{
	d->labelList = l;
}

const VCard::PhoneList &VCard::phoneList() const
{
	return d->phoneList;
}

void VCard::setPhoneList(const VCard::PhoneList &p)
{
	d->phoneList = p;
}

const VCard::EmailList &VCard::emailList() const
{
	return d->emailList;
}

void VCard::setEmailList(const VCard::EmailList &e)
{
	d->emailList = e;
}

const QString &VCard::jid() const
{
	return d->jid;
}

void VCard::setJid(const QString &j)
{
	d->jid = j;
}

const QString &VCard::mailer() const
{
	return d->mailer;
}

void VCard::setMailer(const QString &m)
{
	d->mailer = m;
}

const QString &VCard::timezone() const
{
	return d->timezone;
}

void VCard::setTimezone(const QString &t)
{
	d->timezone = t;
}

const VCard::Geo &VCard::geo() const
{
	return d->geo;
}

void VCard::setGeo(const VCard::Geo &g)
{
	d->geo = g;
}

const QString &VCard::title() const
{
	return d->title;
}

void VCard::setTitle(const QString &t)
{
	d->title = t;
}

const QString &VCard::role() const
{
	return d->role;
}

void VCard::setRole(const QString &r)
{
	d->role = r;
}

const QByteArray &VCard::logo() const
{
	return d->logo;
}

void VCard::setLogo(const QByteArray &i)
{
	d->logo = i;
}

const QString &VCard::logoURI() const
{
	return d->logoURI;
}

void VCard::setLogoURI(const QString &l)
{
	d->logoURI = l;
}

VCard VCard::agent() const
{
	if (d->agent) {
		return *(d->agent); // implicit copy
	}
	return VCard();
}

void VCard::setAgent(const VCard &v)
{
	if ( !d->agent )
		d->agent = QSharedPointer<VCard>(new VCard);
	*(d->agent) = v;
}

const QString VCard::agentURI() const
{
	return d->agentURI;
}

void VCard::setAgentURI(const QString &a)
{
	d->agentURI = a;
}

const VCard::Org &VCard::org() const
{
	return d->org;
}

void VCard::setOrg(const VCard::Org &o)
{
	d->org = o;
}

const QStringList &VCard::categories() const
{
	return d->categories;
}

void VCard::setCategories(const QStringList &c)
{
	d->categories = c;
}

const QString &VCard::note() const
{
	return d->note;
}

void VCard::setNote(const QString &n)
{
	d->note = n;
}

const QString &VCard::prodId() const
{
	return d->prodId;
}

void VCard::setProdId(const QString &p)
{
	d->prodId = p;
}

const QString &VCard::rev() const
{
	return d->rev;
}

void VCard::setRev(const QString &r)
{
	d->rev = r;
}

const QString &VCard::sortString() const
{
	return d->sortString;
}

void VCard::setSortString(const QString &s)
{
	d->sortString = s;
}

const QByteArray &VCard::sound() const
{
	return d->sound;
}

void VCard::setSound(const QByteArray &s)
{
	d->sound = s;
}

const QString &VCard::soundURI() const
{
	return d->soundURI;
}

void VCard::setSoundURI(const QString &s)
{
	d->soundURI = s;
}

const QString &VCard::soundPhonetic() const
{
	return d->soundPhonetic;
}

void VCard::setSoundPhonetic(const QString &s)
{
	d->soundPhonetic = s;
}

const QString &VCard::uid() const
{
	return d->uid;
}

void VCard::setUid(const QString &u)
{
	d->uid = u;
}

const QString &VCard::url() const
{
	return d->url;
}

void VCard::setUrl(const QString &u)
{
	d->url = u;
}

const QString &VCard::desc() const
{
	return d->desc;
}

void VCard::setDesc(const QString &desc)
{
	d->desc = desc;
}

const VCard::PrivacyClass &VCard::privacyClass() const
{
	return d->privacyClass;
}

void VCard::setPrivacyClass(const VCard::PrivacyClass &c)
{
	d->privacyClass = c;
}

const QByteArray &VCard::key() const
{
	return d->key;
}

void VCard::setKey(const QByteArray &k)
{
	d->key = k;
}

} // namespace XMPP
