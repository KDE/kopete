#include <klocale.h>

#include <qstring.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>

#include "gaduadd.h"
#include "gaduaccount.h"
#include "gaduaddcontactpage.h"

GaduAddContactPage::GaduAddContactPage( GaduAccount* owner,
                                        QWidget* parent, const char* name )
    :AddContactPage( parent, name )
{
    (new QVBoxLayout(this))->setAutoAdd(true);
    if( owner->isConnected() ) {
        addUI_ = new gaduAddUI( this );
        account_ = owner;
        canAdd_ = true;
    } else {
        noaddMsg1_ = new QLabel(i18n("You need to be connected to be able to add contacts."), this);
        noaddMsg2_ = new QLabel(i18n("Connect to the MSN network and try again."), this);
        canAdd_ = false;
    }
}

GaduAddContactPage::~GaduAddContactPage()
{
}

bool
GaduAddContactPage::validateData()
{
    bool ok;
    addUI_->addEdit_->text().toULong( &ok );
    return ok;
}

void
GaduAddContactPage::slotFinish(KopeteMetaContact *)
{	//FIXME: use the metacontat!!!
    if ( canAdd_ ) {
        if ( validateData() ) {
            QString userid = addUI_->addEdit_->text();
            account_->addContact( userid, userid );
        }
    } else {
        return;
    }
}

#include "gaduaddcontactpage.moc"

/*
 * Local variables:
 * c-indentation-style: bsd
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 *
 * vim: set et ts=4 sts=4 sw=4:
 */
