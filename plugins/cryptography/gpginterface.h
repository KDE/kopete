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
		/**Encrypt text function
		 * @param text QString text to be encrypted.
		 * @param userIDs the recipients key id's.
		 * @param Options String with the wanted gpg options. ex: "--armor"
		 * returns the encrypted text or empty string if encryption failed
		 */
		static QString encryptText ( QString text,QString userIDs, QString Options="" );

		/**Decrypt text function
		* @param text QString text to be decrypted.
		* @param userID QString the name of the decryption key (only used to prompt user for passphrase)
		*/
		static QString decryptText ( QString text,QString userID );

		static QString checkForUtf8 ( QString s );
};


#endif
