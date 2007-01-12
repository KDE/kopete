/*
  oscarprotocol.h  -  Oscar Protocol Plugin

  Copyright (c) 2003 by Olivier Goffart <ogoffart@kde.org>

  Kopete    (c) 2003 by the Kopete developers  <kopete-devel@kde.org>

  *************************************************************************
  *                                                                       *
  * This program is free software; you can redistribute it and/or modify  *
  * it under the terms of the GNU General Public License as published by  *
  * the Free Software Foundation; either version 2 of the License, or     *
  * (at your option) any later version.                                   *
  *                                                                       *
  *************************************************************************
*/

#ifndef ICQPROTOCOL_H
#define ICQPROTOCOL_H

#include "oscarprotocol.h"
#include "kopetemimetypehandler.h"
// #include "kopeteonlinestatus.h"
// #include "kopetecontactproperty.h"

class QComboBox;
/*class ICQUserInfoWidget;
class ICQContact;*/

namespace ICQ { class OnlineStatusManager; }

class ICQProtocolHandler : public Kopete::MimeTypeHandler
{
public:
	ICQProtocolHandler();
	void handleURL(const QString &mimeType, const KUrl & url) const;
};


class ICQProtocol : public OscarProtocol
{
Q_OBJECT

public:
	ICQProtocol(QObject *parent, const QStringList &args);
	virtual ~ICQProtocol();

	/**
	 * Return the active instance of the protocol
	 */
	static ICQProtocol *protocol();

	virtual bool canSendOffline() const;

	AddContactPage *createAddContactWidget(QWidget *parent, Kopete::Account *account);
	KopeteEditAccountWidget *createEditAccountWidget(Kopete::Account *account, QWidget *parent);
	Kopete::Account *createNewAccount(const QString &accountId);

	ICQ::OnlineStatusManager *statusManager();


	const Kopete::ContactPropertyTmpl firstName;
	const Kopete::ContactPropertyTmpl lastName;
	const Kopete::ContactPropertyTmpl emailAddress;
	const Kopete::ContactPropertyTmpl ipAddress;

	const QMap<int, QString> &genders() { return mGenders; }
	const QMap<int, QString> &countries() { return mCountries; }
	const QMap<int, QString> &languages() { return mLanguages; }
	const QMap<int, QString> &encodings() { return mEncodings; }
	const QMap<int, QString> &maritals() { return mMarital; }
	const QMap<int, QString> &interests() { return mInterests; }
	const QMap<int, QString> &occupations() { return mOccupations; }
	const QMap<int, QString> &organizations() { return mOrganizations; }
	const QMap<int, QString> &affiliations() { return mAffiliations; }

	void fillComboFromTable( QComboBox*, const QMap<int, QString>& );
	void setComboFromTable( QComboBox*, const QMap<int, QString>&, int );
	int getCodeForCombo( QComboBox*, const QMap<int, QString>& );
	/* void fillTZCombo(QComboBox *combo);
	void setTZComboValue(QComboBox *combo, const char &tz);
	char getTZComboValue(QComboBox *combo); */

private:
	void initGenders();
	void initLang();
	void initCountries();
	void initEncodings();
	void initMaritals();
	void initInterests();
	void initOccupations();
	void initOrganizations();
	void initAffiliations();

private:
	static ICQProtocol* protocolStatic_;
	ICQ::OnlineStatusManager* statusManager_;
	QMap<int, QString> mGenders;
	QMap<int, QString> mCountries;
	QMap<int, QString> mLanguages;
	QMap<int, QString> mEncodings;
	QMap<int, QString> mMarital;
	QMap<int, QString> mInterests;
	QMap<int, QString> mOccupations;
	QMap<int, QString> mOrganizations;
	QMap<int, QString> mAffiliations;
	ICQProtocolHandler protohandler;
};
#endif
// vim: set noet ts=4 sts=4 sw=4:
