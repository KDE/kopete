// -*- Mode: c++-mode; c-basic-offset: 2; indent-tabs-mode: t; tab-width: 2; -*-
#include "gaduaway.h"
#include "gaduawayui.h"
#include "gaduaccount.h"
#include "gaduprotocol.h"

#include "kopeteonlinestatus.h"

#include <ktextedit.h>
#include <klocale.h>

#include <qbuttongroup.h>
#include <qradiobutton.h>

GaduAway::GaduAway( GaduAccount *account, QWidget* parent,
										const char* name )
	: KDialogBase( parent, name, true, i18n("Away Dialog"),
								 KDialogBase::Ok | KDialogBase::Apply |  KDialogBase::Cancel,
								 KDialogBase::Ok, true ), account_( account )
{
	ui_ = new GaduAwayUI( this );
	setMainWidget( ui_ );
	connect( this, SIGNAL(applyClicked()), SLOT(slotApply()) );
}

int GaduAway::status() const
{
	return ui_->statusGroup_->selectedId();
}

QString GaduAway::awayText() const
{
	return ui_->textEdit_->text();
}

void GaduAway::slotApply()
{
	if ( account_ )
		account_->changeStatus( GaduProtocol::protocol()->convertStatus( status() ),
														awayText() );
}

#include "gaduaway.moc"
