#include "gadueditaccount.h"
#include "gaduaccount.h"
#include "gaduprotocol.h"

#include <klineedit.h>


GaduEditAccount::GaduEditAccount( GaduProtocol *proto, KopeteAccount *ident,
                                  QWidget *parent, const char *name )
  : GaduAccountEditUI( parent, name ), EditAccountWidget( ident ),
    account_(0), protocol_( proto )
{
}

bool GaduEditAccount::validateData()
{
  //nothing yet
  return true;
}

KopeteAccount* GaduEditAccount::apply()
{
  if ( !account_ )
    account_ = new GaduAccount( protocol_, loginEdit_->text() );

  account_->setPassword( passwordEdit_->text() );
  return account_;
}
