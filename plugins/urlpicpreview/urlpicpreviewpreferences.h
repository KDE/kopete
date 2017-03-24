/*
    urlpicpreviewpreferences.h

    Copyright (c) 2005      by Heiko Schaefer        <heiko@rangun.de>

    Kopete    (c) 2002-2007 by the Kopete developers <kopete-devel@kde.org>

    ************************************************************************ *
    *                                                                        *
    * This program is free software; you can redistribute it and/or modify   *
    * it under the terms of the GNU General Public License as published by   *
    * the Free Software Foundation; version 2, or (at your option) version 3 *
    * of the License.                                                        *
    *                                                                        *
    **************************************************************************
*/

#ifndef URLPICPREVIEWPREFERENCES_H
#define URLPICPREVIEWPREFERENCES_H

#include <kcmodule.h>

class QLayout;
namespace Ui { class URLPicPreviewPrefsUI; }

class URLPicPreviewPreferences : public KCModule
{
		Q_OBJECT

		URLPicPreviewPreferences ( const URLPicPreviewPreferences& );
		URLPicPreviewPreferences& operator= ( const URLPicPreviewPreferences& );

	public:
		explicit URLPicPreviewPreferences ( QWidget* parent = nullptr, const QVariantList& args = QVariantList() );

		virtual ~URLPicPreviewPreferences();
		void load() Q_DECL_OVERRIDE;
		void save() Q_DECL_OVERRIDE;
		void defaults() Q_DECL_OVERRIDE;

	private:
		QLayout * m_layout;
		Ui::URLPicPreviewPrefsUI * m_ui;
};

#endif /* URLPICPREVIEWPREFERENCES_H */

