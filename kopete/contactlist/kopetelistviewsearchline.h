/*
    kopetelistviewsearchline.h - a widget for performing quick searches of Kopete::ListViews

    Copyright (c) 2004      by Richard Smith <kde@metafoo.co.uk>

    Kopete    (c) 2004      by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef KOPETELISTVIEWSEARCHLINE_H
#define KOPETELISTVIEWSEARCHLINE_H

#include <klistviewsearchline.h>

namespace Kopete {
namespace UI {
namespace ListView {

class ListView;

class SearchLine : public KListViewSearchLine
{
	Q_OBJECT
public:
	SearchLine( QWidget *parent, ListView *listView, const char *name = 0 );
	~SearchLine();
	
	void updateSearch( const QString &s );
	
protected:
	virtual void checkItemParentsNotVisible();
	virtual bool checkItemParentsVisible( QListViewItem *it );
	virtual void setItemVisible( QListViewItem *it, bool visible );
	
private:
	QString search;
};

} // end namespace ListView
} // end namespace UI
} // end namespace Kopete

#endif
