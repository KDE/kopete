//
// C++ Interface: groupwisecontactproperties
//
// Description: 
//
//
// Author: SUSE AG <>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef GROUPWISECONTACTPROPERTIES_H
#define GROUPWISECONTACTPROPERTIES_H


#include <qobject.h>

class GroupWiseContactPropsWidget;
class KDialogBase;

/**
Logic, wrapping UI, for displaying contact properties

@author SUSE AG
*/
class GroupWiseContactProperties : public QObject
{
Q_OBJECT
public:
	/**
	 * Display properties given a GroupWiseContact
	 */ 
	GroupWiseContactProperties( GroupWiseContact * contact, QObject *parent, const char *name );
	/**
	 * Display properties given a GroupWise::ContactDetails
	 */
	GroupWiseContactProperties( GroupWise::ContactDetails contactDetails, QObject *parent = 0, const char *name = 0 );
	~GroupWiseContactProperties();

private:
	GroupWiseContactPropsWidget * m_propsWidget;
	KDialogBase * m_dialog;
};

#endif
