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
#include <ktextedit.h>
#include <kurlrequesterdlg.h>
#include <krun.h>
#include <kdirwatch.h>

#include <ktexteditor/highlightinginterface.h>
#include <ktexteditor/editinterface.h>
#include <ktexteditor/document.h>
#include <ktexteditor/view.h>

// Things we fake to get the message preview to work
#include "kopetemetacontact.h"
#include "kopetecontact.h"
#include "kopetemessage.h"

#include "kopeteprefs.h"
#include "kopetexsl.h"
#include "kopeteemoticons.h"
#include "kopeteglobal.h"

#include <qtabwidget.h>

typedef KGenericFactory<AppearanceConfig, QWidget> KopeteAppearanceConfigFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_kopete_appearanceconfig, KopeteAppearanceConfigFactory( "kcm_kopete_appearanceconfig" ) )

class KopeteAppearanceConfigPrivate
{
public:
	Kopete::XSLT *xsltParser;
};

AppearanceConfig::AppearanceConfig(QWidget *parent, const char* /*name*/, const QStringList &args )
: KCModule( KopeteAppearanceConfigFactory::instance(), parent, args )
{
	editedItem = 0L;

	d = new KopeteAppearanceConfigPrivate;

	d->xsltParser = new Kopete::XSLT( KopetePrefs::prefs()->styleContents(), this );

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


	connect( KDirWatch::self() , SIGNAL(dirty(const QString&)) , this, SLOT( slotStyleModified( const QString &) ) );


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
	#ifdef HAVE_XRENDER
	mPrfsContactList->mFadeVisibility->setEnabled(true);
	#else
	mPrfsContactList->mFadeVisibility->setEnabled(false);
	#endif

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
	if( styleChanged || p->styleSheet() != mPrfsChatWindow->styleList->selectedItem()->text() )
		p->setStyleSheet(  mPrfsChatWindow->styleList->selectedItem()->text() );
	kdDebug(14000) << k_funcinfo << p->styleSheet()  << mPrfsChatWindow->styleList->selectedItem()->text() << endl;

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
	styleChanged = false;
}

void AppearanceConfig::load()
{
	//we will change the state of somme controls, which will call some signals.
	//so to don't refresh everything several times, we memorize we are loading.
	loading=true; 
	
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
		itemMap.insert( mPrfsChatWindow->styleList->firstItem(), *it );
		KDirWatch::self()->addFile(*it);

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
#ifdef HAVE_XRENDER
	mPrfsContactList->mFadeVisibility->setChecked( p->contactListFading() );
#else
	mPrfsContactList->mFadeVisibility->setChecked( false );
#endif
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
	
	loading=false;
	slotUpdatePreview();
}

void AppearanceConfig::updateEmoticonlist()
{
	KopetePrefs *p = KopetePrefs::prefs();
	KStandardDirs dir;

	mPrfsEmoticons->icon_theme_list->clear(); // Wipe out old list
	// Get a list of directories in our icon theme dir
	QStringList themeDirs = KGlobal::dirs()->findDirs("emoticons", "");
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
				// Add ourselves to the list, using our directory name  FIXME:  use the first emoticon of the theme.
				QPixmap previewPixmap = QPixmap(locate("emoticons", themeQDir[y]+"/smile.png"));
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

void AppearanceConfig::slotUseEmoticonsChanged( bool  )
{
	emitChanged();
}

void AppearanceConfig::slotSelectedEmoticonsThemeChanged()
{
	QString themeName = mPrfsEmoticons->icon_theme_list->currentText();
	QFileInfo fileInf(KGlobal::dirs()->findResource("emoticons", themeName+"/"));
	mPrfsEmoticons->btnRemoveTheme->setEnabled( fileInf.isWritable() );

	Kopete::Emoticons emoticons( themeName );
	QStringList smileys = emoticons.emoticonAndPicList().values();
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
	QString styleName=KInputDialog::getText( i18n("Add Styles - Kopete") , i18n("Enter the name for the new style you want to add") ,
				QString::null, 0L, this) ;
	if(styleName.isEmpty())
		return;

	if( addStyle( styleName ,
			"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
			"<xsl:stylesheet version=\"1.0\" xmlns:xsl=\"http://www.w3.org/1999/XSL/Transform\">\n"
			"<xsl:output method=\"html\"/>\n"
			"<xsl:template match=\"message\">\n\n\n\n</xsl:template>\n</xsl:stylesheet>"  ) );
	{
		KRun::runURL( KURL(locateLocal("appdata", QString::fromLatin1("styles/%1.xsl").arg( styleName ) )) , "text/plain");
		currentStyle = QString::null; //force to update preview;
	}
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
	QString filePath = itemMap[mPrfsChatWindow->styleList->selectedItem()];
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
			if ( Kopete::XSLT( styleSheet ).isValid() )
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
			QString stylePath = itemMap[ copiedItem ];
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
	editedItem = mPrfsChatWindow->styleList->selectedItem();
	QString stylePath = itemMap[ editedItem ];

	KRun::runURL(stylePath, "text/plain");
}

void AppearanceConfig::slotDeleteStyle()
{
	if( KMessageBox::warningContinueCancel( this, i18n("Are you sure you want to delete the style \"%1\"?")
		.arg( mPrfsChatWindow->styleList->selectedItem()->text() ),
		i18n("Delete Style"), KGuiItem(i18n("Delete Style"),"editdelete")) == KMessageBox::Continue )
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

void AppearanceConfig::slotStyleModified(const QString &filename)
{
	editedItem = mPrfsChatWindow->styleList->selectedItem();
	QString stylePath = itemMap[ editedItem ];

	if(filename == stylePath)
	{
		currentStyle=QString::null;  //force to relead the preview
		slotUpdatePreview();

		emitChanged();
	}
}

bool AppearanceConfig::addStyle( const QString &styleName, const QString &styleSheet )
{
	bool newStyleName = !mPrfsChatWindow->styleList->findItem( styleName );

	if ( newStyleName )
	{
		QString filePath = locateLocal("appdata", QString::fromLatin1("styles/%1.xsl").arg( styleName ) );
		QFile out( filePath );
		if ( out.open( IO_WriteOnly ) )
		{
			QTextStream stream( &out );
			stream << styleSheet;
			out.close();

			KDirWatch::self()->addFile(filePath);

			if ( newStyleName )
			{
				mPrfsChatWindow->styleList->insertItem( styleName, 0 );
				itemMap.insert( mPrfsChatWindow->styleList->firstItem(), filePath );
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

// Reimplement Kopete::Contact and its abstract method
class FakeContact : public Kopete::Contact
{
public:
	FakeContact ( const QString &id, Kopete::MetaContact *mc ) : Kopete::Contact( 0, id, mc ) {}
	virtual Kopete::ChatSession *manager(Kopete::Contact::CanCreateFlags /*c*/) { return 0L; }
	virtual void slotUserInfo() {};
};

static QString sampleConversationXML()
{
	//Kopete::MetaContact jackMC;
	FakeContact myself( i18n( "Myself" ), 0 );
	FakeContact jack( i18n( "Jack" ), /*&jackMC*/ 0 );
	
	Kopete::Message msgIn  ( &jack,   &myself, i18n( "Hello, this is an incoming message :-)" ), Kopete::Message::Inbound );
	Kopete::Message msgOut ( &myself, &jack,   i18n( "Ok, this is an outgoing message" ), Kopete::Message::Outbound );
	Kopete::Message msgCol ( &jack,   &myself, i18n( "Here is an incoming colored message" ), Kopete::Message::Inbound );
	msgCol.setFg( QColor( "DodgerBlue" ) );
	msgCol.setBg( QColor( "LightSteelBlue" ) );
	Kopete::Message msgInt ( &jack,   &myself, i18n( "This is an internal message" ), Kopete::Message::Internal );
	Kopete::Message msgAct ( &jack,   &myself, i18n( "performed an action" ), Kopete::Message::Inbound,
	                        Kopete::Message::PlainText, Kopete::Message::Chat, Kopete::Message::TypeAction );
	Kopete::Message msgHigh( &jack,   &myself, i18n( "This is a highlighted message" ), Kopete::Message::Inbound );
	msgHigh.setImportance( Kopete::Message::Highlight );
	Kopete::Message msgBye ( &myself, &jack,   i18n( "Bye" ), Kopete::Message::Outbound );
	
	return QString::fromLatin1( "<document>" ) + msgIn.asXML().toString() + msgOut.asXML().toString()
	       + msgCol.asXML().toString() + msgInt.asXML().toString() + msgAct.asXML().toString()
	       + msgHigh.asXML().toString() + msgBye.asXML().toString() + QString::fromLatin1( "</document>" );
}

void AppearanceConfig::slotUpdatePreview()
{
	if(loading)
		return;

	QListBoxItem *style = mPrfsChatWindow->styleList->selectedItem();
	if( style && style->text() != currentStyle )
	{
		//FIXME: should be using a ChatMessagePart
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
		
		QString stylePath = itemMap[ style ];
		d->xsltParser->setXSLT( fileContents(stylePath) );
		preview->write( d->xsltParser->transform( sampleConversationXML() ) );
		preview->write( QString::fromLatin1( "</body></html>" ) );
		preview->end();
		
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
		KMessageBox::queuedMessageBox( this, KMessageBox::Error, i18n("Sorry, emoticon themes must be installed from local files."),
		                               i18n("Could Not Install Emoticon Theme") );
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

	KURL themeUrl(KGlobal::dirs()->findResource("emoticons", themeName+"/"));
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
