// userinfodialog.h
//
// Copyright (C)  2003  Zack Rusin <zack@kde.org>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.

#include <kdialogbase.h>
#include <qstring.h>

class KLineEdit;

namespace Kopete {

	class UserInfoDialog : public KDialogBase
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

