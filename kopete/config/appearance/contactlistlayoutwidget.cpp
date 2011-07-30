/*
    ContactList Layout Widget

    Copyright (c) 2009      by Roman Jarosz           <kedgedev@gmail.com>

    Kopete    (c) 2009      by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "contactlistlayoutwidget.h"

#include <QInputDialog>

#include <KMessageBox>

#include "contactlistlayoutmanager.h"

using namespace ContactList;

ContactListLayoutWidget::ContactListLayoutWidget( QWidget *parent )
: QWidget( parent ), mChanged( false ), mLoading( false )
{
	setupUi( this );

	QList<ContactListTokenConfig> tokens = LayoutManager::instance()->tokens();
	for ( int i = 0; i < tokens.size(); i++)
	{
		ContactListTokenConfig clToken = tokens.at( i );
		tokenPool->addToken( new Token( clToken.mName, clToken.mIconName, i ) );
	}

	connect( layoutEdit, SIGNAL(changed()), this, SLOT(emitChanged()) );
	connect( previewButton, SIGNAL(clicked()), this, SLOT(preview()) );
	connect( removeButton, SIGNAL(clicked()), this, SLOT(remove()) );
	connect( layoutComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(setLayout(QString)) );
	connect( LayoutManager::instance(), SIGNAL(layoutListChanged()), this, SLOT(reloadLayoutList()) );
}

void ContactListLayoutWidget::load()
{
	layoutComboBox->clear();

	QStringList layoutNames = LayoutManager::instance()->layouts();
	layoutComboBox->addItems( layoutNames );

	int index = layoutNames.indexOf( LayoutManager::instance()->activeLayoutName() );
	if ( index != -1 )
		layoutComboBox->setCurrentIndex( index );

	setLayout( layoutComboBox->currentText() );
	mChanged = false;
}

bool ContactListLayoutWidget::save()
{
	QString layoutName = mCurrentLayoutName;
	if ( !saveLayoutData( layoutName ) )
		return false;

	LayoutManager::instance()->setActiveLayout( layoutName );
	mChanged = false;
	return true;
}

void ContactListLayoutWidget::emitChanged()
{
	if ( !mChanged && !mLoading )
	{
		mChanged = true;
		emit changed();
	}
}

void ContactListLayoutWidget::setLayout( const QString &layoutName )
{
	if ( mCurrentLayoutName == layoutName )
		return;

	QString layoutNameTmp = mCurrentLayoutName;
	if ( !layoutNameTmp.isEmpty() && !saveLayoutData( layoutNameTmp, true ) )
	{
		int index = layoutComboBox->findText( mCurrentLayoutName );
		if ( index != -1 )
			layoutComboBox->setCurrentIndex( index );

		return;
	}

	mLoading = true;
	mCurrentLayoutName = layoutName;
	removeButton->setEnabled( !LayoutManager::instance()->isDefaultLayout( layoutName ) );
	ContactListLayout layout = LayoutManager::instance()->layout( layoutName );
	layoutEdit->readLayout( layout.layout() );
	mLoading = false;
	mChanged = false;

	// Just emit changed because emitChanged is used only for layout data change
	// and not for change of current active layout
	if ( LayoutManager::instance()->activeLayoutName() != mCurrentLayoutName )
		emit changed();
}

void ContactListLayoutWidget::reloadLayoutList()
{
	disconnect( layoutComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(setLayout(QString)) );

	QString layoutName = layoutComboBox->currentText();
	layoutComboBox->clear();
	layoutComboBox->addItems( LayoutManager::instance()->layouts() );
	int index = layoutComboBox->findText( layoutName );
	if ( index != -1 )
	{
		layoutComboBox->setCurrentIndex( index );
	}
	else
	{
		mCurrentLayoutName.clear();
		setLayout( layoutComboBox->currentText() );
		LayoutManager::instance()->setActiveLayout( layoutComboBox->currentText() );
	}

	connect( layoutComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(setLayout(QString)) );
}

void ContactListLayoutWidget::preview()
{
	ContactListLayout layout;
	layout.setLayout( layoutEdit->config() );
	LayoutManager::instance()->setPreviewLayout( layout );
}

void ContactListLayoutWidget::remove()
{
	if ( !LayoutManager::instance()->isDefaultLayout( mCurrentLayoutName ) )
		LayoutManager::instance()->deleteLayout( mCurrentLayoutName );
}

bool ContactListLayoutWidget::saveLayoutData( QString& layoutName, bool showPrompt )
{
	if ( mChanged )
	{
		if ( showPrompt )
		{
			int ret = KMessageBox::warningYesNoCancel( this, i18n( "Unsaved data?" ), i18n( "Layout" ), KStandardGuiItem::save(),
			                                           KStandardGuiItem::discard(), KStandardGuiItem::cancel(),
			                                           "askRemovingContactOrGroup", KMessageBox::Notify | KMessageBox::Dangerous );
			if ( ret == KMessageBox::Cancel )
				return false;
			else if ( ret == KMessageBox::No )
				return true;
		}

		while ( ContactList::LayoutManager::instance()->isDefaultLayout( layoutName ) ) {
			bool ok = false;
			QString newLayoutName = QInputDialog::getText( this, i18n( "Reserved Layout Name" ),
			                                               i18n( "The layout '%1' is one of the default layouts and cannot be overwritten. Please select a different name.", layoutName ), QLineEdit::Normal, layoutName, &ok );
			if ( !ok )
				return false;
			else if ( !newLayoutName.isEmpty() )
				layoutName = newLayoutName;
		}

		ContactListLayout layout;
		layout.setLayout( layoutEdit->config() );
		return LayoutManager::instance()->addUserLayout( layoutName, layout );
	}
	return true;
}

#include "contactlistlayoutwidget.moc"
