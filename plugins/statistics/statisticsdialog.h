/*
    statisticsdialog.h - Kopete History Dialog

    Copyright (c) 2003-2004 by Marc Cramdal        <marc.cramdal@gmail.com>

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
#include <kdialogbase.h>
#include "kopetemetacontact.h"

class QCanvasView;
class QCanvas;
class QStringList;

class StatisticsWidget;
class StatisticsPlugin;
class StatisticsDB;
class StatisticsContact;

class KHTMLPart;
class KURL;
namespace KParts
{
	class URLArgs;
}

class StatisticsDialog : public KDialogBase
{
	Q_OBJECT
	public:
		StatisticsDialog(StatisticsContact *contact, StatisticsDB* db, QWidget* parent=0,
			const char* name="StatisticsDialog");
	private:
		QString generateHTMLChart(const int *hours, const int *hours2, const int *hours3, const QString & caption, const QString & color);
		QString generateHTMLChartBar(int height, const QString & color, const QString & caption);
		QString stringFromSeconds(const int seconds);
		
		StatisticsWidget *mainWidget;
		KHTMLPart *generalHTMLPart;
		
		/// Database from which we get the statistics
		StatisticsDB *m_db;
		/// Metacontact for which we get the statistics from m_db
		StatisticsContact *m_contact;
		
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
		 * We manage the openURLRequestDelayed signal from the generalHTMLPart->browserExtension() in order to
		 * generate requested pages on the flow.
		 */
		void slotOpenURLRequest(const KURL& url, const KParts::URLArgs&);
		void slotAskButtonClicked();

};


#endif // _STATISTICSDIALOG_H
