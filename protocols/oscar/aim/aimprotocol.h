/*
  oscarprotocol.h  -  Oscar Protocol Plugin

  Copyright (c) 2002 by Tom Linsky <twl6@po.cwru.edu>

  Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

  *************************************************************************
  *                                                                       *
  * This program is free software; you can redistribute it and/or modify  *
  * it under the terms of the GNU General Public License as published by  *
  * the Free Software Foundation; either version 2 of the License, or     *
  * (at your option) any later version.                                   *
  *                                                                       *
  *************************************************************************
  */

#ifndef AIMPROTOCOL_H
#define AIMPROTOCOL_H

#include "kopeteprotocol.h"
#include "kopetecontactproperty.h"

#include <qmap.h>

class KopeteOnlineStatus;

class AIMProtocol : public KopeteProtocol
{
	Q_OBJECT

	public:
		AIMProtocol(QObject *parent, const char *name, const QStringList &args);
		virtual ~AIMProtocol();
		/** Internal status enum */
		enum AIMInternalStatus
		{
			AIMONLINE, AIMOFFLINE, AIMAWAY, AIMCONN
		};

		/**
		* Return the active instance of the protocol
		* because it's a singleton, can only be used inside AIM classes, not in oscar lib
		*/
		static AIMProtocol *protocol();

		bool canSendOffline() const { return false; }

		void deserializeContact( KopeteMetaContact *metaContact,
			const QMap<QString, QString> &serializedData,
			const QMap<QString, QString> &addressBookData );
		AddContactPage *createAddContactWidget(QWidget *parent, KopeteAccount *account);
		KopeteEditAccountWidget *createEditAccountWidget(KopeteAccount *account, QWidget *parent);
		KopeteAccount *createNewAccount(const QString &accountId);

		/**
		 * The set of online statuses that AIM contacts can have
		 */
		const KopeteOnlineStatus statusOnline;
		const KopeteOnlineStatus statusOffline;
		const KopeteOnlineStatus statusAway;
		const KopeteOnlineStatus statusConnecting;

		const Kopete::ContactPropertyTmpl awayMessage;

	private:
		/** The active instance of oscarprotocol */
		static AIMProtocol *protocolStatic_;
};

#endif
// vim: set noet ts=4 sts=4 sw=4:
