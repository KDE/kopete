/*
    systemtray.h  -  Kopete Tray Dock Icon

    Copyright (c) 2002      by Nick Betcher           <nbetcher@kde.org>
    Copyright (c) 2002-2003 by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2003      by Olivier Goffart        <ogoffart@kde.org>

    Kopete    (c) 2002-2005 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef SYSTEMTRAY_H
#define SYSTEMTRAY_H

#include <KStatusNotifierItem>

#include "kopetemessageevent.h"

class QTimer;
class KMenu;

/**
 * @author Nick Betcher <nbetcher@kde.org>
 *
 * NOTE: This class is for use ONLY in libkopete! It is not public API, and
 *       is NOT supposed to remain binary compatible in the future!
 */
class KopeteSystemTray : public KStatusNotifierItem
{
	Q_OBJECT

public:
	/**
	 * Retrieve the system tray instance
	 */
	static KopeteSystemTray* systemTray( QWidget* parent = 0);

	virtual ~KopeteSystemTray();

	// One method, multiple interfaces :-)
	void startBlink( const QString &icon );
	void startBlink();

	void stopBlink();
	bool isBlinking() const;

Q_SIGNALS:
	void aboutToShowMenu(KMenu *am);

private Q_SLOTS:
	void slotAboutToShowMenu();
	void activate(const QPoint &pos=QPoint());

	void slotBlink();
	void slotNewEvent(Kopete::MessageEvent*);
	void slotEventDone(Kopete::MessageEvent *);
	void slotConfigChanged();
	void slotReevaluateAccountStates();

private:
	KopeteSystemTray( QWidget* parent );

	QTimer *mBlinkTimer;
	QString mKopeteIcon;
	QString mBlinkIcon;

	bool mIsBlinkIcon;

	static KopeteSystemTray* s_systemTray;

	QList<Kopete::MessageEvent*> mEventList;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

