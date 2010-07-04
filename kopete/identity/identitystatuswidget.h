/*
    identitystatuswidget.h - Kopete Identity Status configuration widget

    Copyright (c) 2007-2009 by Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>
    Copyright (c) 2007      by Will Stephenson <wstephenson@kde.org>

    Kopete    (c) 2002-2009 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef IDENTITYSTATUSWIDGET_H
#define IDENTITYSTATUSWIDGET_H

#include <QWidget>
#include <QListWidgetItem>
#include <kopete_export.h>

namespace Kopete
{
class Account;
class Contact;
class Identity;
class PropertyContainer;
}

/**
 * @author Gustavo Pichorim Boiko <gustavo.boiko@kemail.net>
 *
 * This dialog is used to add a new identity to 
 */
class KOPETE_IDENTITY_EXPORT IdentityStatusWidget : public QWidget
{
	Q_OBJECT

public:
	/**
	 * @brief Default constructor for the identity status widget
	 */
	explicit IdentityStatusWidget( Kopete::Identity *ident, QWidget *parent = 0 );
	~IdentityStatusWidget();

	/**
	 * This method returns the identity currently being managed by this widget
	 */
	Kopete::Identity *identity() const;
	void setIdentity(Kopete::Identity *identity);

	virtual void setVisible(bool visible);

protected:
	bool eventFilter( QObject *watched, QEvent *event );

private:
	/**
	 * Initialise the widget's contents
	 */
	void load();

private slots:
	void slotAnimate(qreal amount);

	void showAccountContextMenu( const QPoint & );
	void slotAccountClicked( QListWidgetItem * item );
	void slotAccountDoubleClicked( QListWidgetItem * item );
	void slotPhotoClicked();
	void slotAccountRegistered( Kopete::Account *account );
	void slotAccountUnregistered( const Kopete::Account *account );
	void slotAccountStatusIconChanged( Kopete::Contact *contact );
	/**
	 * Handle departing identities
	 */
	void slotIdentityUnregistered( const Kopete::Identity* identity );

	/**
	 * Handle property changes in identities
	 */
	void slotIdentityPropertyChanged(Kopete::PropertyContainer *container );
	void slotIdentityChanged(Kopete::Identity *identity);

private:
	void addAccountItem( Kopete::Account *account );
	void resizeAccountListWidget();
	class Private;
	Private * const d;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

