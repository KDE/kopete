
#ifndef AIMADDCONTACTPAGE_H
#define AIMADDCONTACTPAGE_H

#include <qwidget.h>
#include <qlabel.h>
#include "addcontactpage.h"

namespace Ui { class aimAddContactUI; }
namespace Kopete
{
class Account; 
class MetaContact;
}

class AIMAddContactPage : public AddContactPage
{
Q_OBJECT

public:
	explicit AIMAddContactPage(bool connected, QWidget *parent=0);
	~AIMAddContactPage();

	/** Validates the data entered */
	bool validateData() Q_DECL_OVERRIDE;
	/** Applies the addition to the account */
	bool apply( Kopete::Account *account, Kopete::MetaContact *) Q_DECL_OVERRIDE;

protected:
	/** The actual GUI */
	Ui::aimAddContactUI *m_gui;
	bool canadd;
};
#endif

//kate: tab-width 4; indent-mode csands;

