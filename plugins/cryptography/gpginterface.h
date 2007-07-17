//Code from KGPG

/***************************************************************************
                          gpginterface.h  -  description
                             -------------------
    begin                : Sat Jun 29 2002
    copyright            : (C) 2002 by y0k0 <bj@altern.org>
                           (C) 2007 by Charles Connell <charles@connells.org>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef GPGINTERFACE_H
#define GPGINTERFACE_H

class QString;

/**
 * Encrypt text using gpg.
 */
class GpgInterface : public QObject
{

		Q_OBJECT

	public:
		/**
		* The state of an operation.
		*/
		enum OpState { GoodSig = 0x1, BadSig = 0x2, ErrorSig = 0x4, NoSig = 0x8, Decrypted = 0x16 };
		
		/**Encrypt text function
		* @param text QString text to be encrypted.
		* @param userIDs The recipients key id's.
		* @param options String with the wanted gpg options. ex: "--armor"
		* @param signAlso Sign the text as well as encrypt it
		* @param privateKey The private key with which to do the signing. Must be supplied if signAlso is true
		* @returns The encrypted text or empty string if encryption failed
		*/
		static QString encryptText ( QString text, QString userIDs, QString options="", bool signAlso = false, QString privateKey = "" );

		/**Decrypt text function
		* @param text QString text to be decrypted.
		* @param userID QString the name of the decryption key (only used to prompt user for passphrase)
		* @param opState Set to the state of the signature on the message, if any
		* @returns The cleartext, or empty string if decryption failed
		*/
		static QString decryptText ( QString text, QString userID, int &opState );
		
		/**Sign text function
		* @param text QString text to be sign
		* @param userID The signer
		* @returns Signed text, or empty string if signing failed
		*/
		static QString signText ( QString text, QString userID );

		static QString checkForUtf8 ( QString s );
		
		static QString getPassword (QString password, QString userID, int counter);
};


#endif
