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

#include <k3listviewsearchline.h>
#include <kopete_export.h>

namespace Kopete {
namespace UI {
namespace ListView {

class ListView;

class KOPETE_EXPORT SearchLine : public K3ListViewSearchLine
{
	Q_OBJECT
public:
	/**
	 * Constructs a SearchLine with \a listView being the
	 * ListView to be filtered.
	 *
	 * If \a listView is null then the widget will be disabled until a listview
	 * is set with setListView().
	 */
	explicit SearchLine( QWidget *parent = 0, ListView *listView = 0 );
	/**
	 * Destroys the SearchLine.
	 */
	~SearchLine();
	
	void updateSearch( const QString &s );
	
protected:
	virtual void checkItemParentsNotVisible();
	virtual bool checkItemParentsVisible( Q3ListViewItem *it );
	virtual void setItemVisible( Q3ListViewItem *it, bool visible );
	
private:
	QString search;
	bool searchEmpty;
};

} // end namespace ListView
} // end namespace UI
} // end namespace Kopete

#endif
