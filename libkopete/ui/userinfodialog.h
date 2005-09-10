/*
    userinfodialog.h

    Copyright (c) 2003 by Zack Rusin <zack@kde.org>

    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef USERINFODIALOG_H
#define USERINFODIALOG_H

#include <kdialogbase.h>
#include <qstring.h>

#include "kopete_export.h"

class KLineEdit;

namespace Kopete {

	class KOPETE_EXPORT UserInfoDialog : public KDialogBase
	{
		Q_OBJECT
	public:
		UserInfoDialog( const QString& descr );
		virtual ~UserInfoDialog();


		/**
		 * Specifies the look of this dialog. If set to HTML only
		 * KHTMLPart will be in the dialog and it's look can be customized
		 * through setStyleSheet
		 * @see setStyleSheet
		 */
		enum DialogStyle { HTML, Widget };
		void setStyle( DialogStyle style );

		// The functions below set elements as specified in the name.
		// If an element is not set it won't be displayed.
		void setName( const QString& name );
		void setId( const QString& id );
		void setAwayMessage( const QString& msg );
		void setStatus( const QString& status );
		void setWarningLevel(const QString& level  );
		void setOnlineSince( const QString& since );
		void setInfo( const QString& info );
		void setAddress( const QString& addr );
		void setPhone( const QString& phone );

		void addCustomField( const QString& name, const QString& txt );
		void addHTMLText( const QString& str );

		///Shows the dialog
		virtual void show();
	protected:
		/**
		 * This function has to be called after setting all the fields.
		 * It builds the GUI for the dialog. By default show() calls it.
		 */
		virtual void create();
		//Fills the dialog HTML if DialogStyle is HTML
		virtual void fillHTML();
		//Fills the dialog with widgets if DialogStyle is Widget
		virtual void fillWidgets();

		/**
		 * If the DialogStyle is set to HTML one can customize the look of this
		 * dialog by setting the right stylesheet. The CSS id elements that can be
		 * customized include : "name", "id", "warningLevel", "onlineSince",
		 * "address", "phone", "status", "awayMessage" and "info".
		 */
		void setStyleSheet( const QString& css );

		QHBox* addLabelEdit( const QString& label, const QString& text, KLineEdit*& edit );

	private:
		struct UserInfoDialogPrivate;
		UserInfoDialogPrivate *d;
	};

}
#endif
