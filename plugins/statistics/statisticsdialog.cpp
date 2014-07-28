/*
    statisticsdialog.cpp - Kopete Statistics Dialog

    Copyright (c) 2003-2004 by Marc Cramdal        <marc.cramdal@gmail.com>

    Copyright (c) 2007      by the Kopete Developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "statisticsdialog.h"

#include <qtabwidget.h>
#include <qwidget.h>

#include <qstring.h>
#include <QBuffer>
#include <KDateTable>
#include <KGlobalSettings>
#include <KColorScheme>
#include <QScrollArea>
#include <QObject>

#include <khbox.h>
#include <kdialog.h>
#include <klocalizedstring.h>
#include <khtml_part.h>
#include <kstandarddirs.h>

#include "kopetemetacontact.h"
#include "kopeteonlinestatus.h"

#include "statisticscontact.h"
#include "ui_statisticswidgetbase.h"
#include "statisticsplugin.h"
#include "statisticsdb.h"

StatisticsDialog::StatisticsDialog ( StatisticsContact *contact, StatisticsDB *db, QWidget* parent ) : KDialog ( parent ), m_db ( db ), m_contact ( contact )
{
	setAttribute ( Qt::WA_DeleteOnClose, true );
	setButtons ( KDialog::Close );
	setDefaultButton ( KDialog::Close );
	setCaption ( i18n ( "Statistics for %1", contact->metaContact()->displayName() ) );
	QWidget * w = new QWidget ( this );
	dialogUi = new Ui::StatisticsWidgetUI();
	dialogUi->setupUi ( w );
	setMainWidget ( w );


	KHBox *generalHBox = new KHBox ( this );

	generalHTMLPart = new KHTMLPart ( generalHBox );
	connect ( generalHTMLPart->browserExtension(), SIGNAL (openUrlRequestDelayed(KUrl,KParts::OpenUrlArguments,KParts::BrowserArguments)),
	          this, SLOT (slotOpenURLRequest(KUrl,KParts::OpenUrlArguments,KParts::BrowserArguments)) );
	generalHTMLPart->setJScriptEnabled ( false );
	generalHTMLPart->setJavaEnabled ( false );
	generalHTMLPart->setMetaRefreshEnabled ( false );
	generalHTMLPart->setPluginsEnabled ( false );
	generalHTMLPart->setOnlyLocalReferences ( true );

	dialogUi->tabWidget->insertTab ( 0, generalHBox, i18n ( "General" ) );
	dialogUi->tabWidget->setCurrentIndex ( 0 );

	KColorScheme scheme ( QPalette::Active, KColorScheme::View );
	m_onlineColor = scheme.background ( KColorScheme::ActiveBackground ).color().darker(130);
	m_awayColor = scheme.background ( KColorScheme::NeutralBackground ).color().darker(130);
	m_offlineColor = scheme.background ( KColorScheme::AlternateBackground ).color().darker(130);
	m_backgroundColor = scheme.background ( KColorScheme::NormalBackground ).color().darker(130);
	m_textColor = scheme.foreground ( KColorScheme::NormalText ).color();

	calendarHTMLPart = new KHTMLPart ( dialogUi->calendarHBox );
	calendarHTMLPart->setJScriptEnabled ( false );
	calendarHTMLPart->setJavaEnabled ( false );
	calendarHTMLPart->setMetaRefreshEnabled ( false );
	calendarHTMLPart->setPluginsEnabled ( false );
	calendarHTMLPart->setOnlyLocalReferences ( true );

	dialogUi->calendarKey->setTextFormat ( Qt::RichText );
	dialogUi->calendarKey->setText ( i18n ( "Key:  "
	                                        "<font color=\"%1\">Online</font>  "
	                                        "<font color=\"%2\">Away</font>  "
	                                        "<font color=\"%3\">Offline</font>",
	                                        m_onlineColor.name(),
	                                        m_awayColor.name(),
	                                        m_offlineColor.name() ) );

	dialogUi->datePicker->setDate ( QDate::currentDate() );
	connect ( dialogUi->datePicker, SIGNAL (dateChanged(QDate)), this, SLOT (fillCalendarCells()) );
	connect ( dialogUi->datePicker, SIGNAL (dateChanged(QDate)), this, SLOT (generateOneDayStats()) );

	setFocus();
	setEscapeButton ( Close );

	generatePageGeneral();
	fillCalendarCells();
	generateOneDayStats();
}

StatisticsDialog::~StatisticsDialog()
{
	delete dialogUi;
}

// We only generate pages when the user clicks on a link
void StatisticsDialog::slotOpenURLRequest ( const KUrl& url, const KParts::OpenUrlArguments &, const KParts::BrowserArguments & )
{
	if ( url.protocol() == "main" )
	{
		generatePageGeneral();
	}
	else if ( url.protocol() == "dayofweek" )
	{
		generatePageForDay ( url.path().toInt() );
	}
	else if ( url.protocol() == "monthofyear" )
	{
		generatePageForMonth ( url.path().toInt() );
	}
}

/*void StatisticsDialog::parseTemplate(QString Template)
{
	QString fileString = ::locate("appdata", "kopete_statistics.template.html");
	QString templateString;
	QFile file(file);
	if (file.open(QIODevice::ReadOnly))
	{
		QTextStream stream(&file);
		templateString = stream.read();
		file.close();
	}
	// The template is loaded in templateString now.
	templateString.strReplace(
}*/

void StatisticsDialog::generatePageForMonth ( const int monthOfYear )
{
	QStringList values = m_db->query ( QString ( "SELECT status, datetimebegin, datetimeend "
	                                   "FROM contactstatus WHERE metacontactid LIKE '%1' ORDER BY datetimebegin;" ).arg ( m_contact->metaContactId() ) );

	QStringList values2;

	for ( int i=0; i<values.count(); i+=3 )
	{
		QDateTime dateTimeBegin;
		dateTimeBegin.setTime_t ( values[i+1].toInt() );
		/// @todo Same as for Day, check if second datetime is on the same month
		if ( dateTimeBegin.date().month() == monthOfYear )
		{
			values2.push_back ( values[i] );
			values2.push_back ( values[i+1] );
			values2.push_back ( values[i+2] );
		}
	}
	generatePageFromQStringList ( values2, QDate::longMonthName ( monthOfYear ) );
}

void StatisticsDialog::generatePageForDay ( const int dayOfWeek )
{
	QStringList values = m_db->query ( QString ( "SELECT status, datetimebegin, datetimeend "
	                                   "FROM contactstatus WHERE metacontactid LIKE '%1' ORDER BY datetimebegin;" ).arg ( m_contact->metaContactId() ) );

	QStringList values2;

	for ( int i=0; i<values.count(); i+=3 )
	{
		QDateTime dateTimeBegin;
		dateTimeBegin.setTime_t ( values[i+1].toInt() );
		QDateTime dateTimeEnd;
		dateTimeEnd.setTime_t ( values[i+2].toInt() );
		if ( dateTimeBegin.date().dayOfWeek() == dayOfWeek )
		{
			if ( dateTimeEnd.date().dayOfWeek() != dayOfWeek )
				// Day of week is not the same at beginning and at end of the event
			{
				values2.push_back ( values[i] );
				values2.push_back ( values[i+1] );

				// datetime from value[i+1]

				dateTimeBegin = QDateTime ( dateTimeBegin.date(), QTime ( 0, 0, 0 ) );
				dateTimeBegin.addSecs ( dateTimeBegin.time().secsTo ( QTime ( 23, 59, 59 ) ) );
				values2.push_back ( QString::number ( dateTimeBegin.toTime_t() ) );
			}
			else
			{
				values2.push_back ( values[i] );
				values2.push_back ( values[i+1] );
				values2.push_back ( values[i+2] );
			}
		}
	}
	generatePageFromQStringList ( values2, QDate::longDayName ( dayOfWeek ) );

}

/// @todo chart problem at midnight.
void StatisticsDialog::generatePageFromQStringList ( QStringList values, const QString & subTitle )
{
	KColorScheme colorScheme ( QPalette::Active, KColorScheme::View );
	generalHTMLPart->begin();
	generalHTMLPart->write ( QString ( "<html><head><style>.bar { margin:0px;} "
	                                   "body"
	                                   "{"
	                                   "font-size:11px"
	                                   "}"
	                                   ".chart"								// Style for the charts
	                                   "{ height:100px;"
	                                   "border-left:1px solid #999;"
	                                   "border-bottom:1px solid #999;"
	                                   "vertical-align:bottom;"
	                                   "}"
	                                   ".statgroup"							// Style for groups of similar statistics
	                                   "{ margin-bottom:10px;"
	                                   "background-color:white;"
	                                   "border-left: 5px solid #369;"
	                                   "border-top: 1px dashed #999;"
	                                   "border-bottom: 1px dashed #999;"
	                                   "margin-left: 10px;"
	                                   "margin-right: 5px;"
	                                   "padding:3px 3px 3px 10px;}"
	                                   "</style></head><body>" +
	                                   i18n ( "<h1>Statistics for %1</h1>", m_contact->metaContact()->displayName() ) +
	                                   "<h3>%1</h3><hr>" ).arg ( subTitle ) );

	generalHTMLPart->write ( i18n ( "<div class=\"statgroup\"><b><a href=\"main:generalinfo\" title=\"General summary view\">General</a></b><br>"
	                                "<span title=\"Select the day or month for which you want to view statistics\"><b>Days: </b>"
	                                "<a href=\"dayofweek:1\">Monday</a>&nbsp;"
	                                "<a href=\"dayofweek:2\">Tuesday</a>&nbsp;"
	                                "<a href=\"dayofweek:3\">Wednesday</a>&nbsp;"
	                                "<a href=\"dayofweek:4\">Thursday</a>&nbsp;"
	                                "<a href=\"dayofweek:5\">Friday</a>&nbsp;"
	                                "<a href=\"dayofweek:6\">Saturday</a>&nbsp;"
	                                "<a href=\"dayofweek:7\">Sunday</a><br>"
	                                "<b>Months: </b>"
	                                "<a href=\"monthofyear:1\">January</a>&nbsp;"
	                                "<a href=\"monthofyear:2\">February</a>&nbsp;"
	                                "<a href=\"monthofyear:3\">March</a>&nbsp;"
	                                "<a href=\"monthofyear:4\">April</a>&nbsp;"
	                                "<a href=\"monthofyear:5\">May</a>&nbsp;"
	                                "<a href=\"monthofyear:6\">June</a>&nbsp;"
	                                "<a href=\"monthofyear:7\">July</a>&nbsp;"
	                                "<a href=\"monthofyear:8\">August</a>&nbsp;"
	                                "<a href=\"monthofyear:9\">September</a>&nbsp;"
	                                "<a href=\"monthofyear:10\">October</a>&nbsp;"
	                                "<a href=\"monthofyear:11\">November</a>&nbsp;"
	                                "<a href=\"monthofyear:12\">December</a>&nbsp;"
	                                "</span></div><br>" ) );

//	dialogUi->listView->addColumn(i18n("Status"));
//	dialogUi->listView->addColumn(i18n("Start Date"));
//	dialogUi->listView->addColumn(i18n("End Date"));
//	dialogUi->listView->addColumn(i18n("Start Date"));
//	dialogUi->listView->addColumn(i18n("End Date"));

	QString todayString;
	todayString.append ( i18n ( "<div class=\"statgroup\" title=\"Contact status history for today\"><h2>Today</h2><table width=\"100%\"><tr><td>Status</td><td>From</td><td>To</td></tr>" ) );

	bool today;

	int totalTime = 0; // this is in seconds
	int totalAwayTime = 0; // this is in seconds
	int totalOnlineTime = 0; // this is in seconds
	int totalOfflineTime = 0; // idem

	int hours[24]; // in seconds, too
	int iMaxHours = 0;
	int hoursOnline[24]; // this is in seconds
	int iMaxHoursOnline = 0;
	int hoursAway[24]; // this is in seconds
	int iMaxHoursAway = 0;
	int hoursOffline[24]; // this is in seconds. Hours where we are sure contact is offline
	int iMaxHoursOffline = 0;

	for ( uint i=0; i<24; i++ )
	{
		hours[i] = 0;
		hoursOnline[i] = 0;
		hoursAway[i] = 0;
		hoursOffline[i] = 0;
	}

	for ( int i=0; i<values.count(); i+=3 /* because SELECT 3 columns */ )
	{
		/* 	Here we try to interpret one database entry...
			What's important here, is to not count two times the same hour for instance
			This is why there are some if in all this stuff ;-)
		*/


		// it is the STARTDATE from the database
		QDateTime dateTime1;
		dateTime1.setTime_t ( values[i+1].toInt() );
		// it is the ENDDATE from the database
		QDateTime dateTime2;
		dateTime2.setTime_t ( values[i+2].toInt() );

		if ( dateTime1.date() == QDate::currentDate() || dateTime2.date() == QDate::currentDate() )
			today = true;
		else today = false;

		totalTime += dateTime1.secsTo ( dateTime2 );

		if ( Kopete::OnlineStatus::statusStringToType ( values[i] ) == Kopete::OnlineStatus::Online )
			totalOnlineTime += dateTime1.secsTo ( dateTime2 );
		else if ( Kopete::OnlineStatus::statusStringToType ( values[i] ) == Kopete::OnlineStatus::Away )
			totalAwayTime += dateTime1.secsTo ( dateTime2 );
		else if ( Kopete::OnlineStatus::statusStringToType ( values[i] ) == Kopete::OnlineStatus::Busy )
			totalAwayTime += dateTime1.secsTo ( dateTime2 );
		else if ( Kopete::OnlineStatus::statusStringToType ( values[i] ) == Kopete::OnlineStatus::Offline )
			totalOfflineTime += dateTime1.secsTo ( dateTime2 );


		/*
		 * To build the chart/hours
		 */

		// Number of hours between dateTime1 and dateTime2
		int nbHours = ( int ) ( dateTime1.secsTo ( dateTime2 ) /3600.0 );

		uint tempHour =
		    dateTime1.time().hour() == dateTime2.time().hour()
		    ? dateTime1.secsTo ( dateTime2 ) // (*)
		    : 3600 - dateTime1.time().minute() *60 - dateTime1.time().second();
		hours[dateTime1.time().hour() ] += tempHour;

		if ( Kopete::OnlineStatus::statusStringToType ( values[i] ) == Kopete::OnlineStatus::Online )
			hoursOnline[dateTime1.time().hour() ] += tempHour;
		else if ( Kopete::OnlineStatus::statusStringToType ( values[i] ) == Kopete::OnlineStatus::Away )
			hoursAway[dateTime1.time().hour() ] += tempHour;
		else if ( Kopete::OnlineStatus::statusStringToType ( values[i] ) == Kopete::OnlineStatus::Busy )
			hoursAway[dateTime1.time().hour() ] += tempHour;
		else if ( Kopete::OnlineStatus::statusStringToType ( values[i] ) == Kopete::OnlineStatus::Offline )
			hoursOffline[dateTime1.time().hour() ] += tempHour;

		for ( int j= dateTime1.time().hour() +1; j < dateTime1.time().hour() + nbHours - 1; j++ )
		{
			hours[j%24] += 3600;
			if ( Kopete::OnlineStatus::statusStringToType ( values[i] ) == Kopete::OnlineStatus::Online )
				hoursOnline[j%24] += 3600;
			else if ( Kopete::OnlineStatus::statusStringToType ( values[i] ) == Kopete::OnlineStatus::Away )
				hoursAway[j%24] += 3600;
			else if ( Kopete::OnlineStatus::statusStringToType ( values[i] ) == Kopete::OnlineStatus::Busy )
				hoursAway[j%24] += 3600;
			else if ( Kopete::OnlineStatus::statusStringToType ( values[i] ) == Kopete::OnlineStatus::Offline )
				hoursOffline[j%24] += 3600;
		}


		if ( dateTime1.time().hour() != dateTime2.time().hour() )
			// We don't want to count this if the hour from dateTime2 is the same than the one from dateTime1
			// since it as already been taken in account in the (*) instruction
		{
			tempHour = dateTime2.time().minute() *60 +dateTime2.time().second();
			hours[dateTime2.time().hour() ] += tempHour;

			if ( Kopete::OnlineStatus::statusStringToType ( values[i] ) == Kopete::OnlineStatus::Online )
				hoursOnline[dateTime2.time().hour() ] += tempHour;
			else if ( Kopete::OnlineStatus::statusStringToType ( values[i] ) == Kopete::OnlineStatus::Away )
				hoursAway[dateTime2.time().hour() ] += tempHour;
			else if ( Kopete::OnlineStatus::statusStringToType ( values[i] ) == Kopete::OnlineStatus::Busy )
				hoursAway[dateTime2.time().hour() ] += tempHour;
			else if ( Kopete::OnlineStatus::statusStringToType ( values[i] ) == Kopete::OnlineStatus::Offline )
				hoursOffline[dateTime2.time().hour() ] += tempHour;


		}

		if ( today )
		{
			QString status;
			if ( Kopete::OnlineStatus::statusStringToType ( values[i] ) == Kopete::OnlineStatus::Online )
				status = i18n ( "Online" );
			else if ( Kopete::OnlineStatus::statusStringToType ( values[i] ) == Kopete::OnlineStatus::Away )
				status = i18n ( "Away" );
			else if ( Kopete::OnlineStatus::statusStringToType ( values[i] ) == Kopete::OnlineStatus::Busy )
				status = i18n ( "Busy" );
			else if ( Kopete::OnlineStatus::statusStringToType ( values[i] ) == Kopete::OnlineStatus::Offline )
				status = i18n ( "Offline" );

			todayString.append ( QString ( "<tr><td>%2</td><td>%3</td><td>%4</td></tr>" ).arg ( status, dateTime1.time().toString(), dateTime2.time().toString() ) );
		}

		// We add a listview item to the log list
		// QDateTime listViewDT1, listViewDT2;
		// listViewDT1.setTime_t(values[i+1].toInt());
		// listViewDT2.setTime_t(values[i+2].toInt());
		// new K3ListViewItem(dialogUi->listView, values[i], values[i+1], values[i+2], listViewDT1.toString(), listViewDT2.toString());
	}


	todayString.append ( "</table></div>" );

	// Get the max from the hours*
	for ( uint i=1; i<24; i++ )
	{
		if ( hours[iMaxHours] < hours[i] )
			iMaxHours = i;
		if ( hoursOnline[iMaxHoursOnline] < hoursOnline[i] )
			iMaxHoursOnline = i;
		if ( hoursOffline[iMaxHoursOffline] < hoursOffline[i] )
			iMaxHoursOffline = i;
		if ( hoursAway[iMaxHoursAway] < hoursAway[i] )
			iMaxHoursAway = i;
	}

	//

	/*
	 * Here we really generate the page
	 */
	// Some "total times"
	generalHTMLPart->write ( i18n ( "<div class=\"statgroup\">" ) );
	generalHTMLPart->write ( i18n ( "<b title=\"The total time %1 was visible\">"
	                                "Total visible time :</b> %2<br>", m_contact->metaContact()->displayName(), stringFromSeconds ( totalOnlineTime + totalAwayTime ) ) );
	generalHTMLPart->write ( i18n ( "<b title=\"The total time %1 was online\">"
	                                "Total online time :</b> %2<br>", m_contact->metaContact()->displayName(), stringFromSeconds ( totalOnlineTime ) ) );
	generalHTMLPart->write ( i18n ( "<b title=\"The total time %1 was away\">Total busy time :</b> %2<br>", m_contact->metaContact()->displayName(), stringFromSeconds ( totalAwayTime ) ) );
	generalHTMLPart->write ( i18n ( "<b title=\"The total time %1 was offline\">Total offline time :</b> %2", m_contact->metaContact()->displayName(), stringFromSeconds ( totalOfflineTime ) ) );
	generalHTMLPart->write ( QString ( "</div>" ) );

	if ( subTitle == i18n ( "General information" ) )
		/*
		 * General stats that should not be shown on "day" or "month" pages
		 */
	{
		generalHTMLPart->write ( QString ( "<div class=\"statgroup\">" ) );
		generalHTMLPart->write ( i18np ( "<b>Average message length:</b> %1 character<br>",
						"<b>Average message length:</b> %1 characters<br>", m_contact->messageLength() ) );
		generalHTMLPart->write ( i18np ( "<b>Time between two messages:</b> %1 second",
						 "<b>Time between two messages:</b> %1 seconds", m_contact->timeBetweenTwoMessages() ) );
		generalHTMLPart->write ( QString ( "</div>" ) );

		generalHTMLPart->write ( QString ( "<div class=\"statgroup\">" ) );
		generalHTMLPart->write ( i18n ( "<b title=\"The last time you talked with %1\">Last talk :</b> %2<br>", m_contact->metaContact()->displayName(), KGlobal::locale()->formatDateTime ( m_contact->lastTalk() ) ) );
		generalHTMLPart->write ( i18n ( "<b title=\"The last time %1 was online or away\">Last time present :</b> %2", m_contact->metaContact()->displayName(), KGlobal::locale()->formatDateTime ( m_contact->lastPresent() ) ) );
		generalHTMLPart->write ( QString ( "</div>" ) );

		//generalHTMLPart->write(QString("<div class=\"statgroup\">"));
		//generalHTMLPart->write(i18n("<b title=\"%1 uses to set his status online at these hours (EXPERIMENTAL)\">Main online events :</b><br>").arg(m_contact->metaContact()->displayName()));
		//QValueList<QTime> mainEvents = m_contact->mainEvents(Kopete::OnlineStatus::Online);
		//for (uint i=0; i<mainEvents.count(); i++)
		//generalHTMLPart->write(QString("%1<br>").arg(mainEvents[i].toString()));
		//generalHTMLPart->write(QString("</div>"));

		generalHTMLPart->write ( "<div title=\"" +i18n ( "Current status" ) + "\" class=\"statgroup\">" );
		generalHTMLPart->write ( i18n ( "Is <b>%1</b> since <b>%2</b>",
		                                Kopete::OnlineStatus ( m_contact->oldStatus() ).description(),
		                                KGlobal::locale()->formatDateTime ( m_contact->oldStatusDateTime() ) ) );
		generalHTMLPart->write ( QString ( "</div>" ) );
	}

	/*
	 * Chart which shows the hours where plugin has seen this contact online
	 */
	generalHTMLPart->write ( QString ( "<div class=\"statgroup\">" ) );
	generalHTMLPart->write ( QString ( "<table width=\"100%\"><tr><td colspan=\"3\">" )
	                         + i18nc ( "TRANSLATOR: please reverse 'left' and 'right' as appropriate for your language",
	                                   "When was this contact visible?<br />All charts are in 24 blocks, "
	                                   "one per hour, encompassing one day. %1 is on the left, "
	                                   "and %2 is on the right.", KGlobal::locale()->formatTime ( QTime ( 0, 0 ) ),
	                                   KGlobal::locale()->formatTime ( QTime ( 23, 59 ) ) )
	                         + QString ( "</td></tr>" ) );
	generalHTMLPart->write ( QString ( "<tr><td height=\"200\" valign=\"bottom\" colspan=\"3\" class=\"chart\">" ) );

	QString chartString;
	QByteArray colorPath;
	QPixmap pixmap ( 1, 1 );
	pixmap.fill ( Qt::black );
	// Generate base64 picture.
	QByteArray tempArray;
	QBuffer tempBuffer ( &tempArray );
	tempBuffer.open ( QIODevice::WriteOnly );
	if ( pixmap.save ( &tempBuffer, "PNG" ) )
		colorPath = tempArray.toBase64();
	for ( uint i=0; i<24; i++ )
	{

		int hrWidth = qRound ( ( double ) ( hoursOnline[i] + hoursAway[i] ) / ( double ) hours[iMaxHours]*100. );
		chartString += QString ( "<img class=\"margin:0px;\"  height=\"" )
		               + ( totalTime ? QString::number ( hrWidth ) : QString::number ( 0 ) )
		               + QString ( "\" src=\"data:image/png;base64," )
		               +colorPath
		               +"\" width=\"4%\" title=\""
		               +i18n ( "Between %1 and %2, %3 was visible for %4% of the hour.",
		                       KGlobal::locale()->formatTime ( QTime ( i, 0 ) ),
		                       KGlobal::locale()->formatTime ( QTime ( ( i+1 ) % 24, 0 ) ),
		                       m_contact->metaContact()->displayName(), hrWidth )
		               + QString ( "\">" );
	}
	generalHTMLPart->write ( chartString );
	generalHTMLPart->write ( QString ( "</td></tr>" ) );



	generalHTMLPart->write ( QString ( "<tr><td>" )
	                         +i18n ( "Online time" )
	                         +QString ( "</td><td>" )
	                         +i18n ( "Away time" )
	                         +QString ( "</td><td>" )
	                         +i18n ( "Offline time" )
	                         +QString ( "</td></tr><td valign=\"bottom\" width=\"33%\" class=\"chart\">" ) );


	generalHTMLPart->write ( generateHTMLChart ( hoursOnline, hoursAway, hoursOffline, i18n ( "online" ),
	                         m_onlineColor ) );
	generalHTMLPart->write ( QString ( "</td><td valign=\"bottom\" width=\"33%\" class=\"chart\">" ) );
	generalHTMLPart->write ( generateHTMLChart ( hoursAway, hoursOnline, hoursOffline, i18n ( "away" ),
	                         m_awayColor ) );
	generalHTMLPart->write ( QString ( "</td><td valign=\"bottom\" width=\"33%\" class=\"chart\">" ) );
	generalHTMLPart->write ( generateHTMLChart ( hoursOffline, hoursAway, hoursOnline, i18n ( "offline" ),
	                         m_offlineColor ) );
	generalHTMLPart->write ( QString ( "</td></tr></table></div>" ) );

	if ( subTitle == i18n ( "General information" ) )
		/* On main page, show the different status of the contact today
		 */
	{
		generalHTMLPart->write ( QString ( todayString ) );
	}
	generalHTMLPart->write ( QString ( "</body></html>" ) );

	generalHTMLPart->end();

}

void StatisticsDialog::generatePageGeneral()
{
	QStringList values;
	values = m_db->query ( QString ( "SELECT status, datetimebegin, datetimeend "
	                                 "FROM contactstatus WHERE metacontactid LIKE '%1' ORDER BY datetimebegin;" )
	                       .arg ( m_contact->metaContactId() ) );
	generatePageFromQStringList ( values, i18n ( "General information" ) );
}

QString StatisticsDialog::generateHTMLChart ( const int *hours, const int *hours2, const int *hours3, const QString & caption, const QColor & color )
{
	QString chartString;
	QByteArray colorPath;

	QPixmap pixmap ( 1, 1 );
	pixmap.fill ( color );
	// Generate base64 picture.
	QByteArray tempArray;
	QBuffer tempBuffer ( &tempArray );
	tempBuffer.open ( QIODevice::WriteOnly );
	if ( pixmap.save ( &tempBuffer, "PNG" ) )
		colorPath = tempArray.toBase64();

	for ( uint i=0; i<24; i++ )
	{
		int totalTime = hours[i] + hours2[i] + hours3[i];

		int hrWidth = qRound ( ( double ) hours[i]/ ( double ) totalTime*100. );
		chartString += QString ( "<img class=\"margin:0px;\"  height=\"" )
		               + ( totalTime ? QString::number ( hrWidth ) : QString::number ( 0 ) )
		               + QString ( "\" src=\"data:image/png;base64," )
		               + colorPath
		               + "\" width=\"4%\" title=\""
		               + i18n ( "Between %1 and %2, %3 was %4% %5.",
		                        KGlobal::locale()->formatTime ( QTime ( i, 0 ) ),
		                        KGlobal::locale()->formatTime ( QTime ( ( i+1 ) % 24, 0 ) ),
		                        m_contact->metaContact()->displayName(),
		                        hrWidth,
		                        caption )
		               +"\">";
	}
	return chartString;
}

QString StatisticsDialog::stringFromSeconds ( const int seconds )
{
	return KGlobal::locale()->prettyFormatDuration ( (unsigned long)seconds * 1000 );
}

void StatisticsDialog::fillCalendarCells()
{
	QDateTime firstOfMonth ( dialogUi->datePicker->date() );
	QDateTime lastOfMonth ( dialogUi->datePicker->date() );
	firstOfMonth.setDate ( QDate ( firstOfMonth.date().year(), firstOfMonth.date().month(), 1 ) );
	lastOfMonth.setDate ( QDate ( lastOfMonth.date().year(), lastOfMonth.date().month(),
	                              lastOfMonth.date().daysInMonth() ) );

	QStringList values = m_db->query ( QString ( "SELECT status, datetimebegin, datetimeend "
	                                   "FROM contactstatus WHERE metacontactid LIKE '%1' AND "
	                                   "datetimebegin BETWEEN '%2' AND '%3' "
	                                   "AND datetimeend BETWEEN '%4' AND '%5';" )
	                                   .arg ( m_contact->metaContactId() )
	                                   .arg ( firstOfMonth.toTime_t() )
	                                   .arg ( lastOfMonth.toTime_t() )
	                                   .arg ( firstOfMonth.toTime_t() )
	                                   .arg ( lastOfMonth.toTime_t() ) );

	QVector <Kopete::OnlineStatus> statuses ( 32, Kopete::OnlineStatus::Unknown );
	// list of dates and statuses using begintimes
	for ( int i=0; i < values.count(); i += 3 )
	{
		QDate date ( QDateTime::fromTime_t ( values.at ( i+1 ).toUInt() ).date() );
		Kopete::OnlineStatus status = Kopete::OnlineStatus::statusStringToType ( values.at ( i ) );
		if ( status > statuses.at ( date.day() ) )
		{
			statuses[date.day() ] = status;
		}
	}
	// list of dates and statuses using endtimes
	for ( int i=0; i < values.count(); i += 3 )
	{
		QDate date ( QDateTime::fromTime_t ( values.at ( i+2 ).toUInt() ).date() );
		Kopete::OnlineStatus status = Kopete::OnlineStatus::statusStringToType ( values.at ( i ) );
		if ( status > statuses.at ( date.day() ) )
		{
			statuses[date.day() ] = status;
		}
	}

	for ( int i=0; i < statuses.count(); i++ )
	{
		QColor color ( m_backgroundColor );

		if ( statuses.at ( i ) == Kopete::OnlineStatus::Online )
			color = m_onlineColor;
		else if ( statuses.at ( i ) == Kopete::OnlineStatus::Away )
			color = m_awayColor;
		else if ( statuses.at ( i ) == Kopete::OnlineStatus::Busy )
			color = m_awayColor;
		else if ( statuses.at ( i ) == Kopete::OnlineStatus::Offline )
			color = m_offlineColor;

		dialogUi->datePicker->dateTable()->setCustomDatePainting (
		    QDate ( firstOfMonth.date().year(), firstOfMonth.date().month(), i ),
		    m_textColor, KDateTable::RectangleMode, color );
	}
	dialogUi->datePicker->update();
}

void StatisticsDialog::generateOneDayStats ()
{
	QDate day = dialogUi->datePicker->date();
	QDateTime topOfDay = QDateTime ( day );
	QDateTime endOfDay = topOfDay.addDays ( 1 );
	QStringList values = m_db->query ( QString ( "SELECT status, datetimebegin, datetimeend "
	                                   "FROM contactstatus WHERE metacontactid LIKE '%1' AND "
	                                   "datetimebegin BETWEEN '%2' AND '%3' "
	                                   "AND datetimeend BETWEEN '%4' AND '%5';" )
	                                   .arg ( m_contact->metaContactId() )
	                                   .arg ( topOfDay.toTime_t() )
	                                   .arg ( endOfDay.toTime_t() )
	                                   .arg ( topOfDay.toTime_t() )
	                                   .arg ( endOfDay.toTime_t() ) );

	QString todayString;
	todayString.append ( i18n ( "<h2>%1</h2><table width=\"100%\"><tr><td>Status</td><td>From</td><td>To</td></tr>",
	                            KGlobal::locale()->formatDate ( topOfDay.date(), KLocale::ShortDate ) ) );

	for ( int i=0; i<values.count(); i+=3 )
	{
		// it is the STARTDATE from the database
		QDateTime dateTime1;
		dateTime1.setTime_t ( values[i+1].toInt() );
		// it is the ENDDATE from the database
		QDateTime dateTime2;
		dateTime2.setTime_t ( values[i+2].toInt() );

		QString status;
		if ( Kopete::OnlineStatus::statusStringToType ( values[i] ) == Kopete::OnlineStatus::Online )
			status = i18n ( "Online" );
		else if ( Kopete::OnlineStatus::statusStringToType ( values[i] ) == Kopete::OnlineStatus::Away )
			status = i18n ( "Away" );
		else if ( Kopete::OnlineStatus::statusStringToType ( values[i] ) == Kopete::OnlineStatus::Busy )
			status = i18n ( "Busy" );
		else if ( Kopete::OnlineStatus::statusStringToType ( values[i] ) == Kopete::OnlineStatus::Offline )
			status = i18n ( "Offline" );

		todayString.append ( QString ( "<tr><td>%2</td><td>%3</td><td>%4</td></tr>" ).arg ( status, dateTime1.time().toString(), dateTime2.time().toString() ) );
	}
	todayString.append ( "</table></div>" );
	
	calendarHTMLPart->begin();
	calendarHTMLPart->write ( QString ( "<html><head><style>.bar { margin:0px;} "
			"body"
			"{"
			"font-size:11px"
			"}"
			"</style></head><body>" ) );
	calendarHTMLPart->write ( todayString );
	calendarHTMLPart->write ( QString ( "</body></html>" ) );
	calendarHTMLPart->end();

}

#include "statisticsdialog.moc"
