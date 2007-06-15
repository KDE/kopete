#include "generalinfopage.h"
#include "ui_generalcontactinfobase.h"
#include <kopetepropertycontainer.h>
#include <kopeteglobal.h>

#include <KLocale>
#include <QLabel>

namespace Kopete
{

namespace UI
{

class GeneralInfoPage::Private
{
public:
	Private()
	{
		ui = new Ui::GeneralContactInfoBase();
	}
	~Private()
	{
		delete ui;
	}
	Ui::GeneralContactInfoBase *ui;
	QString photoPath;
};

GeneralInfoPage::GeneralInfoPage(const Kopete::PropertyContainer *properties)
: InfoPage(properties)
{
	d = new Private();

	d->ui->setupUi(this);
}

GeneralInfoPage::~GeneralInfoPage()
{
	delete d;
}

void GeneralInfoPage::load()
{
	// nickname
	if (m_properties->hasProperty( Kopete::Global::Properties::self()->nickName().key() ))
	{
		d->ui->nickName->setEnabled(true);
		d->ui->nickName->setText( m_properties->getProperty(Kopete::Global::Properties::self()->nickName()).value().toString() );
	}
	else
	{
		d->ui->nickName->clear();
		d->ui->nickName->setEnabled(false);
	}

	// photo
	if (m_properties->hasProperty( Kopete::Global::Properties::self()->photo().key() ))
	{
		d->photoPath = m_properties->getProperty(Kopete::Global::Properties::self()->photo()).value().toString();
		d->ui->photoLabel->setPixmap( QPixmap(d->photoPath) );
		d->ui->photoLabel->setEnabled(true);
	}
	else
	{
		d->photoPath = QString();
		d->ui->photoLabel->setText( i18n("No photo") );
		d->ui->photoLabel->setEnabled(false);
	}

	// first name
	if (m_properties->hasProperty( Kopete::Global::Properties::self()->firstName().key() ))
	{
		d->ui->firstName->setEnabled(true);
		d->ui->firstName->setText( m_properties->getProperty(Kopete::Global::Properties::self()->firstName()).value().toString() );
	}
	else
	{
		d->ui->firstName->setEnabled(false);
		d->ui->firstName->clear();
	}

	// last name
	if (m_properties->hasProperty( Kopete::Global::Properties::self()->lastName().key() ))
	{
		d->ui->lastName->setEnabled(true);
		d->ui->lastName->setText( m_properties->getProperty(Kopete::Global::Properties::self()->lastName()).value().toString() );
	}
	else
	{
		d->ui->lastName->setEnabled(false);
		d->ui->lastName->clear();
	}

}

void GeneralInfoPage::save()
{
	if (m_properties->hasProperty( Kopete::Global::Properties::self()->nickName().key() ))
		m_properties->setProperty( Kopete::Global::Properties::self()->nickName(), d->ui->nickName->text() );
	
	if (m_properties->hasProperty( Kopete::Global::Properties::self()->photo().key() ))
		m_properties->setProperty( Kopete::Global::Properties::self()->photo(), d->photoPath );

	if (m_properties->hasProperty( Kopete::Global::Properties::self()->firstName().key() ))
		m_properties->setProperty( Kopete::Global::Properties::self()->firstName(), d->ui->firstName->text() );
	
	if (m_properties->hasProperty( Kopete::Global::Properties::self()->lastName().key() ))
		m_properties->setProperty( Kopete::Global::Properties::self()->lastName(), d->ui->lastName->text() );
	
}

QString GeneralInfoPage::pageName() const
{
	return i18n("General Info");
}

/*
void GeneralInfoPage::slotPropertyChanged( Kopete::Contact *contact, const QString &key,
			  const QVariant &oldValue, const QVariant &newValue )
{
	Q_UNUSED(contact);
	Q_UNUSED(key);
	Q_UNUSED(oldValue);
	Q_UNUSED(newValue);
	// nothing to update here
}
*/

} // namespace UI
} // namespace Kopete

#include "generalinfopage.moc"
