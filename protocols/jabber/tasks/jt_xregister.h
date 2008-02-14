 /*
    Copyright (c) 2008 by Igor Janssen  <alaves17@gmail.com>

    Kopete    (c) 2008 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
 */

#ifndef JT_XREGISTER_H
#define JT_XREGISTER_H

#include "xmpp_tasks.h"
#include "xmpp_xdata.h"

using namespace XMPP;

class JT_XRegister : public JT_Register
{
	Q_OBJECT
public:
	JT_XRegister(Task *parent);

	void setXForm(const Form &frm, const XData &_form);

	bool take(const QDomElement &);
	QDomElement iq() const;
	QDomElement xdataElement() const;

	void onGo();

private:
	QDomElement _iq;
};

#endif
