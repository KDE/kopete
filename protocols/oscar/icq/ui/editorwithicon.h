/*
    editorwithicon.h  -  Editor With Icon

    Copyright (c) 2007 by Roman Jarosz <kedgedev@centrum.cz>
    Kopete    (c) 2007 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef EDITORWITHICON_H
#define EDITORWITHICON_H

#include <QWidget>
#include <QList>

class QToolButton;
class QLineEdit;

/**
* A one-line text editor with icon.
*
* @author Roman Jarosz <kedgedev@centrum.cz>
*/

class EditorWithIcon : public QWidget
{
	Q_OBJECT

public:
	/**
	 * Constructs a new one-line text editor with icon
	 *
	 * @param icons The list of icons user can choose from.
	 * @param parent The parent of the new widget
	 */
	explicit EditorWithIcon( const QList<QIcon> &icons, QWidget *parent = 0 );

	/** Returns the icons user can choose from */
	QList<QIcon> icons() const { return mIcons; }

	/** Sets editor's text */
	void setText( const QString &text );

	/** Returns editor's text */
	QString text() const;

	/** Returns selected icon index */
	int iconIndex() const { return mIconIndex; }

public slots:
	/** Sets selected icon index */
	void setIconIndex( int index );

private slots:
	/** Popups widget with icons */
	void popupIcons();

private:
	QList<QIcon> mIcons;
	int mIconIndex;

	QToolButton *mIconButton;
	QLineEdit *mLineEdit;
};

#endif
