/*
    statisticsdialog.h - Kopete Statistics Dialog

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

#ifndef _STATISTICSDIALOG_H
#define _STATISTICSDIALOG_H

#include <qwidget.h>
#include <kdialog.h>
#include <kurl.h>
#include "kopetemetacontact.h"

class QStringList;

namespace Ui { class StatisticsWidgetUI; }
class StatisticsDB;
class StatisticsContact;

class KHTMLPart;

namespace KParts
{
	struct BrowserArguments;
	class OpenUrlArguments;
}

class StatisticsDialog : public KDialog
{
	Q_OBJECT
	public:
		typedef QPair<QDate, QString> StatusPair;
		
		StatisticsDialog(StatisticsContact *contact, StatisticsDB* db, QWidget* parent=0);
		~StatisticsDialog();
		
		QSize sizeHint () const { return QSize ( 800, 600 ); }
		
	private:
		QString generateHTMLChart(const int *hours, const int *hours2, const int *hours3, const QString & caption, const QColor & color);
		QString generateHTMLChartBar(int height, const QString & color, const QString & caption);
		QString stringFromSeconds(const int seconds);

		Ui::StatisticsWidgetUI *dialogUi;
		KHTMLPart *generalHTMLPart;
		KHTMLPart *calendarHTMLPart;
		
		/// Database from which we get the statistics
		StatisticsDB *m_db;
		/// Metacontact for which we get the statistics from m_db
		StatisticsContact *m_contact;
		
		/// colors for graphs and calendar
		QColor m_onlineColor;
		QColor m_awayColor;
		QColor m_offlineColor;
		QColor m_backgroundColor;
		QColor m_textColor;
		
		void generatePageFromQStringList(QStringList values, const QString & subTitle);
		
		/// Generates the main page
		void generatePageGeneral();
		/**
		 * @brief Generates the page for a given day of the week.
		 * \param dayOfWeek Monday..Sunday, 0..7
		 */
		void generatePageForDay(const int dayOfWeek);
		void generatePageForMonth(const int monthOfYear);
		
private slots:
		/**
		 * We manage the openUrlRequestDelayed signal from the generalHTMLPart->browserExtension() in order to
		 * generate requested pages on the fly.
		 */
		void slotOpenURLRequest(const KUrl& url, const KParts::OpenUrlArguments &, const KParts::BrowserArguments &);
		
		/**
		 * Color the calendar cells to show statuses across entire months
		 */
		void fillCalendarCells ();
		
		/**
		 * Display details of one day of contact's activity
		 */
		void generateOneDayStats ();

};


#endif // _STATISTICSDIALOG_H
