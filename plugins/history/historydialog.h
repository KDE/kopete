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

class DMPair
{
	public:
		DMPair() {md = QDate(0, 0, 0); mc = 0; }
		DMPair(QDate d, Kopete::MetaContact *c) { md = d; mc =c; }
		QDate date() const { return md; }
		Kopete::MetaContact* metaContact() const { return mc; }
		bool operator==(const DMPair p1) const { return p1.date() == this->date() && p1.metaContact() == this->metaContact(); }
	private:
		QDate md;
		Kopete::MetaContact *mc;
};

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
		void slotContactChanged(int index);
		void slotFilterChanged(int index);

		void init();
		void slotLoadDays();

		void slotRightClick(const QString &url, const QPoint &point);
		void slotCopy();
		void slotCopyURL();

	private:
		enum Disabled { Prev=1, Next=2 };
		void refreshEnabled( /*Disabled*/ uint disabled );

		void initProgressBar(const QString& text, int nbSteps);
		void doneProgressBar();
		void init(Kopete::MetaContact *mc);
		void init(Kopete::Contact *c);

		/**
		 * Show the messages in the HTML View
		 */
		void setMessages(QValueList<Kopete::Message> m);

		void listViewShowElements(bool s);

		/**
		 * Search if @param item already has @param text child
		 */
		bool hasChild(KListViewItem* item, int month);

		/**
		 * We show history dialog to look at the log for a metacontact. Here is this metacontact.
		 */
		Kopete::MetaContact *mMetaContact;

		QPtrList<Kopete::MetaContact> mMetaContactList;

		// History View
		KHTMLView *mHtmlView;
		KHTMLPart *mHtmlPart;
		HistoryViewer *mMainWidget;
		Kopete::XSLT *mXsltParser;

		struct Init
		{
			QValueList<DMPair> dateMCList; // mc for MetaContact
		} mInit;

		bool mSearching;

		KAction *mCopyAct;
		KAction *mCopyURLAct;
		QString mURL;
};

#endif
