/*
    appearanceconfig.cpp  -  Kopete Look Feel Config

    Copyright (c) 2001-2002 by Duncan Mac-Vicar Prett <duncan@kde.org>
    Kopete    (c) 2002-2004 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "appearanceconfig.h"
#include "appearanceconfig_emoticons.h"
#include "appearanceconfig_chatwindow.h"
#include "appearanceconfig_colors.h"
#include "appearanceconfig_contactlist.h"

#include "tooltipeditdialog.h"

#include <qcheckbox.h>
#include <qdir.h>
#include <qlayout.h>
#include <qhbuttongroup.h>
#include <qspinbox.h>
#include <qslider.h>
#include <qlabel.h>

#include <kdeversion.h>
#include <kinputdialog.h>

#include <kapplication.h>
#include <kcolorcombo.h>
#include <kcolorbutton.h>
#include <kdebug.h>
#include <kfontrequester.h>
#include <kgenericfactory.h>
#include <khtmlview.h>
#include <khtml_part.h>
#include <kio/netaccess.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpushbutton.h>
#include <kstandarddirs.h>
#include <klibloader.h>
#include <ktrader.h>
#include <ktextedit.h>
#include <kurlrequesterdlg.h>

#include <ktexteditor/highlightinginterface.h>
#include <ktexteditor/editinterface.h>
#include <ktexteditor/document.h>
#include <ktexteditor/view.h>

#include "kopeteprefs.h"
#include "kopetemessage.h"
#include "styleeditdialog.h"
#include "kopetexsl.h"
#include "kopetecontact.h"
#include "kopeteemoticons.h"
#include "kopeteglobal.h"

#include <qtabwidget.h>

typedef KGenericFactory<AppearanceConfig, QWidget> KopeteAppearanceConfigFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_kopete_appearanceconfig, KopeteAppearanceConfigFactory( "kcm_kopete_appearanceconfig" ) )

class KopeteAppearanceConfigPrivate
{
public:
	KopeteXSLT *xsltParser;
};

AppearanceConfig::AppearanceConfig(QWidget *parent, const char* /*name*/, const QStringList &args )
: KCModule( KopeteAppearanceConfigFactory::instance(), parent, args )
{
	editedItem = 0L;

	d = new KopeteAppearanceConfigPrivate;

	d->xsltParser = new KopeteXSLT( KopetePrefs::prefs()->styleContents(), this );

	(new QVBoxLayout(this))->setAutoAdd(true);
	mAppearanceTabCtl = new QTabWidget(this, "mAppearanceTabCtl");


	// "Emoticons" TAB ==========================================================
	mPrfsEmoticons = new AppearanceConfig_Emoticons(mAppearanceTabCtl);
	connect(mPrfsEmoticons->chkUseEmoticons, SIGNAL(toggled(bool)),
		this, SLOT(slotUseEmoticonsChanged(bool)));
	connect(mPrfsEmoticons->icon_theme_list, SIGNAL(selectionChanged()),
		this, SLOT(slotSelectedEmoticonsThemeChanged()));
	connect(mPrfsEmoticons->btnInstallTheme, SIGNAL(clicked()),
		this, SLOT(installNewTheme()));
	connect(mPrfsEmoticons->btnRemoveTheme, SIGNAL(clicked()),
		this, SLOT(removeSelectedTheme()));

	mAppearanceTabCtl->addTab(mPrfsEmoticons, i18n("&Emoticons"));

	// "Chat Window" TAB ========================================================
	mPrfsChatWindow = new AppearanceConfig_ChatWindow(mAppearanceTabCtl);
	connect(mPrfsChatWindow->mTransparencyEnabled, SIGNAL(toggled(bool)),
		this, SLOT(slotTransparencyChanged(bool)));
	connect(mPrfsChatWindow->styleList, SIGNAL(selectionChanged(QListBoxItem *)),
		this, SLOT(slotStyleSelected()));
	connect(mPrfsChatWindow->addButton, SIGNAL(clicked()),
		this, SLOT(slotAddStyle()));
	connect(mPrfsChatWindow->editButton, SIGNAL(clicked()),
		this, SLOT(slotEditStyle()));
	connect(mPrfsChatWindow->deleteButton, SIGNAL(clicked()),
		this, SLOT(slotDeleteStyle()));
	connect(mPrfsChatWindow->importButton, SIGNAL(clicked()),
		this, SLOT(slotImportStyle()));
	connect(mPrfsChatWindow->copyButton, SIGNAL(clicked()),
		this, SLOT(slotCopyStyle()));

	connect(mPrfsChatWindow->mTransparencyTintColor, SIGNAL(activated (const QColor &)),
		this, SLOT(emitChanged()));
	connect(mPrfsChatWindow->mTransparencyValue, SIGNAL(valueChanged(int)),
		this, SLOT(emitChanged()));

	mPrfsChatWindow->htmlFrame->setFrameStyle(QFrame::WinPanel | QFrame::Sunken);
	QVBoxLayout *l = new QVBoxLayout(mPrfsChatWindow->htmlFrame);
	preview = new KHTMLPart(mPrfsChatWindow->htmlFrame, "preview");
	preview->setJScriptEnabled(false);
	preview->setJavaEnabled(false);
	preview->setPluginsEnabled(false);
	preview->setMetaRefreshEnabled(false);
	KHTMLView *htmlWidget = preview->view();
	htmlWidget->setMarginWidth(4);
	htmlWidget->setMarginHeight(4);
	htmlWidget->setFocusPolicy(NoFocus);
	htmlWidget->setSizePolicy(
		QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
	l->addWidget(htmlWidget);

	mAppearanceTabCtl->addTab( mPrfsChatWindow, i18n("Chat Window") );

	// "Contact List" TAB =======================================================
	mPrfsContactList = new AppearanceConfig_ContactList(mAppearanceTabCtl);
	connect(mPrfsContactList->mTreeContactList, SIGNAL(toggled(bool)),
		this, SLOT(emitChanged()));
	connect(mPrfsContactList->mSortByGroup, SIGNAL(toggled(bool)),
		this, SLOT(emitChanged()));
	connect(mPrfsContactList->mEditTooltips, SIGNAL(clicked()),
		this, SLOT(slotEditTooltips()));
	connect(mPrfsContactList->mIndentContacts, SIGNAL(toggled(bool)),
		this, SLOT(emitChanged()));
	connect(mPrfsContactList->mDisplayMode, SIGNAL(clicked(int)),
		this, SLOT(emitChanged()));
	connect(mPrfsContactList->mAnimateChanges, SIGNAL(toggled(bool)),
		this, SLOT(emitChanged()));
	connect(mPrfsContactList->mFadeVisibility, SIGNAL(toggled(bool)),
		this, SLOT(emitChanged()));
	connect(mPrfsContactList->mFoldVisibility, SIGNAL(toggled(bool)),
		this, SLOT(emitChanged()));

	// don't enable the checkbox if XRender is not available
	mPrfsContactList->mFadeVisibility->setEnabled( HAVE_XRENDER );

	mAppearanceTabCtl->addTab(mPrfsContactList, i18n("Contact List"));

	// "Colors and Fonts" TAB ===================================================
	mPrfsColors = new AppearanceConfig_Colors(mAppearanceTabCtl);
	connect(mPrfsColors->foregroundColor, SIGNAL(changed(const QColor &)),
		this, SLOT(slotHighlightChanged()));
	connect(mPrfsColors->backgroundColor, SIGNAL(changed(const QColor &)),
		this, SLOT(slotHighlightChanged()));
	connect(mPrfsColors->fontFace, SIGNAL(fontSelected(const QFont &)),
		this, SLOT(slotChangeFont()));
	connect(mPrfsColors->textColor, SIGNAL(changed(const QColor &)),
		this, SLOT(slotUpdatePreview()));
	connect(mPrfsColors->bgColor, SIGNAL(changed(const QColor &)),
		this, SLOT(slotUpdatePreview()));
	connect(mPrfsColors->linkColor, SIGNAL(changed(const QColor &)),
		this, SLOT(slotUpdatePreview()));
	connect(mPrfsColors->mGreyIdleMetaContacts, SIGNAL(toggled(bool)),
		this, SLOT(emitChanged()));
	connect(mPrfsColors->idleContactColor, SIGNAL(changed(const QColor &)),
		this, SLOT(emitChanged()));
	connect(mPrfsColors->mUseCustomFonts, SIGNAL(toggled(bool)),
		this, SLOT(emitChanged()));
	connect(mPrfsColors->mSmallFont, SIGNAL(fontSelected(const QFont &)),
		this, SLOT(emitChanged()));
	connect(mPrfsColors->mNormalFont, SIGNAL(fontSelected(const QFont &)),
		this, SLOT(emitChanged()));
	connect(mPrfsColors->mGroupNameColor, SIGNAL(changed(const QColor &)),
		this, SLOT(emitChanged()));

	connect(mPrfsColors->mBgOverride, SIGNAL(toggled(bool)),
		this, SLOT(emitChanged()));
	connect(mPrfsColors->mFgOverride, SIGNAL(toggled(bool)),
		this, SLOT(emitChanged()));
	connect(mPrfsColors->mRtfOverride, SIGNAL(toggled(bool)),
		this, SLOT(emitChanged()));

	mAppearanceTabCtl->addTab(mPrfsColors, i18n("Colors && Fonts"));

	// ==========================================================================

	errorAlert = false;
	styleChanged = false;
	slotTransparencyChanged(mPrfsChatWindow->mTransparencyEnabled->isChecked());

	load();
	//EmitChanged when something change.
}

AppearanceConfig::~AppearanceConfig()
{
	delete d;
}

void AppearanceConfig::save()
{
//	kdDebug(14000) << k_funcinfo << "called." << endl;
	KopetePrefs *p = KopetePrefs::prefs();

	// "Emoticons" TAB ==========================================================
	p->setIconTheme( mPrfsEmoticons->icon_theme_list->currentText() );
	p->setUseEmoticons ( mPrfsEmoticons->chkUseEmoticons->isChecked() );

	// "Chat Window" TAB ========================================================
	p->setTransparencyColor( mPrfsChatWindow->mTransparencyTintColor->color() );
	p->setTransparencyEnabled( mPrfsChatWindow->mTransparencyEnabled->isChecked() );
	p->setTransparencyValue( mPrfsChatWindow->mTransparencyValue->value() );
	if( styleChanged || p->styleSheet() != itemMap[ mPrfsChatWindow->styleList->selectedItem() ] )
		p->setStyleSheet( itemMap[ mPrfsChatWindow->styleList->selectedItem() ] );

	// "Contact List" TAB =======================================================
	p->setTreeView(mPrfsContactList->mTreeContactList->isChecked());
	p->setSortByGroup(mPrfsContactList->mSortByGroup->isChecked());
	p->setContactListIndentContacts(mPrfsContactList->mIndentContacts->isChecked());
	p->setContactListDisplayMode(KopetePrefs::ContactDisplayMode(mPrfsContactList->mDisplayMode->selectedId()));
	p->setContactListAnimation(mPrfsContactList->mAnimateChanges->isChecked());
	p->setContactListFading(mPrfsContactList->mFadeVisibility->isChecked());
	p->setContactListFolding(mPrfsContactList->mFoldVisibility->isChecked());

	// "Colors & Fonts" TAB =====================================================
	p->setHighlightBackground(mPrfsColors->backgroundColor->color());
	p->setHighlightForeground(mPrfsColors->foregroundColor->color());
	p->setBgColor(mPrfsColors->bgColor->color());
	p->setTextColor(mPrfsColors->textColor->color());
	p->setLinkColor(mPrfsColors->linkColor->color());
	p->setFontFace(mPrfsColors->fontFace->font());
	p->setIdleContactColor(mPrfsColors->idleContactColor->color());
	p->setGreyIdleMetaContacts(mPrfsColors->mGreyIdleMetaContacts->isChecked());
	p->setContactListUseCustomFonts(mPrfsColors->mUseCustomFonts->isChecked());
	p->setContactListCustomSmallFont(mPrfsColors->mSmallFont->font());
	p->setContactListCustomNormalFont(mPrfsColors->mNormalFont->font());
	p->setContactListGroupNameColor(mPrfsColors->mGroupNameColor->color());

	p->setBgOverride( mPrfsColors->mBgOverride->isChecked() );
	p->setFgOverride( mPrfsColors->mFgOverride->isChecked() );
	p->setRtfOverride( mPrfsColors->mRtfOverride->isChecked() );

	p->save();
	errorAlert = false;
	styleChanged = false;
}

void AppearanceConfig::load()
{
	if( errorAlert )
		return;
//	kdDebug(14000) << k_funcinfo << "called" << endl;
	KopetePrefs *p = KopetePrefs::prefs();

	// "Emoticons" TAB ==========================================================
	updateEmoticonlist();
	mPrfsEmoticons->chkUseEmoticons->setChecked( p->useEmoticons() );
	slotUseEmoticonsChanged ( p->useEmoticons() );

	// "Chat Window" TAB ========================================================
	mPrfsChatWindow->mTransparencyEnabled->setChecked( p->transparencyEnabled() );
	mPrfsChatWindow->mTransparencyTintColor->setColor( p->transparencyColor() );
	mPrfsChatWindow->mTransparencyValue->setValue( p->transparencyValue() );

	// FIXME: Using the filename as user-visible name is not translatable! - Martijn
	mPrfsChatWindow->styleList->clear();
	QStringList chatStyles = KGlobal::dirs()->findAllResources( "appdata", QString::fromLatin1( "styles/*.xsl" ) );
	for ( QStringList::Iterator it = chatStyles.begin(); it != chatStyles.end(); ++it )
	{
		QFileInfo fi( *it );
		QString fileName = fi.fileName().section( '.', 0, 0 );
		mPrfsChatWindow->styleList->insertItem( fileName, 0 );
		itemMap.insert( mPrfsChatWindow->styleList->firstItem(), fileName );

		if ( fileName == p->styleSheet() )
			mPrfsChatWindow->styleList->setSelected( mPrfsChatWindow->styleList->firstItem(), true );
	}
	mPrfsChatWindow->styleList->sort();

	// "Contact List" TAB =======================================================
	mPrfsContactList->mTreeContactList->setChecked( p->treeView() );
	mPrfsContactList->mSortByGroup->setChecked( p->sortByGroup() );
	mPrfsContactList->mIndentContacts->setChecked( p->contactListIndentContacts() );
	mPrfsContactList->mDisplayMode->setButton( p->contactListDisplayMode() );
	mPrfsContactList->mAnimateChanges->setChecked( p->contactListAnimation() );
	mPrfsContactList->mFadeVisibility->setChecked( p->contactListFading() && HAVE_XRENDER );
	mPrfsContactList->mFoldVisibility->setChecked( p->contactListFolding() );

	// "Colors & Fonts" TAB =====================================================
	mPrfsColors->foregroundColor->setColor(p->highlightForeground());
	mPrfsColors->backgroundColor->setColor(p->highlightBackground());
	mPrfsColors->textColor->setColor(p->textColor());
	mPrfsColors->linkColor->setColor(p->linkColor());
	mPrfsColors->bgColor->setColor(p->bgColor());
	mPrfsColors->fontFace->setFont(p->fontFace());
	mPrfsColors->mGreyIdleMetaContacts->setChecked(p->greyIdleMetaContacts());
	mPrfsColors->idleContactColor->setColor(p->idleContactColor());
	mPrfsColors->mUseCustomFonts->setChecked(p->contactListUseCustomFonts());
	mPrfsColors->mSmallFont->setFont(p->contactListCustomSmallFont());
	mPrfsColors->mNormalFont->setFont(p->contactListCustomNormalFont());
	mPrfsColors->mGroupNameColor->setColor(p->contactListGroupNameColor());

	mPrfsColors->mBgOverride->setChecked( p->bgOverride() );
	mPrfsColors->mFgOverride->setChecked( p->fgOverride() );
	mPrfsColors->mRtfOverride->setChecked( p->rtfOverride() );
}

void AppearanceConfig::updateEmoticonlist()
{
	KopetePrefs *p = KopetePrefs::prefs();
	KStandardDirs dir;

	mPrfsEmoticons->icon_theme_list->clear(); // Wipe out old list
	// Get a list of directories in our icon theme dir
	QStringList themeDirs = KGlobal::dirs()->findDirs("data", "kopete/pics/emoticons");
	// loop adding themes from all dirs into theme-list
	for(unsigned int x = 0;x < themeDirs.count();x++)
	{
		QDir themeQDir(themeDirs[x]);
		themeQDir.setFilter( QDir::Dirs ); // only scan for subdirs
		themeQDir.setSorting( QDir::Name ); // I guess name is as good as any
		for(unsigned int y = 0; y < themeQDir.count(); y++)
		{
			QStringList themes = themeQDir.entryList(QDir::Dirs, QDir::Name);
			// We don't care for '.' and '..'
			if ( themeQDir[y] != "." && themeQDir[y] != ".." )
			{
				// Add ourselves to the list, using our directory name
				QPixmap previewPixmap = QPixmap(locate("data", "kopete/pics/emoticons/"+themeQDir[y]+"/smile.png"));
				mPrfsEmoticons->icon_theme_list->insertItem(previewPixmap,themeQDir[y]);
			}
		}
	}

	// Where is that theme in our big-list-o-themes?
	QListBoxItem *item = mPrfsEmoticons->icon_theme_list->findItem( p->iconTheme() );

	if (item) // found it... make it the currently selected theme
		mPrfsEmoticons->icon_theme_list->setCurrentItem( item );
	else // Er, it's not there... select the current item
		mPrfsEmoticons->icon_theme_list->setCurrentItem( 0 );
}

void AppearanceConfig::slotUseEmoticonsChanged( bool b )
{
	mPrfsEmoticons->setEnabled( b );
	emitChanged();
}

void AppearanceConfig::slotSelectedEmoticonsThemeChanged()
{
	QString themeName = mPrfsEmoticons->icon_theme_list->currentText();
	QFileInfo fileInf(KGlobal::dirs()->findResource("data",
		"kopete/pics/emoticons/"+themeName+"/"));
	mPrfsEmoticons->btnRemoveTheme->setEnabled( fileInf.isWritable() );

	KopeteEmoticons emoticons( themeName );
	QStringList smileys = emoticons.picList();
	QString newContentText = "<qt>";

	for(QStringList::Iterator it = smileys.begin(); it != smileys.end(); ++it )
		newContentText += QString::fromLatin1("<img src=\"%1\"> ").arg(*it);

	newContentText += QString::fromLatin1("</qt>");
	mPrfsEmoticons->icon_theme_preview->setText(newContentText);
	emitChanged();
}

void AppearanceConfig::slotTransparencyChanged ( bool checked )
{
	mPrfsChatWindow->mTransparencyTintColor->setEnabled( checked );
	mPrfsChatWindow->mTransparencyValue->setEnabled( checked );
	emitChanged();
}

void AppearanceConfig::slotHighlightChanged()
{
//	bool value = mPrfsChatWindow->highlightEnabled->isChecked();
//	mPrfsChatWindow->foregroundColor->setEnabled ( value );
//	mPrfsChatWindow->backgroundColor->setEnabled ( value );
	slotUpdatePreview();
}

void AppearanceConfig::slotChangeFont()
{
	currentStyle = QString::null; //force to update preview;
	slotUpdatePreview();
	emitChanged();
}

void AppearanceConfig::slotAddStyle()
{
	editedItem = 0L;
	styleEditor = new StyleEditDialog(0L,"style", true);
	(new QHBoxLayout( styleEditor->editFrame ))->setAutoAdd( true );
	KTrader::OfferList offers = KTrader::self()->query( "KTextEditor/Document" );
	KService::Ptr service = *offers.begin();
	KLibFactory *factory = KLibLoader::self()->factory( service->library().latin1() );
	editDocument = static_cast<KTextEditor::Document *>( factory->create( styleEditor->editFrame, 0, "KTextEditor::Document" ) );

	if(!editDocument)
		return; //TODO: show an error if the plugin can't be loaded

	// FIXME: Can someone explain me why editDocument has no parent?
	// Is it a problem in Kate? in the KLibrary system? This is a workaround
	// for that. That also solves a crash while closing Kopete
	connect(styleEditor, SIGNAL(destroyed()), editDocument, SLOT(deleteLater()) );
	// Explanation of the crash (Bug 67494) :
	// There is a static deleter in the Kate KPart to free some classes. This
	// one also deletes some Highlight object. And there is another staticDeleter
	// in Kopete to free the LibLoader, which will free the Kate Library, which
	// will delete Kate's Document, which will deref some Highlight object
	// ***BOOM***  (Highlight objects are already deleted)
	// So this is maybe a problem in Kate. But if we delete object before closing
	// Kopete, when they are not useful anymore, no problem

	editDocument->createView( styleEditor->editFrame, 0 )->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding) );
	KTextEditor::editInterface( editDocument )->setText( QString::fromLatin1(
		"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		"<xsl:stylesheet version=\"1.0\" xmlns:xsl=\"http://www.w3.org/1999/XSL/Transform\">\n"
		"<xsl:output method=\"html\"/>\n"
		"<xsl:template match=\"message\">\n\n\n\n</xsl:template>\n</xsl:stylesheet>" ) );
	updateHighlight();
	styleEditor->show();
	connect( styleEditor->buttonOk, SIGNAL(clicked()), this, SLOT(slotStyleSaved()) );
	connect( styleEditor->buttonCancel, SIGNAL(clicked()), styleEditor, SLOT(deleteLater()) );
	currentStyle = QString::null; //force to update preview;
}

void AppearanceConfig::updateHighlight()
{
	KTextEditor::HighlightingInterface *hi = KTextEditor::highlightingInterface( editDocument );
	int count = hi->hlModeCount();
	for( int i=0; i < count; i++ )
	{
		if( hi->hlModeName(i) == QString::fromLatin1("XML") )
		{
			hi->setHlMode(i);
			break;
		}
	}
	emitChanged();
}

void AppearanceConfig::slotStyleSelected()
{
	QString styleName = itemMap[mPrfsChatWindow->styleList->selectedItem()];
	QString filePath = locateLocal("appdata", QString::fromLatin1("styles/%1.xsl").arg( styleName ) );
	QFileInfo fi( filePath );
	if(fi.isWritable())
	{
		mPrfsChatWindow->editButton->setEnabled( true );
		mPrfsChatWindow->deleteButton->setEnabled( true );
	}
	else
	{
		mPrfsChatWindow->editButton->setEnabled( false );
		mPrfsChatWindow->deleteButton->setEnabled( false );
	}
	slotUpdatePreview();
	emitChanged();
}

void AppearanceConfig::slotImportStyle()
{
	KURL chosenStyle = KURLRequesterDlg::getURL( QString::null, this, i18n( "Choose Stylesheet" ) );
	if ( !chosenStyle.isEmpty() )
	{
		QString stylePath;
		// FIXME: Using NetAccess uses nested event loops with all associated problems.
		//        Better use normal KIO and an async API - Martijn
		if ( KIO::NetAccess::download( chosenStyle, stylePath, this ) )
		{
			QString styleSheet = fileContents( stylePath );
			if ( KopeteXSLT( styleSheet ).isValid() )
			{
				QFileInfo fi( stylePath );
				addStyle( fi.fileName().section( '.', 0, 0 ), styleSheet );
			}
			else
			{
				KMessageBox::queuedMessageBox( this, KMessageBox::Error,
					i18n( "'%1' is not a valid XSLT document. Import canceled." ).arg( chosenStyle.path() ),
					i18n( "Invalid Style" ) );
			}
		}
		else
		{
			KMessageBox::queuedMessageBox( this, KMessageBox::Error,
				i18n( "Could not import '%1'. Check access permissions/network connection." ).arg( chosenStyle.path() ),
				i18n( "Import Error" ) );
		}
	}
}

void AppearanceConfig::slotCopyStyle()
{
	QListBoxItem *copiedItem = mPrfsChatWindow->styleList->selectedItem();
	if( copiedItem )
	{
		QString styleName =
			KInputDialog::getText( i18n( "New Style Name" ), i18n( "Enter the name of the new style:" ) );

		if ( !styleName.isEmpty() )
		{
			QString stylePath = locate("appdata", QString::fromLatin1("styles/%1.xsl").arg(itemMap[ copiedItem ]) );
			addStyle( styleName, fileContents( stylePath ) );
		}
	}
	else
	{
		KMessageBox::queuedMessageBox( this, KMessageBox::Sorry,
			i18n("Please select a style to copy."), i18n("No Style Selected") );
	}
	emitChanged();
}

void AppearanceConfig::slotEditStyle()
{
	slotAddStyle();
	editedItem = mPrfsChatWindow->styleList->selectedItem();
	QString stylePath = locate("appdata", QString::fromLatin1("styles/%1.xsl").arg(itemMap[ editedItem ]) );
	QString model = fileContents( stylePath );
	KTextEditor::editInterface( editDocument )->setText( model );
	updateHighlight();
	styleEditor->styleName->setText( editedItem->text() );
	emitChanged();
}

void AppearanceConfig::slotDeleteStyle()
{
	if( KMessageBox::warningContinueCancel( this, i18n("Are you sure you want to delete the style \"%1\"?")
		.arg( mPrfsChatWindow->styleList->selectedItem()->text() ),
		i18n("Delete Style"), i18n("Delete Style")) == KMessageBox::Continue )
	{
		QListBoxItem *style = mPrfsChatWindow->styleList->selectedItem();
		QString filePath = itemMap[ style ];
		itemMap.remove( style );

		QFileInfo fi( filePath );
		if( fi.isWritable() )
			QFile::remove( filePath );

		if( style->next() )
			mPrfsChatWindow->styleList->setSelected( style->next(), true );
		else
			mPrfsChatWindow->styleList->setSelected( style->prev(), true );
		delete style;
	}
	emitChanged();
}

void AppearanceConfig::slotStyleSaved()
{
	if( addStyle( styleEditor->styleName->text(), KTextEditor::editInterface( editDocument )->text() ) )
	{
		styleEditor->deleteLater();
		emitChanged();
	}
	else //The style has not been saved for a reason or another - don't loose it
		styleEditor->show();
}

bool AppearanceConfig::addStyle( const QString &styleName, const QString &styleSheet )
{
	bool newStyleName = !mPrfsChatWindow->styleList->findItem( styleName );
	bool editExistingStyle = (mPrfsChatWindow->styleList->selectedItem() &&
				 mPrfsChatWindow->styleList->selectedItem()->text()==styleName);

	if ( newStyleName || editExistingStyle )
	{
		QString filePath = locateLocal("appdata", QString::fromLatin1("styles/%1.xsl").arg( styleName ) );
		QFile out( filePath );
		if ( out.open( IO_WriteOnly ) )
		{
			QTextStream stream( &out );
			stream << styleSheet;
			out.close();

			if ( newStyleName )
			{
				mPrfsChatWindow->styleList->insertItem( styleName, 0 );
				itemMap.insert( mPrfsChatWindow->styleList->firstItem(), styleName );
				mPrfsChatWindow->styleList->setSelected( mPrfsChatWindow->styleList->firstItem(), true );
				mPrfsChatWindow->styleList->sort();
			}
			else
				slotUpdatePreview();

			styleChanged = true;
			return true;
		}
		else
		{
			KMessageBox::queuedMessageBox( this, KMessageBox::Error, i18n("Error saving file. Check access permissions to \"%1\".").arg( filePath ), i18n("Could Not Save") );
		}
	}
	else
	{
		KMessageBox::queuedMessageBox( this, KMessageBox::Error, i18n("A style named \"%1\" already exists. Please rename the style.").arg( styleName ), i18n("Could Not Save") );
	}

	//The style has not been saved for a reason or another
	return false;
}

//reimplement KopeteContact and his abstract method
class TestContact : public KopeteContact
{
public:
	TestContact (  const QString &id  ) : KopeteContact ( (KopeteAccount*)0L, id, 0L ) {}
	virtual KopeteMessageManager* manager(bool) { return 0L; }
};

void AppearanceConfig::slotUpdatePreview()
{
	QListBoxItem *style = mPrfsChatWindow->styleList->selectedItem();
	if( style && itemMap[ style ] != currentStyle )
	{
		KopeteContact *myself = new TestContact( i18n( "Myself" ) );
		KopeteContact *jack = new TestContact( i18n( "Jack" ) );

		KopeteMessage msgIn(  jack,   myself, i18n( "Hello, this is an incoming message :-)" ), KopeteMessage::Inbound );
		KopeteMessage msgOut( myself, jack,   i18n( "Ok, this is an outgoing message" ), KopeteMessage::Outbound );
		KopeteMessage msgInt( jack,   myself, i18n( "This is an internal message" ), KopeteMessage::Internal );
		KopeteMessage msgAct( jack,   myself, i18n( "performed an action" ), KopeteMessage::Action );
		//KopeteMessage msgHigh( jack, myself, i18n( "This is a highlighted message" ), KopeteMessage::Inbound );

		preview->begin();
		preview->write( QString::fromLatin1(
			"<html><head><style>"
			"body{ font-family:%1;color:%2; }"
			"td{ font-family:%3;color:%4; }"
			".highlight{ color:%5;background-color:%6 }"
			"</style></head>"
			"<body bgcolor=\"%7\" vlink=\"%8\" link=\"%9\">"
		).arg( mPrfsColors->fontFace->font().family() ).arg( mPrfsColors->textColor->color().name() )
			.arg( mPrfsColors->fontFace->font().family() ).arg( mPrfsColors->textColor->color().name() )
			.arg( mPrfsColors->foregroundColor->color().name() ).arg( mPrfsColors->backgroundColor->color().name() )
			.arg( mPrfsColors->bgColor->color().name() ).arg( mPrfsColors->linkColor->color().name() )
			.arg( mPrfsColors->linkColor->color().name() ) );

		// Parsing a XSLT message is incredibly slow! that's why I commented out some preview messages
		QString stylePath = locate("appdata", QString::fromLatin1("styles/%1.xsl").arg(itemMap[ style ]) );
		d->xsltParser->setXSLT( fileContents(stylePath) );
		preview->write( d->xsltParser->transform( msgIn.asXML().toString() ) );
		preview->write( d->xsltParser->transform( msgOut.asXML().toString() ) );
		msgIn.setFg( QColor( "DodgerBlue" ) );
		msgIn.setBg( QColor( "LightSteelBlue" ) );
		msgIn.setBody( i18n( "Here is an incoming colored message" ) );
		preview->write( d->xsltParser->transform( msgIn.asXML().toString() ) );
		preview->write( d->xsltParser->transform( msgInt.asXML().toString() ) );
		preview->write( d->xsltParser->transform( msgAct.asXML().toString() ) );
		//msgHigh.setImportance( KopeteMessage::Highlight );
		//preview->write( d->xsltParser->transform( msgHigh.asXML().toString() ) ) ;
		msgOut.setBody( i18n( "Bye" ) );
		preview->write( d->xsltParser->transform( msgOut.asXML().toString() ) );

		preview->write( QString::fromLatin1( "</body></html>" ) );
		preview->end();

		delete myself;
		delete jack;

		emitChanged();
	}
}

QString AppearanceConfig::fileContents( const QString &path )
{
 	QString contents;
	QFile file( path );
	if ( file.open( IO_ReadOnly ) )
	{
		QTextStream stream( &file );
		contents = stream.read();
		file.close();
	}

	return contents;
}

void AppearanceConfig::emitChanged()
{
	emit changed( true );
}

void AppearanceConfig::installNewTheme()
{
	KURL themeURL = KURLRequesterDlg::getURL(QString::null, this,
			i18n("Drag or Type Emoticon Theme URL"));
	if ( themeURL.isEmpty() )
		return;

	//TODO: support remote theme files!
	if ( !themeURL.isLocalFile() )
	{
		KMessageBox::queuedMessageBox( this, KMessageBox::Error, i18n("Sorry, emoticon themes must be installed from local files"),
		                               i18n("Could not install emoticon theme") );
		return;
	}

	Kopete::Global::installEmoticonTheme( themeURL.path() );
	updateEmoticonlist();
}

void AppearanceConfig::removeSelectedTheme()
{
	QListBoxItem *selected = mPrfsEmoticons->icon_theme_list->selectedItem();
	if(selected==0)
		return;

	QString themeName = selected->text();

	QString question=i18n("<qt>Are you sure you want to remove the "
			"<strong>%1</strong> emoticon theme?<br>"
			"<br>"
			"This will delete the files installed by this theme.</qt>").
		arg(themeName);

	int res = KMessageBox::questionYesNo(this, question, i18n("Confirmation"));
	if (res!=KMessageBox::Yes)
		return;

	KURL themeUrl(KGlobal::dirs()->findResource("data", "kopete/pics/emoticons/"+themeName+"/"));
	KIO::NetAccess::del(themeUrl, this);

	updateEmoticonlist();
}

void AppearanceConfig::slotEditTooltips()
{
	TooltipEditDialog *dlg = new TooltipEditDialog(this);
	connect(dlg, SIGNAL(changed(bool)), this, SIGNAL(changed(bool)));
	dlg->exec();
	delete dlg;
}

#include "appearanceconfig.moc"
// vim: set noet ts=4 sts=4 sw=4:
