#include "contentdialog.h"
#include <QVBoxLayout>
#include <QLabel>

using namespace XMPP;

ContentDialog::ContentDialog()
{
	ui.setupUi(this);
}

ContentDialog::~ContentDialog()
{
	for (int i = 0; i < m_checkBoxes.count(); i++)
	{
		delete m_checkBoxes[i];
	}
}

void ContentDialog::setContents(QList<JingleContent> c)
{
	for (int i = 0; i < c.count(); i++)
	{
		QCheckBox *cb = new QCheckBox(c[i].dataType(), this);
		cb->setChecked(true);
		if (c[i].dataType() == "Unknown Namespace. Join us on #kopete on irc.freenode.org .") //Ouch !
		{
			cb->setChecked(false);
			cb->setEnabled(false);
		}
		m_contentNames << c[i].name();
		ui.verticalLayout->insertWidget(0, cb);
		m_checkBoxes << cb;
	}
	QLabel *label = new QLabel("Check the contents you wanna accept : ", this);
	ui.verticalLayout->insertWidget(0, label);
}

void ContentDialog::setSession(JingleSession *s)
{
	m_session = s;
	setWindowTitle(QString("New Jingle session from ") + s->to().full());
	setContents(s->contents());
}

QStringList ContentDialog::checked()
{
	QStringList ret;
	for (int i = 0; i < m_checkBoxes.count(); i++)
	{
		if (m_checkBoxes[i]->checkState() == Qt::Checked)
		{
			qDebug() << "ContentDialog::checked() : checked : " << m_contentNames.at(i);
			ret << m_contentNames.at(i);
		}
	}
	return ret;
}

QStringList ContentDialog::unChecked()
{
	QStringList ret;
	for (int i = 0; i < m_checkBoxes.count(); i++)
	{
		if (m_checkBoxes[i]->checkState() == Qt::Unchecked)
		{
			qDebug() << "ContentDialog::unChecked() : unchecked : " << m_contentNames.at(i);
			ret << m_contentNames.at(i);
		}
	}
	return ret;
}

JingleSession *ContentDialog::session()
{
	return m_session;
}
