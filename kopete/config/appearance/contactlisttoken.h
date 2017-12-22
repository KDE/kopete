/*
    Kopete ContactList Token

    Copyright (c) 2009 by Roman Jarosz <kedgedev@gmail.com>

    Kopete    (c) 2009 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
#ifndef CONTACTLISTTOKEN_H
#define CONTACTLISTTOKEN_H

#include <TokenWithLayout.h>


class ContactListTokenFactory : public TokenFactory
{
public:
	Token * createToken( const QString &text, const QString &iconName, int value, QWidget *parent = nullptr ) Q_DECL_OVERRIDE;
};

class ContactListToken : public TokenWithLayout
{
	Q_OBJECT
public:
	ContactListToken( const QString &text, const QString &iconName, int value, QWidget *parent = nullptr );

	bool small() const;
	void setSmall( bool small );

	bool optimalSize() const;
	void setOptimalSize( bool optimalSize );

protected:
	void fillMenu( QMenu * menu ) Q_DECL_OVERRIDE;
	void menuExecuted( const QAction* action ) Q_DECL_OVERRIDE;

private:
	bool m_small;
	bool m_optimalSize;
};

#endif
