/***************************************************************************
     yahooprotocol.h  -  Base class for the Kopete Yahoo protocol
                             -------------------
    begin                : Fri Apr 26 2002
    copyright            : (C) 2002 by Bruno Rodrigues
    email                : bruno.rodrigues@litux.org

    Based on code from   : (C) 2002 by Duncan Mac-Vicar Prett
    email                : duncan@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef YAHOOPROTOCOL_H
#define YAHOOPROTOCOL_H


// Local Includes
#include "yahooprefs.h"

// Kopete Includes

// QT Includes
#include <qpixmap.h>

// KDE Includes
#include "kopeteprotocol.h"


class StatusBarIcon;	// libkopete::ui::statusbaricon


// Yahoo Protocol
class YahooProtocol : public QObject, public KopeteProtocol {
	Q_OBJECT public:

		YahooProtocol();	// Constructor
		bool unload();		// Unload statusbar icon

	public slots:
		void Connect();			// Connect to server
		void Disconnect();		// Disconnect from server
		void setAvailable();	// Set user Available
		void setAway();			// Set user away

		bool isConnected() const;	// Return true if connected
		bool isAway() const;		// Return true if away

		QString protocolIcon() const;	// Return protocol icon name
		AddContactPage *createAddContactWidget(QWidget * parent);
										// Return "add contact" dialog

		void slotIconRightClicked(const QPoint);	
							// CallBack when clicking on statusbar icon
		void slotSettingsChanged(void);
							// Callback when settings changed

	signals:
		void protocolUnloading();	// Unload Protocol

	private:
		bool mIsConnected;				// Am I connected ?
		QString mUsername, mPassword, mServer; int mPort;	
										// Configuration data
		StatusBarIcon *statusBarIcon;	// Statusbar Icon Object
		YahooPreferences *mPrefs;		// Preferences Object
		QPixmap onlineIcon;				// Icons
		QPixmap offlineIcon;
		QPixmap busyIcon;
		QPixmap idleIcon;
		QPixmap mobileIcon;
		
		void initIcons();	// Load Icons
};

#endif

// vim: set noet ts=4 sts=4 sw=4:
