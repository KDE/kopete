/*
    addressbooklink.h - Manages operations involving the KDE Address Book

    Copyright (c) 2005 Will Stephenson <wstephenson@kde.org>

    Kopete    (c) 2002-2004 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef KABCPERSISTENCE_H
#define KABCPERSISTENCE_H

#include <QObject>

#include "kopete_export.h"

// Goal is to have all the address book modifying code in one place
// Currently in 
// *) Add Contact Wizard
// *) KopeteMetaContact
// *) KopeteAddrBookExport
// *) KABC Export Wizard - TODO - think about sequence of events when adding addressees AND writing their IM data. - Extra save should be unnecessary because we are sharing a kabc instance
// *) Select addressbook entry

namespace KABC
{
	class AddressBook;
	class Resource;
}

namespace Kopete
{

	class MetaContact;
	
class KOPETE_EXPORT KABCPersistence : public QObject
{
	Q_OBJECT
	public:
		/**
		 * \brief Retrieve the instance of AccountManager.
		 *
		 * The account manager is a singleton class of which only a single
		 * instance will exist. If no manager exists yet this function will
		 * create one for you.
		 *
		 * \return the instance of the AccountManager
		 */
		static KABCPersistence* self();
		
		explicit KABCPersistence( QObject * parent = 0, const char * name = 0 );
		~KABCPersistence();
		/**
		 * @brief Access Kopete's KDE address book instance
		 */
		static KABC::AddressBook* addressBook();
		/**
		 * @brief Change the KABC data associated with this metacontact
		 *
		 * The KABC exposed data changed, so change it in KABC.
		 * Replaces Kopete::MetaContact::updateKABC()
		 */
		void write( MetaContact * mc );

		/**
		 * @brief Remove any KABC data for this meta contact
		 */
		void removeKABC( MetaContact * mc );
	
		/**
		 * Check for any new addresses added to this contact's KABC entry
		 * and prompt if they should be added in Kopete too.
		 * @return whether any contacts were added from KABC.
		 */
		bool syncWithKABC( MetaContact * mc );
		
		/**
		 * Request an address book write, will be delayed to bundle any others happening around the same time
		 */
		void writeAddressBook( KABC::Resource * res );
	protected:

		static void splitField( const QString &str, QString &app, QString &name, QString &value );
	protected slots:
		/**
		 * Perform a delayed address book write
		 */
		void slotWriteAddressBook();
	private:
		class Private;
		Private * const d;
};

} // end namespace Kopete

#endif // KABCPERSISTENCE_H
