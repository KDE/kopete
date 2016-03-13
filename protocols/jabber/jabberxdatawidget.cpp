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

#include "jabberxdatawidget.h"

#include <QLabel>
#include <QCheckBox>
#include <QComboBox>
#include <QListWidget>
#include <QLineEdit>
#include <QTextEdit>
#include <KDebug>

//-------------------------------------------------------------------------------------------------//
class XDataWidgetField
{
public:
	XDataWidgetField(XMPP::XData::Field f)
	{
		mField = f;
	}

	virtual ~XDataWidgetField()
	{
	}

	QString labelText() const
	{
		return mField.label();
	}

	QString reqText() const
	{
		return "";
	}

	virtual XMPP::XData::Field field() const
	{
		return mField;
	}

	virtual bool isValid() const
	{
		return true;
	}

private:
	XMPP::XData::Field mField;
};


//-------------------------------------------------------------------------------------------------//

class BooleanField : public XDataWidgetField
{
public:
	BooleanField(XMPP::XData::Field f, int row, QWidget *parent, QGridLayout *layout):
	XDataWidgetField(f)
	{
		check = new QCheckBox(parent);
		check->setText(labelText());
		if(f.value().count() > 0)
		{
			QString s = f.value().first();
			if(s == "1" || s == "true" || s == "yes")
				check->setChecked(true);
		}
		layout->addWidget(check, row, 1, 1, 2);

		QLabel *req = new QLabel(reqText(), parent);
		layout->addWidget(req, row, 2);

		if(!f.desc().isEmpty())
		{
			check->setToolTip(f.desc());
			req->setToolTip(f.desc());
		}
	}

	virtual XMPP::XData::Field field() const
	{
		XMPP::XData::Field f = XDataWidgetField::field();
		QStringList val;
		val << QString(check->isChecked() ? "1" : "0");
		f.setValue(val);
		return f;
	}

protected:
	QCheckBox *check;
};

//-------------------------------------------------------------------------------------------------//

class FixedField : public XDataWidgetField
{
public:
	FixedField(XMPP::XData::Field f, int row, QWidget *parent, QGridLayout *layout):
	XDataWidgetField(f)
	{
		QString text;
		const QStringList val = f.value();
		for(QStringList::ConstIterator it = val.begin(); it != val.end(); ++it)
		{
			if(!text.isEmpty())
				text += "<br>";
			text += *it;
		}
		QLabel *label = new QLabel("<qt>" + text + "</qt>", parent);
		label->setWordWrap(true);
		layout->addWidget(label, row, 0, 1, 3);

		if(!f.desc().isEmpty())
			label->setToolTip(f.desc());
	}
};

//-------------------------------------------------------------------------------------------------//

class HiddenField : public XDataWidgetField
{
public:
	HiddenField(XMPP::XData::Field f):
	XDataWidgetField(f)
	{
	}
};

//-------------------------------------------------------------------------------------------------//

class ListSingleField : public XDataWidgetField
{
public:
	ListSingleField(XMPP::XData::Field f, int row, QWidget *parent, QGridLayout *layout):
	XDataWidgetField(f)
	{
		QLabel *label = new QLabel(labelText(), parent);
		layout->addWidget(label, row, 0);

		combo = new QComboBox(parent);
		layout->addWidget(combo, row, 1);
		combo->setInsertPolicy(QComboBox::NoInsert);
		QString sel;
		if (!f.value().isEmpty())
			sel = f.value().first();
		XMPP::XData::Field::OptionList opts = f.options();
		for(XMPP::XData::Field::OptionList::Iterator it = opts.begin(); it != opts.end(); ++it)
		{
			QString lbl = (*it).label;
			if(lbl.isEmpty())
				lbl = (*it).value;
			combo->addItem(lbl);
			if ((*it).value == sel)
				combo->setItemText(combo->currentIndex(), lbl);
		}

		QLabel *req = new QLabel(reqText(), parent);
		layout->addWidget(req, row, 2);

		if(!f.desc().isEmpty())
		{
			label->setToolTip(f.desc());
			combo->setToolTip(f.desc());
			req->setToolTip(f.desc());
		}
	}

	virtual XMPP::XData::Field field() const
	{
		// TODO is value unique?
		QString lbl = combo->currentText();
		XMPP::XData::Field f = XDataWidgetField::field();
		QStringList val;
		XMPP::XData::Field::OptionList opts = f.options();
		XMPP::XData::Field::OptionList::Iterator it = opts.begin();
		for ( ; it != opts.end(); ++it)
			if ( (*it).label == lbl || (*it).value == lbl )
			{
				val << (*it).value;
				break;
			}
		f.setValue(val);
		return f;
	}

private:
	QComboBox *combo;
};

//-------------------------------------------------------------------------------------------------//

class ListMultiField : public XDataWidgetField
{
public:
	ListMultiField(XMPP::XData::Field f, int row, QWidget *parent, QGridLayout *layout):
	XDataWidgetField(f)
	{
		QLabel *label = new QLabel(labelText(), parent);
		layout->addWidget(label, row, 0);

		list = new QListWidget(parent);
		layout->addWidget(list, row, 1);
		list->setSelectionMode(QAbstractItemView::MultiSelection);

		XMPP::XData::Field::OptionList opts = f.options();
		XMPP::XData::Field::OptionList::Iterator it = opts.begin();
		for(; it != opts.end(); ++it)
		{
			QString lbl = (*it).label;
			if(lbl.isEmpty())
				lbl = (*it).value;
			QListWidgetItem *item = new QListWidgetItem(lbl, list);
			QStringList val = f.value();
			QStringList::Iterator sit = val.begin();
			for(; sit != val.end(); sit++)
				if((*it).label == *sit || (*it).value == *sit)
					list->setItemSelected(item, true);
		}

		QLabel *req = new QLabel(reqText(), parent);
		layout->addWidget(req, row, 2);
		if(!f.desc().isEmpty())
		{
			label->setToolTip(f.desc());
			list->setToolTip(f.desc());
			req->setToolTip(f.desc());
		}
	}

	XMPP::XData::Field field() const
	{
		XMPP::XData::Field f = XDataWidgetField::field();
		QStringList val;
		for(int i = 0; i < list->count(); ++i)
		{
			QListWidgetItem *item = list->item(i);
			if(list->isItemSelected(item))
			{
				QString lbl = item->text();
				XMPP::XData::Field::OptionList opts = f.options();
				XMPP::XData::Field::OptionList::Iterator it = opts.begin();
				for(; it != opts.end(); ++it)
				{
					if((*it).label == lbl || (*it).value == lbl)
					{
						val << (*it).value;
						break;
					}
				}
			}
		}
		f.setValue(val);
		return f;
	}

private:
	QListWidget *list;
};

//-------------------------------------------------------------------------------------------------//

class TextMultiField : public XDataWidgetField
{
public:
	TextMultiField(XMPP::XData::Field f, int row, QWidget *parent, QGridLayout *layout):
	XDataWidgetField(f)
	{
		QLabel *label = new QLabel(labelText(), parent);
		layout->addWidget(label, row, 0);

		edit = new QTextEdit(parent);
		layout->addWidget(edit, row, 1);
		QString text;
		const QStringList val = f.value();
		QStringList::ConstIterator it = val.begin();
		for(; it != val.end(); ++it)
		{
			if(!text.isEmpty())
				text += '\n';
			text += *it;
		}
		edit->setText(text);

		QLabel *req = new QLabel(reqText(), parent);
		layout->addWidget(req, row, 2);

		if(!f.desc().isEmpty())
		{
			label->setToolTip(f.desc());
			edit->setToolTip(f.desc());
			req->setToolTip(f.desc());
		}
	}

	virtual XMPP::XData::Field field() const
	{
		XMPP::XData::Field f = XDataWidgetField::field();
		f.setValue(edit->toPlainText().split('\n'));
		return f;
	}

protected:
	QTextEdit *edit;
};

//-------------------------------------------------------------------------------------------------//

class TextSingleField : public XDataWidgetField
{
public:
	TextSingleField(XMPP::XData::Field f, int row, QWidget *parent, QGridLayout *layout):
	XDataWidgetField(f)
	{
		QString text;
		if(f.value().count())
			text = f.value().first();

		QLabel *label = new QLabel(labelText(), parent);
		layout->addWidget(label, row, 0);

		edit = new QLineEdit(parent);
		edit->setText(text);
		layout->addWidget(edit, row, 1);

		QLabel *req = new QLabel(reqText(), parent);
		layout->addWidget(req, row, 2);

		if(!f.desc().isEmpty())
		{
			label->setToolTip(f.desc());
			edit->setToolTip(f.desc());
			req->setToolTip(f.desc());
		}
	}

	virtual XMPP::XData::Field field() const
	{
		XMPP::XData::Field f = XDataWidgetField::field();
		QStringList val;
		val << edit->text();
		f.setValue(val);
		return f;
	}

protected:
	QLineEdit *edit;
};

//-------------------------------------------------------------------------------------------------//

class TextPrivateField : public TextSingleField
{
public:
	TextPrivateField(XMPP::XData::Field f, int row, QWidget *parent, QGridLayout *layout):
	TextSingleField(f, row, parent, layout)
	{
		edit->setEchoMode(QLineEdit::Password);
	}
};

//-------------------------------------------------------------------------------------------------//

class JidMultiField : public TextMultiField
{
public:
	JidMultiField(XMPP::XData::Field f, int row, QWidget *parent, QGridLayout *layout):
	TextMultiField(f, row, parent, layout)
	{
	}

	virtual bool isValid() const
	{
		// TODO
		return true;
	}
};

//-------------------------------------------------------------------------------------------------//

class JidSingleField : public TextSingleField
{
public:
	JidSingleField(XMPP::XData::Field f, int row, QWidget *parent, QGridLayout *layout):
	TextSingleField(f, row, parent, layout)
	{
	}

	virtual bool isValid() const
	{
		// TODO
		return true;
	}
};

//-------------------------------------------------------------------------------------------------//



JabberXDataWidget::JabberXDataWidget(const XMPP::XData &data, QWidget *parent) : QWidget(parent)
{
	const XMPP::XData::FieldList &f = data.fields();
	mFields.clear();
	
	int fields = 0;
	if(!data.instructions().isEmpty())
		fields++;

	if(f.count() == 0 && fields == 0)
		return;

	XMPP::XData::FieldList::ConstIterator it = f.begin();
	for(; it != f.end(); ++it)
	{
		switch((*it).type())
		{
			case XMPP::XData::Field::Field_Boolean:
			case XMPP::XData::Field::Field_Fixed:
			case XMPP::XData::Field::Field_JidMulti:
			case XMPP::XData::Field::Field_JidSingle:
			case XMPP::XData::Field::Field_ListMulti:
			case XMPP::XData::Field::Field_ListSingle:
			case XMPP::XData::Field::Field_TextMulti:
			case XMPP::XData::Field::Field_TextPrivate:
			case XMPP::XData::Field::Field_TextSingle:
				fields++;
				break;
			case XMPP::XData::Field::Field_Hidden:
				break;
		}
	}

	QGridLayout *formLayout = new QGridLayout(parent);
	setLayout(formLayout);

	int row = 0;
	if(!data.instructions().isEmpty())
	{
		QLabel *instr = new QLabel(data.instructions(), parent);
		instr->setWordWrap(true);
		instr->setScaledContents(true);
		instr->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse);
		//connect(instr, SIGNAL(linkActivated(QString)), SLOT(linkActivated(QString)));
		formLayout->addWidget(instr, row, 0, 1, 3);
		row++;
	}
	it = f.begin();
	for(; it != f.end(); ++it, ++row)
	{
		XDataWidgetField *f;
		switch((*it).type())
		{
			case XMPP::XData::Field::Field_Boolean:
				f = new BooleanField(*it, row, this, formLayout);
				break;
			case XMPP::XData::Field::Field_Fixed:
				f = new FixedField(*it, row, this, formLayout);
				break;
			case XMPP::XData::Field::Field_Hidden:
				f = new HiddenField(*it);
				break;
			case XMPP::XData::Field::Field_JidMulti:
				f = new JidMultiField(*it, row, this, formLayout);
				break;
			case XMPP::XData::Field::Field_JidSingle:
				f = new JidSingleField(*it, row, this, formLayout);
				break;
			case XMPP::XData::Field::Field_ListMulti:
				f = new ListMultiField(*it, row, this, formLayout);
				break;
			case XMPP::XData::Field::Field_ListSingle:
				f = new ListSingleField(*it, row, this, formLayout);
				break;
			case XMPP::XData::Field::Field_TextMulti:
				f = new TextMultiField(*it, row, this, formLayout);
				break;
			case XMPP::XData::Field::Field_TextPrivate:
				f = new TextPrivateField(*it, row, this, formLayout);
				break;
			case XMPP::XData::Field::Field_TextSingle:
				f = new TextSingleField(*it, row, this, formLayout);
				break;
		}
		mFields.append(f);
	}
	//formLayout->addStretch(1);
}

JabberXDataWidget::~JabberXDataWidget()
{
}

XMPP::XData::FieldList JabberXDataWidget::fields() const
{
	XMPP::XData::FieldList f;
	for(QList<XDataWidgetField *>::ConstIterator it = mFields.constBegin(); it != mFields.constEnd(); ++it)
		f.append((*it)->field());
	return f;
}

bool JabberXDataWidget::isValid() const
{
	for(QList<XDataWidgetField *>::ConstIterator it = mFields.constBegin(); it != mFields.constEnd(); ++it)
		if(!(*it)->isValid())
			return false;
	return true;
}

#include "jabberxdatawidget.moc"
