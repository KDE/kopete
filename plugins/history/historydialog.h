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

#include <kdialogbase.h>
#include <qstringlist.h>

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

/**
 * @author Richard Stellingwerff <remenic@linuxfromscratch.org>
 * @author Stefan Gehn <metz AT gehn.net>
 */

class HistoryDialog : public KDialogBase
{
	Q_OBJECT

	public:
		HistoryDialog(Kopete::MetaContact *mc, int count=50, QWidget* parent=0,
			const char* name="HistoryDialog");

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
		* << button clicked
		*/
		void slotBackClicked();

		/**
		* >> button clicked
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

		void slotOpenURLRequest(const KURL &url, const KParts::URLArgs &/*args*/);

	private:
		enum Disabled { Prev=1, Next=2 };
		void refreshEnabled( /*Disabled*/ uint disabled );

		void setMessages(QValueList<Kopete::Message> m);

		// amount of entries to read at once
		unsigned int msgCount;

		HistoryLogger *mLogger;
		Kopete::MetaContact *mMetaContact;
		// History View
		KHTMLView *mHtmlView;
		KHTMLPart *mHtmlPart;
		HistoryViewer *mMainWidget;
		Kopete::XSLT *mXsltParser;
};

#endif
