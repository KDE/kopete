/*
    kopetehistory2dialog.h - Kopete History2 Dialog

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

#ifndef HISTORYDIALOG_H
#define HISTORYDIALOG_H

#include <QtCore/QList>

#include <kdialog.h>
#include <kurl.h>

#include "kopetemessage.h"

class QTreeWidgetItem;

class KAction;
class KHTMLView;
class KHTMLPart;
namespace KParts { class BrowserArguments; class OpenUrlArguments; class Part; }

namespace Kopete { class MetaContact; }
namespace Kopete { class Contact; }
namespace Kopete { class XSLT; }

namespace Ui { class History2Viewer; }

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
class History2Dialog : public KDialog
{
	Q_OBJECT

	public:
		explicit History2Dialog(Kopete::MetaContact *mc, QWidget* parent=0);
		~History2Dialog();

		/**
		 * Calls init(Kopete::Contact *c) for each subcontact of the metacontact
		 */


	signals:
		void closing();

	private slots:
		void slotOpenURLRequest(const KUrl &url, const KParts::OpenUrlArguments &, const KParts::BrowserArguments &);

		// Called when a date is selected in the treeview
		void dateSelected(QTreeWidgetItem *);

		void slotSearch();
		void searchFinished();

		void slotSearchTextChanged(const QString& txt); // To enable/disable search button
		void slotContactChanged(int index);
		void slotFilterChanged(int index);

		void init(QString s);

		void slotRightClick(const QString &url, const QPoint &point);
		void slotCopy();
		void slotCopyURL();

		void slotImportHistory2();

	private:
		enum Disabled { Prev=1, Next=2 };
		void refreshEnabled( /*Disabled*/ uint disabled );

		void initProgressBar(const QString& text, int nbSteps);
		void doneProgressBar();

		/**
		 * Show the messages in the HTML View
		 */
		void setMessages(QList<Kopete::Message> m);

		void treeWidgetHideElements(bool s);

		QString highlight(const QString &htmlText, const QString &highlight) const;
		QString escapeXMLText(const QString& text) const;

		/**
		 * We show history2 dialog to look at the log for a metacontact. Here is this metacontact.
		 */
		Kopete::MetaContact *mMetaContact;

		QList<Kopete::MetaContact*> mMetaContactList;

		// History2 View
		KHTMLView *mHtmlView;
		KHTMLPart *mHtmlPart;
		Ui::History2Viewer *mMainWidget;
		Kopete::XSLT *mXsltParser;

		struct Init
		{
			QList<DMPair> dateMCList; // mc for MetaContact
		} mInit;

		bool mSearching;

		KAction *mCopyAct;
		KAction *mCopyURLAct;
		QString mURL;
};

#endif
