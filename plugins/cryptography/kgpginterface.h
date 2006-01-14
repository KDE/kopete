//Code from KGPG

/***************************************************************************
                          kgpginterface.h  -  description
                             -------------------
    begin                : Sat Jun 29 2002
    copyright            : (C) 2002 by 
    email                : 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KGPGINTERFACE_H
#define KGPGINTERFACE_H


#include <kurl.h>

/**
 * Encrypt a file using gpg.
 */
//class KgpgEncryptFile : public QObject {
class KgpgInterface : public QObject {

  Q_OBJECT

    public:
	/**
	 * Initialize the class
	 */
        KgpgInterface();
	
	
	/**Encrypt text function
	 * @param text QString text to be encrypted.
	 * @param userIDs the recipients key id's.
	 * @param Options String with the wanted gpg options. ex: "--armor"
	 * returns the encrypted text or empty string if encyption failed
	 */
	 static QString KgpgEncryptText(QString text,QString userIDs, QString Options="");
	
	 /**Decrypt text function
	 * @param text QString text to be decrypted.
	 * @param userID QString the name of the decryption key (only used to prompt user for passphrase)
	 */
	static QString KgpgDecryptText(QString text,QString userID);
//	static QString KgpgDecryptFileToText(KURL srcUrl,QString userID);

	/*
	 * Destructor for the class.
	 */
	~KgpgInterface();
	
	static QString checkForUtf8(QString txt);

	
    private slots:
	
signals:
	        
    private:
    /**
	 * @internal structure for communication
	 */
        QString message,tempKeyFile,userIDs,txtprocess,output;
		QCString passphrase;
		bool deleteSuccess,konsLocal,anonymous,txtsent,decfinished,decok,badmdc;
		int signSuccess;
		int step,signb,sigsearch;
		QString konsSignKey, konsKeyID;
		
		
	/**
	 * @internal structure for the file information
	 */
        KURL file;
	/**
	 * @internal structure to send signal only once on error.
	 */
	bool encError;
};


#endif
