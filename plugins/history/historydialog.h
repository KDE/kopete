/*
    kopetehistorydialog.h - Kopete History Dialog

    Copyright (c) 2002 by  Richard Stellingwerff <remenic@linuxfromscratch.org>

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

#ifndef _HISTORYDIALOG_H
#define _HISTORYDIALOG_H

#include <kdialogbase.h>
#include <qstringlist.h>

#include "kopetemessage.h"

class QGridLayout;
class QPushButton;
class KTextBrowser;
class QDomDocument;
class QFile;
class QProgressBar;
class QLabel;
class QLineEdit;
class QGroupBox;
class QCheckBox;
class QBoxLayout;


class KDialogBase;
class HistoryWidget;
class KopeteContact;
class KopeteMetaContact;
class HistoryLogger;

/**
 * @author Richard Stellingwerff <remenic@linuxfromscratch.org>
 *
 */

class HistoryDialog : public KDialogBase
{
	Q_OBJECT
public:
	HistoryDialog( KopeteContact *mContact, const bool showclose = true, int count = 50, QWidget* parent=0, const char* name="HistoryDialog");
	HistoryDialog( KopeteMetaContact *mContact, const bool showclose = true, int count = 50, QWidget* parent=0, const char* name="HistoryDialog");

	void init();

signals:
	void closing();

private slots:
	/**
	 * < button clicked
	 */
	void slotPrevClicked();

	/**
	 * > button clicked
	 */
	void slotNextClicked();

	/**
	 * << pressed
	 */
	void slotBackClicked();

	/**
	 * >> pressed
	 */
	void slotForwardClicked();

	/**
	 * search button clicked
	 */
	void slotSearchClicked();

	/**
	 * checkbox mReversed toggled
	 */
	void slotReversedToggled( bool toggled );

	/**
	 * checkbox mIncoming toggled
	 */
	void slotIncomingToggled( bool toggled );

	void addMessage( KopeteMessage::MessageDirection dir, QString nick, QString date, QString body );



private:
	void buildWidget( int count );

	void refreshEnabled( );


	// the actual log view
	KTextBrowser *mHistoryView;

	// where to start reading
	int msgStart;
	// amount of entries to read at once
	int msgCount;

	// main layout
	QGridLayout *layout;

	// BASIC CONTROLS
	// previous button "<"
	QPushButton *mPrevious;
	// next button ">"
	QPushButton *mNext;
	// all the way back "<<"
	QPushButton *mBack;
	// all the way forward ">>"
	QPushButton *mForward;

	// search button
	QPushButton *mSearchButton;
	// progress bar
	QProgressBar *mProgress;

	// "Search: " label
	QLabel *mSearchLabel;
	// input field for search
	QLineEdit *mSearchInput;

	// the options groupbox
	QGroupBox *optionsBox;
	// the main layout for the options groupbox
	QGridLayout *optionsLayout;
	// check boxes layout
	QBoxLayout *optionsCBLayout;

	// show oldest message first
	QCheckBox *mReverse;
	// only show incoming messages
	QCheckBox *mIncoming;

	// A buffer speeds up a lot
	QString mSuperBuffer;
	// Nickname of the user.
	QString mUser;

	// List of logFileNames
	QStringList logFileNames;

	HistoryLogger *m_logger;


};

#endif


