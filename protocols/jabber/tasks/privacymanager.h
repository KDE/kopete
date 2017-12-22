/*
 * privacymanager.h
 * Copyright (C) 2006  Remko Troncon
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef PRIVACYMANAGER_H
#define PRIVACYMANAGER_H

#include <QObject>
#include <QStringList>
#include <QDomElement>

#include "xmpp_task.h"
#include "privacylist.h"

class QString;

namespace XMPP {

class PrivacyListListener : public Task
{
	Q_OBJECT

	public:
		PrivacyListListener ( Task* parent );

		bool take ( const QDomElement &e ) Q_DECL_OVERRIDE;
};

class GetPrivacyListsTask : public Task
{
		Q_OBJECT

	private:
		QDomElement iq_;
		QStringList lists_;
		QString default_, active_;

	public:
		GetPrivacyListsTask ( Task* parent );

		void onGo() Q_DECL_OVERRIDE;
		bool take ( const QDomElement &x ) Q_DECL_OVERRIDE;
		const QStringList& lists();
		const QString& defaultList();
		const QString& activeList();
};

class SetPrivacyListsTask : public Task
{
		Q_OBJECT

	private:
		bool changeDefault_, changeActive_, changeList_;
		PrivacyList list_;
		QString value_;

	public:
		SetPrivacyListsTask ( Task* parent );

		void onGo() Q_DECL_OVERRIDE;
		void setActive ( const QString& active );
		void setDefault ( const QString& d );
		void setList ( const PrivacyList& list );
		bool take ( const QDomElement &x ) Q_DECL_OVERRIDE;
};

class GetPrivacyListTask : public Task
{
		Q_OBJECT

	private:
		QDomElement iq_;
		QString name_;
		PrivacyList list_;

	public:
		GetPrivacyListTask ( Task* parent, const QString& name );

		void onGo() Q_DECL_OVERRIDE;
		bool take ( const QDomElement &x ) Q_DECL_OVERRIDE;
		const PrivacyList& list();

};

class PrivacyManager : public QObject
{
		Q_OBJECT

	public:
		PrivacyManager ( XMPP::Task* rootTask );
		virtual ~PrivacyManager();

		void requestListNames();

		void changeDefaultList ( const QString& name );
		void changeActiveList ( const QString& name );
		void changeList ( const PrivacyList& list );
		void getDefaultList();
		void requestList ( const QString& name );

		// Convenience
		void block ( const QString& );

	protected:
		static QStringList blockedContacts ( const PrivacyList&, bool* allBlocked );

	private slots:
		void receiveLists();
		void receiveList();
		void changeDefaultList_finished();
		void changeActiveList_finished();
		void changeList_finished();
		void getDefault_listsReceived ( const QString&, const QString&, const QStringList& );
		void getDefault_listsError();
		void getDefault_listReceived ( const PrivacyList& );
		void getDefault_listError();

		void block_getDefaultList_success ( const PrivacyList& );
		void block_getDefaultList_error();

	signals:
		void changeDefaultList_success();
		void changeDefaultList_error();
		void changeActiveList_success();
		void changeActiveList_error();
		void changeList_success();
		void changeList_error();
		void defaultListAvailable ( const PrivacyList& );
		void defaultListError();
		void listChangeSuccess();
		void listChangeError();
		void listReceived ( const PrivacyList& p );
		void listError();
		void listsReceived ( const QString& defaultList, const QString& activeList, const QStringList& lists );
		void listsError();

	private:
		XMPP::Task* rootTask_;
		PrivacyListListener* listener_;

		bool getDefault_waiting_;
		QString getDefault_default_;

		QStringList block_targets_;
		bool block_waiting_;
};

} // namespace XMPP

#endif
