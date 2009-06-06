// oscarencodingselectiondialog.h
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

#ifndef OSCARENCODINGSELECTIONDIALOG_H
#define OSCARENCODINGSELECTIONDIALOG_H

#include <kdialog.h>
#include "kopete_export.h"

namespace Ui { class OscarEncodingBaseUI; }

class OSCAR_EXPORT OscarEncodingSelectionDialog : public KDialog
{
Q_OBJECT
public:
    explicit OscarEncodingSelectionDialog( QWidget* parent = 0, int initialEncoding = 4);
    ~OscarEncodingSelectionDialog();

    int selectedEncoding() const;

signals:
    void closing( int );

protected slots:
    void slotOk();
    void slotCancel();

private:
    Ui::OscarEncodingBaseUI* m_encodingUI;
    QMap<int, QString> m_encodings;
};

#endif
