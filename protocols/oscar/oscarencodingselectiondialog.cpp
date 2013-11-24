// oscarencodingselectiondialog.cpp

// Copyright (C)  2005  Matt Rogers <mattr@kde.org>

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
// 02110-1301, USA.

#include "oscarencodingselectiondialog.h"
#include "ui_oscarencodingselectionbase.h"

#include <kdebug.h>
#include <klocale.h>

OscarEncodingSelectionDialog::OscarEncodingSelectionDialog( QWidget* parent, int initialEncoding )
    : KDialog( parent )
{
	setCaption( i18n( "Select Encoding" ) );
	setButtons( KDialog::Ok | KDialog::Cancel );
    int initialEncodingIndex;

    setAttribute( Qt::WA_DeleteOnClose, false );

	QWidget* w = new QWidget( this );
	m_encodingUI = new Ui::OscarEncodingBaseUI;
	m_encodingUI->setupUi( w );

    //fill the encoding combo boxes
  	m_encodings.insert(0, i18n("Default"));
  	m_encodings.insert(2026, i18n("Big5"));
	m_encodings.insert(2101, i18n("Big5-HKSCS"));
	m_encodings.insert(18, i18n("euc-JP Japanese"));
	m_encodings.insert(38, i18n("euc-KR Korean"));
	m_encodings.insert(57, i18n("GB-2312 Chinese"));
	m_encodings.insert(113, i18n("GBK Chinese"));
	m_encodings.insert(114, i18n("GB18030 Chinese"));

	m_encodings.insert(16, i18n("JIS Japanese"));
	m_encodings.insert(17, i18n("Shift-JIS Japanese"));

	m_encodings.insert(2084, i18n("KOI8-R Russian"));
	m_encodings.insert(2088, i18n("KOI8-U Ukrainian"));

	m_encodings.insert(4, i18n("ISO-8859-1 Western"));
	m_encodings.insert(5, i18n("ISO-8859-2 Central European"));
	m_encodings.insert(6, i18n("ISO-8859-3 Central European"));
	m_encodings.insert(7, i18n("ISO-8859-4 Baltic"));
	m_encodings.insert(8, i18n("ISO-8859-5 Cyrillic"));
	m_encodings.insert(9, i18n("ISO-8859-6 Arabic"));
	m_encodings.insert(10, i18n("ISO-8859-7 Greek"));
	m_encodings.insert(11, i18n("ISO-8859-8 Hebrew, visually ordered"));
	m_encodings.insert(85, i18n("ISO-8859-8-I Hebrew, logically ordered"));
	m_encodings.insert(12, i18n("ISO-8859-9 Turkish"));
	m_encodings.insert(13, i18n("ISO-8859-10"));
	m_encodings.insert(109, i18n("ISO-8859-13"));
	m_encodings.insert(110, i18n("ISO-8859-14"));
	m_encodings.insert(111, i18n("ISO-8859-15 Western"));

	m_encodings.insert(2250, i18n("Windows-1250 Central European"));
	m_encodings.insert(2251, i18n("Windows-1251 Cyrillic"));
	m_encodings.insert(2252, i18n("Windows-1252 Western"));
	m_encodings.insert(2253, i18n("Windows-1253 Greek"));
	m_encodings.insert(2254, i18n("Windows-1254 Turkish"));
	m_encodings.insert(2255, i18n("Windows-1255 Hebrew"));
	m_encodings.insert(2256, i18n("Windows-1256 Arabic"));
	m_encodings.insert(2257, i18n("Windows-1257 Baltic"));
	m_encodings.insert(2258, i18n("Windows-1258 Viet Nam"));

	m_encodings.insert(2009, i18n("IBM 850"));
	m_encodings.insert(2085, i18n("IBM 866"));

	m_encodings.insert(2259, i18n("TIS-620 Thai"));

	m_encodings.insert(106, i18n("UTF-8 Unicode"));
	m_encodings.insert(1015, i18n("UTF-16 Unicode"));

	m_encodingUI->encodingCombo->insertItems( 0, m_encodings.values() );
	if( (initialEncodingIndex = m_encodings.keys().indexOf(initialEncoding)) == -1 )
    {
        kWarning() << "Requested encoding mib " << initialEncoding
                << " not in encoding list - defaulting to first encoding item"
                << " in list to be shown in combobox initially" << endl;
        /* initialEncodingIndex = position in combobox, value 0 currently
         * corresponds to ISO-8859-1, generally to the first item in combobox,
         * which usually is the default
         */
        initialEncodingIndex = 0;
    }
    m_encodingUI->encodingCombo->setCurrentIndex( initialEncodingIndex );
    setMainWidget( w );
    connect(this,SIGNAL(okClicked()),this,SLOT(slotOk()));
    connect(this,SIGNAL(cancelClicked()),this,SLOT(slotCancel()));
}

OscarEncodingSelectionDialog::~OscarEncodingSelectionDialog()
{
    delete m_encodingUI;
}

int OscarEncodingSelectionDialog::selectedEncoding() const
{
    QString encoding = m_encodingUI->encodingCombo->currentText();
	int mib = m_encodings.keys()[ m_encodings.values().indexOf(encoding) ];
    
    if( mib == -1 )
        return 0;
    return mib;
}

void OscarEncodingSelectionDialog::slotOk()
{
    emit closing( QDialog::Accepted );
}

void OscarEncodingSelectionDialog::slotCancel()
{
    emit closing( QDialog::Rejected );
}

#include "oscarencodingselectiondialog.moc"

