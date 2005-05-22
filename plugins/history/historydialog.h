/*
    kopetehistorydialog.h - Kopete History Dialog

    Copyright (c) 2002 by  Richard Stellingwerff <remenic@linuxfromscratch.org>
    Copyright (c) 2004 by  Stefan Gehn <metz AT gehn.net>

    Kopete    (c) 2002-2004 by the Kopete developers  <kopete-devel@kde.org>

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

#include <qfile.h>
#include <qstringlist.h>

#include <kdialogbase.h>
#include <klistview.h>

#include "kopetemessage.h"

class HistoryViewer;

//class HistoryWidget;
namespace Kopete { class MetaContact; }
namespace Kopete { class XSLT; }
class HistoryLogger;
class KHTMLView;
class KHTMLPart;

class KURL;
namespace KParts { struct URLArgs; class Part; }


class KListViewDateItem;

/**
 * @author Richard Stellingwerff <remenic@linuxfromscratch.org>
 * @author Stefan Gehn <metz AT gehn.net>
 */

class HistoryDialog : public KDialogBase
{
	Q_OBJECT

	public:
		HistoryDialog(Kopete::MetaContact *mc, QWidget* parent=0,
			const char* name="HistoryDialog");
		~HistoryDialog();

		/**
		 * Calls init(Kopete::Contact *c) for each subcontact of the metacontact
		 */
		void init();
		void init(Kopete::Contact *c);

	signals:
		void closing();

	private slots:
		void slotOpenURLRequest(const KURL &url, const KParts::URLArgs &/*args*/);

		// Called when a date is selected in the treeview
		void dateSelected(QListViewItem *);

		void slotSearch();

		// Reinitialise search
		void slotSearchErase();
		void slotSearchTextChanged(const QString& txt); // To enable/disable search button

		void searchFirstStep();
		void searchSecondStep();
		

	private:
		enum Disabled { Prev=1, Next=2 };
		void refreshEnabled( /*Disabled*/ uint disabled );

		/**
		 * Show the messages in the HTML View
		 */
		void setMessages(QValueList<Kopete::Message> m);

		/**
		 * Search if @param item already has @param text child
		 */
		bool hasChild(KListViewItem* item, int month);

		HistoryLogger *mLogger;

		/**
		 * We show history dialog to look at the log for a metacontact. Here is this metacontact.
		 */
		Kopete::MetaContact *mMetaContact;


		// History View
		KHTMLView *mHtmlView;
		KHTMLPart *mHtmlPart;
		HistoryViewer *mMainWidget;
		Kopete::XSLT *mXsltParser;

		struct Search
		{
				typedef QMap<QDate, bool> DateSearchMap;
				DateSearchMap dateSearchMap;

				KListViewDateItem *item;

				int resultMatches;

				bool foundPrevious;
				QDate datePrevious;
		} *mSearch;
};

#endif
