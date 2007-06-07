/*
    kopeteeditglobalidentitywidget.h  -  Kopete Edit Global Identity widget

    Copyright (c) 2005-2007 by Michaël Larouche       <larouche@kde.org>

    Kopete    (c) 2002-2007 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef KOPETEEDITGLOBALIDENTITYWIDGET_H
#define KOPETEEDITGLOBALIDENTITYWIDGET_H

#include <QtGui/QWidget>
#include <QtGui/QLabel>

class QSize;

/**
 * @brief This is a simple widget added to a toolbar in KopeteWindow.
 *
 * It can edit the global photo and the global nickname. 
 * When either the photo or the nickname change, it's set the source to Custom.
 * When well connected(signal/slot), it react to the toolbar icon size change.
 *
 * @author Michaël Larouche <larouche@kde.org>
 */
class KopeteEditGlobalIdentityWidget : public QWidget
{
	Q_OBJECT
public:
	KopeteEditGlobalIdentityWidget(QWidget *parent = 0 );
	virtual ~KopeteEditGlobalIdentityWidget();

public slots:
	/**
	 * @brief Change the icon size for the global identity picture.
	 *
	 * @param size New icon size.
	 */
	void setIconSize(const QSize &size);

private:
	/**
	 * Create the internal widgets and signal/slots connections
	 */
	void createGUI();

private slots:
	/**
	 * When a global identity key is changed, update the GUI.
	 */
	void updateGUI(const QString &key, const QVariant &value);
	/**
	 * The photo label was clicked, show a ImageFileDialog.
	 */
	void photoClicked();
	/**
	 * The nickname was changed, display the text in red to display the change.
	 */
	void lineNicknameTextChanged(const QString &text);
	/**
	 * User press Return/Enter in the KLineEdit, commit the new nickname.
	 */
	void changeNickname();

private:
	class Private;
	Private *d;
};

class QMouseEvent;
/**
 * This is a special label that react to click. 
 * Also display a "hand" when hovered.
 *
 * @author Michaël Larouche
 */
class ClickableLabel : public QLabel
{
	Q_OBJECT	
public:
	ClickableLabel(QWidget *parent = 0);

signals:
	void clicked();

protected:
	void mouseReleaseEvent(QMouseEvent *event);
};

#endif
