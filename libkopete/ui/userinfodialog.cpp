#include "userinfodialog.h"

namespace Kopete {

struct UserInfoDialog::UserInfoDialogPrivate {
	QString name;
	QString id;
	QString awayMessage;
	QString status;
	QString warningLevel;
	QString onlineSince;
	QString info;
	QString address;
	QString phone;
	QMap<QString,QString> customFields;
	QVBoxLayout *topLayout;
	DialogStyle style;
	KHTMLPart   *htmlPart;

	KLineEdit *nameEdit;
	KLineEdit *idEdit;
	KLineEdit *statusEdit;
	KLineEdit *warningEdit;
	KLineEdit *onlineEdit;
	KLineEdit *addressEdit;
	KLineEdit *phoneEdit;
	KTextBrowser *awayBrowser;
	KTextBrowser *infoBrowser;
};

UserInfoDialog::UserInfoDialog( const QString& descr )
	: KDialogBase( kapp->mainWidget(), "userinfodialog",
								 true, i18n("User info - ") + descr,
								 KDialogBase::Ok )
{
	d = new UserInfoDialogPrivate;
	QWidget *page = new QWidget( this );
	setMainWidget( page );
	d->topLayout = new QVBoxLayout( page, 0, spacingHint() );
	d->style = HTML;
}

UserInfoDialog::~UserInfoDialog()
{
	delete d; d=0;
}

void UserInfoDialog::setStyle( DialogStyle style )
{
	d->style = style;
}

void UserInfoDialog::setName( const QString& name )
{
	d->name = name;
}

void UserInfoDialog::setId( const QString& id )
{
	d->id = id;
}

void UserInfoDialog::setAwayMessage( const QString& msg )
{
	d->awayMessage = msg;
}

void UserInfoDialog::setStatus( const QString& status )
{
	d->status = status;
}

void UserInfoDialog::setWarningLevel(const QString& level )
{
	d->warningLevel = level;
}

void UserInfoDialog::setOnlineSince( const QString& since )
{
	d->onlineSince = since;
}

void UserInfoDialog::setInfo( const QString& info )
{
	d->info = info;
}

void UserInfoDialog::setAddress( const QString& addr )
{
	d->addr = addr;
}

void UserInfoDialog::setPhone( const QString& phone )
{
	d->phone = phone;
}

void UserInfoDialog::addCustomField( const QString& name, const QString& txt )
{

}

void UserInfoDialog::addHTMLText( const QString& str )
{

}

void UserInfoDialog::addLabelEdit( const QString& label, const QString& text )
{

}

void UserInfoDialog::fillHTML()
{
	d->htmlPart = new KHTMLPart( this );

	QString text = "<html><head></head><body>";

	if ( d->name.isEmpty() ) {
		text.append( "<div id=\"name\"><b>" + i18n("Name : ") + "</b>" );
		text.append( d->name + "</div><br>" );
	}

	if ( d->id.isEmpty() ) {
		text.append( "<div id=\"id\"><b>" + i18n("Id : ") + "</b>" );
		text.append( d->id + "</div><br>" );
	}

	if ( d->warningLevel.isEmpty() ) {
		text.append( "<div id=\"warningLevel\"><b>" + i18n("Warning Level : ") + "</b>" );
		text.append( d->warningLevel + "</div><br>" );
	}

	if ( d->onlineSince.isEmpty() ) {
		text.append( "<div id=\"onlineSince\"><b>" + i18n("Online Since : ") + "</b>" );
		text.append( d->onlineSince + "</div><br>" );
	}

	if ( d->address.isEmpty() ) {
		text.append( "<div id=\"address\"><b>" + i18n("Address : ") + "</b>" );
		text.append( d->address + "</div><br>" );
	}

	if ( d->phone.isEmpty() ) {
		text.append( "<div id=\"phone\"><b>" + i18n("Phone : ") + "</b>" );
		text.append( d->phone + "</div><br>" );
	}

	if ( d->status.isEmpty() ) {
		text.append( "<div id=\"status\"><b>" + i18n("Status : ") + "</b>" );
		text.append( d->status + "</div><br>" );
	}

	if ( d->awayMessage.isEmpty() ) {
		text.append( "<div id=\"awayMessage\"><b>" + i18n("Away Message : ") + "</b>" );
		text.append( d->awayMessage + "</div><br>" );
	}

	if ( d->info.isEmpty() ) {
		text.append( "<div id=\"info\"><b>" + i18n("Info : ") + "</b>" );
		text.append( d->info + "</div><br>" );
	}

	d->htmlPart->begin();
	d->htmlPart->write( text );
	d->htmlPart->end();
}

void UserInfoDialog::fillWidgets()
{

}
