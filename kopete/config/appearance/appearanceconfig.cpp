/*
    appearanceconfig.cpp  -  Kopete Look Feel Config

    Copyright (c) 2001-2002 by Duncan Mac-Vicar Prett <duncan@kde.org>
    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "appearanceconfig.h"
#include "appearanceconfig_chatwindow.h"
#include "appearanceconfig_colors.h"

#include <qcheckbox.h>
#include <qdir.h>
#include <qlayout.h>
#include <qhbuttongroup.h>
#include <qspinbox.h>
#include <qslider.h>
#include <qlabel.h>

#include <kdeversion.h>
#if KDE_IS_VERSION( 3, 1, 90 )
#include <kinputdialog.h>
#else
#include <klineeditdlg.h>
#endif

#include <klineedit.h>
#include <kcolorcombo.h>
#include <kcolorbutton.h>
#include <kdebug.h>
#include <khtmlview.h>
#include <khtml_part.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kio/netaccess.h>
#include <kstandarddirs.h>
#include <kurlrequesterdlg.h>
#include <kpushbutton.h>
#include <kfontdialog.h>
#include <ktrader.h>
#include <klibloader.h>
#include <ktextedit.h>
#include <kgenericfactory.h>
#include <ktrader.h>


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

#include <qtabwidget.h>

typedef KGenericFactory<AppearanceConfig, QWidget> KopeteAppearanceConfigFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_kopete_appearanceconfig, KopeteAppearanceConfigFactory( "kcm_kopete_appearanceconfig" ) )


AppearanceConfig::AppearanceConfig(QWidget *parent, const char* /*name*/, const QStringList &args )
	: KCModule( KopeteAppearanceConfigFactory::instance(), parent, args )
{
	(new QVBoxLayout(this))->setAutoAdd(true);
	mAppearanceTabCtl = new QTabWidget(this, "mAppearanceTabCtl");

	editedItem = 0L;


	// "Emoticons" TAB ==========================================================
	mEmoticonsTab = new QFrame(mAppearanceTabCtl);
	(new QVBoxLayout(mEmoticonsTab, KDialog::marginHint(),
		KDialog::spacingHint()))->setAutoAdd(true);
	mUseEmoticonsChk = new QCheckBox(i18n("&Use emoticons"), mEmoticonsTab);
	icon_theme_list = new KListBox(mEmoticonsTab, "icon_theme_list");
	new QLabel(i18n("Preview:"), mEmoticonsTab);
/*	icon_theme_preview = new KIconView(mEmoticonsTab, "icon_theme_preview");
	icon_theme_preview->setFixedHeight(64);
	icon_theme_preview->setItemsMovable(false);
	icon_theme_preview->setSelectionMode(QIconView::NoSelection);
	icon_theme_preview->setFocusPolicy(NoFocus);
	icon_theme_preview->setSpacing(2);
*/
	icon_theme_preview = new KTextEdit(mEmoticonsTab, "icon_theme_preview");
	icon_theme_preview->setFixedHeight(64);
	icon_theme_preview->setFocusPolicy(NoFocus);
	icon_theme_preview->setReadOnly(true);
	icon_theme_preview->setWrapPolicy(QTextEdit::Anywhere);
	icon_theme_preview->setTextFormat(Qt::RichText);
/* // Doesn't work, don't ask me why [mETz]
	QStyleSheet *style = icon_theme_preview->styleSheet();
	QStyleSheetItem *img = style->item("img");
	img->setMargin(QStyleSheetItem::MarginAll, 8);
*/
	connect(mUseEmoticonsChk, SIGNAL(toggled(bool)),
		this, SLOT(slotUseEmoticonsChanged(bool)));
	connect(icon_theme_list, SIGNAL(selectionChanged()),
		this, SLOT(slotSelectedEmoticonsThemeChanged()));
	mAppearanceTabCtl->addTab(mEmoticonsTab, i18n("&Emoticons"));


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


	// "Colors" TAB =============================================================
	mPrfsColors = new AppearanceConfig_Colors(mAppearanceTabCtl);
	connect(mPrfsColors->foregroundColor, SIGNAL(changed(const QColor &)),
		this, SLOT(slotHighlightChanged()));
	connect(mPrfsColors->backgroundColor, SIGNAL(changed(const QColor &)),
		this, SLOT(slotHighlightChanged()));
	connect(mPrfsColors->fontFace, SIGNAL(clicked()),
		this, SLOT(slotChangeFont()));
	connect(mPrfsColors->textColor, SIGNAL(changed(const QColor &)),
		this, SLOT(slotUpdatePreview()));
	connect(mPrfsColors->bgColor, SIGNAL(changed(const QColor &)),
		this, SLOT(slotUpdatePreview()));
	connect(mPrfsColors->linkColor, SIGNAL(changed(const QColor &)),
		this, SLOT(slotUpdatePreview()));
	connect(mPrfsColors->mGreyIdleMetaContacts, SIGNAL(toggled(bool)),
		this, SLOT(slotGreyIdleMetaContactsChanged(bool)));

	mAppearanceTabCtl->addTab(mPrfsColors, i18n("Colors && Fonts"));
	// ==========================================================================

	errorAlert = false;
	styleChanged = false;
	slotTransparencyChanged(mPrfsChatWindow->mTransparencyEnabled->isChecked());

	load();
}

void AppearanceConfig::save()
{
//	kdDebug(14000) << k_funcinfo << "called." << endl;
	KopetePrefs *p = KopetePrefs::prefs();


	// "Emoticons" TAB ==========================================================
	p->setIconTheme( icon_theme_list->currentText() );
	p->setUseEmoticons ( mUseEmoticonsChk->isChecked() );


	// "Chat Window" TAB ========================================================
	p->setTransparencyColor( mPrfsChatWindow->mTransparencyTintColor->color() );
	p->setTransparencyEnabled( mPrfsChatWindow->mTransparencyEnabled->isChecked() );
	p->setTransparencyValue( mPrfsChatWindow->mTransparencyValue->value() );
	p->setBgOverride( mPrfsChatWindow->mTransparencyBgOverride->isChecked() );
	if( styleChanged || p->styleSheet() != itemMap[ mPrfsChatWindow->styleList->selectedItem() ] )
		p->setStyleSheet( itemMap[ mPrfsChatWindow->styleList->selectedItem() ] );


	// "Colors & Fonts" TAB =====================================================
	p->setHighlightBackground(mPrfsColors->backgroundColor->color());
	p->setHighlightForeground(mPrfsColors->foregroundColor->color());
	p->setBgColor(mPrfsColors->bgColor->color());
	p->setTextColor(mPrfsColors->textColor->color());
	p->setLinkColor(mPrfsColors->linkColor->color());
	p->setFontFace(mPrfsColors->fontFace->font());
	p->setIdleContactColor(mPrfsColors->idleContactColor->color());
	p->setGreyIdleMetaContacts(mPrfsColors->mGreyIdleMetaContacts->isChecked());

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
	KStandardDirs dir;
	icon_theme_list->clear(); // Wipe out old list
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
				QPixmap previewPixmap = QPixmap(locate("data","kopete/pics/emoticons/"+themeQDir[y]+"/smile.png"));
				icon_theme_list->insertItem(previewPixmap,themeQDir[y]);
			}
		}
	}

	// Where is that theme in our big-list-o-themes?
	QListBoxItem *item = icon_theme_list->findItem( p->iconTheme() );

	if (item) // found it... make it the currently selected theme
		icon_theme_list->setCurrentItem( item );
	else // Er, it's not there... select the current item
		icon_theme_list->setCurrentItem( 0 );

	mUseEmoticonsChk->setChecked( p->useEmoticons() );
	slotUseEmoticonsChanged     ( p->useEmoticons() );


	// "Chat Window" TAB ========================================================
	mPrfsChatWindow->mTransparencyEnabled->setChecked( p->transparencyEnabled() );
	mPrfsChatWindow->mTransparencyTintColor->setColor( p->transparencyColor() );
	mPrfsChatWindow->mTransparencyValue->setValue( p->transparencyValue() );
	mPrfsChatWindow->mTransparencyBgOverride->setChecked( p->bgOverride() );

	QStringList mChatStyles = KGlobal::dirs()->findAllResources(
		"appdata", QString::fromLatin1("styles/*.xsl") );
	mPrfsChatWindow->styleList->clear();
	for( QStringList::Iterator it = mChatStyles.begin(); it != mChatStyles.end(); ++it)
	{
		QFileInfo fi( *it );
		QString fileName = fi.fileName().section('.',0,0);
		mPrfsChatWindow->styleList->insertItem( fileName, 0 );
		itemMap.insert(mPrfsChatWindow->styleList->firstItem(), *it);
		if((*it) == p->styleSheet())
		{
			mPrfsChatWindow->styleList->setSelected(
				mPrfsChatWindow->styleList->firstItem(), true);
		}
	}
	mPrfsChatWindow->styleList->sort();


	// "Colors & Fonts" TAB =====================================================
	mPrfsColors->foregroundColor->setColor(p->highlightForeground());
	mPrfsColors->backgroundColor->setColor(p->highlightBackground());
	mPrfsColors->textColor->setColor(p->textColor());
	mPrfsColors->linkColor->setColor(p->linkColor());
	mPrfsColors->bgColor->setColor(p->bgColor());
	mPrfsColors->fontFace->setFont(p->fontFace());
	mPrfsColors->fontFace->setText(p->fontFace().family());
	mPrfsColors->mGreyIdleMetaContacts->setChecked(p->greyIdleMetaContacts());
	mPrfsColors->idleContactColor->setColor(p->idleContactColor());
	slotGreyIdleMetaContactsChanged(p->greyIdleMetaContacts());
}

void AppearanceConfig::slotUseEmoticonsChanged(bool b)
{
	icon_theme_list->setEnabled(b);
	icon_theme_preview->setEnabled(b);
}

void AppearanceConfig::slotSelectedEmoticonsThemeChanged()
{
//	kdDebug(14000) << k_funcinfo << "called." << endl;

/*	icon_theme_preview->clear();

	KopeteEmoticons emoticons( icon_theme_list->currentText() );
	QPixmap previewPixmap;

	QStringList smileys = emoticons.picList();

	for ( QStringList::Iterator it = smileys.begin(); it != smileys.end(); ++it )
	{
		previewPixmap = QPixmap((*it));
		if (!previewPixmap.isNull())
			new KIconViewItem(icon_theme_preview, 0, previewPixmap);
	}
*/

	KopeteEmoticons emoticons(icon_theme_list->currentText());
	QStringList smileys = emoticons.picList();
	QString newContentText = "<qt>";

	for(QStringList::Iterator it = smileys.begin(); it != smileys.end(); ++it )
	{
		newContentText += QString::fromLatin1("<img src=\"%1\"> ").arg(*it);
	}
	newContentText += QString::fromLatin1("</qt>");
	icon_theme_preview->setText(newContentText);
}

void AppearanceConfig::slotTransparencyChanged ( bool checked )
{
	mPrfsChatWindow->mTransparencyTintColor->setEnabled( checked );
	mPrfsChatWindow->mTransparencyValue->setEnabled( checked );
	mPrfsChatWindow->mTransparencyBgOverride->setEnabled( checked );
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
	QFont mFont = KopetePrefs::prefs()->fontFace();
	KFontDialog::getFont( mFont );
	KopetePrefs::prefs()->setFontFace( mFont );
	mPrfsColors->fontFace->setFont( mFont );
	mPrfsColors->fontFace->setText( mFont.family() );
	currentStyle = QString::null; //force to update preview;
	slotUpdatePreview();
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
	editDocument->createView( styleEditor->editFrame, 0 )->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding) );
	KTextEditor::editInterface( editDocument )->setText(
		QString::fromLatin1(
			"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
			"<xsl:stylesheet version=\"1.0\" xmlns:xsl=\"http://www.w3.org/1999/XSL/Transform\">\n"
			"<xsl:output method=\"html\"/>\n"
			"<xsl:template match=\"message\">\n\n\n\n</xsl:template>\n</xsl:stylesheet>") );
	updateHighlight();
	styleEditor->show();
	connect( styleEditor->buttonOk, SIGNAL(clicked()), this, SLOT(slotStyleSaved()) );
	connect( styleEditor->buttonCancel, SIGNAL(clicked()), styleEditor, SLOT(deleteLater()) );
	currentStyle=QString::null; //force to update preview;
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
}

void AppearanceConfig::slotStyleSelected()
{
	QFileInfo fi(itemMap[mPrfsChatWindow->styleList->selectedItem()]);
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
}

void AppearanceConfig::slotImportStyle()
{
	KURL chosenStyle = KURLRequesterDlg::getURL(QString::null, this,
		i18n("Choose Stylesheet"));

	if(!chosenStyle.isEmpty())
	{
		QString stylePath;
		if( KIO::NetAccess::download(chosenStyle, stylePath ) )
		{
			QString xslString = fileContents( stylePath );
			if( KopeteXSL::isValid( xslString ) )
			{
				QFileInfo fi( stylePath );
				addStyle( fi.fileName().section('.',0,0), xslString );
			}
			else
			{
				KMessageBox::error(
					this,
					i18n("\"%1\" is not a valid XSL document. Import canceled.")
						.arg(chosenStyle.path()),
					i18n("Invalid Style") );
			}
		}
		else
		{
			KMessageBox::error(
				this,
				i18n("Could not import \"%1\". Check access " \
					"permissions / network connection.")
					.arg(chosenStyle.path()),
				i18n("Import Error"));
		}
	}
}

void AppearanceConfig::slotCopyStyle()
{
	QListBoxItem *copiedItem = mPrfsChatWindow->styleList->selectedItem();
	if( copiedItem )
	{
#if KDE_IS_VERSION( 3, 1, 90 )
		QString styleName = KInputDialog::getText(
			i18n("New Style Name"),
			i18n("Enter the name of the new style:")
			);
#else
		QString styleName = KLineEditDlg::getText(
			i18n("New Style Name"),
			i18n("Enter the name of the new style:")
			);
#endif
		if ( !styleName.isEmpty() )
		{
			QString copiedXSL = fileContents( itemMap[ copiedItem] );
			addStyle( styleName, copiedXSL );
		}
	}
	else
	{
		KMessageBox::error(this,
			i18n("Please select a style to copy."), i18n("No Style Selected") );
	}
}

void AppearanceConfig::slotEditStyle()
{
	slotAddStyle();
	editedItem = mPrfsChatWindow->styleList->selectedItem();
	QString model = fileContents( itemMap[ editedItem] );
	KTextEditor::editInterface( editDocument )->setText( model );
	updateHighlight();
	styleEditor->styleName->setText( editedItem->text() );
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

		QFileInfo fi(filePath);
		if( fi.isWritable() )
			QFile::remove( filePath );

		if( style->next() )
			mPrfsChatWindow->styleList->setSelected( style->next(), true );
		else
			mPrfsChatWindow->styleList->setSelected( style->prev(), true );
		delete style;
	}
}

void AppearanceConfig::slotStyleSaved()
{
	QString fileSource = KTextEditor::editInterface( editDocument )->text();
	QString filePath = itemMap[ editedItem ];
	delete editedItem;

	if( !KopeteXSL::isValid( fileSource ) )
		KMessageBox::error( this, i18n("This is not a valid XSL document. Please double check your modifications."), i18n("Invalid Style") );

	if( !filePath.isNull() )
	{
		QFileInfo fi(filePath);
		if( fi.isWritable() )
			QFile::remove( filePath );
	}

	addStyle( styleEditor->styleName->text(), fileSource );

	styleEditor->deleteLater();
}

void AppearanceConfig::addStyle( const QString &styleName, const QString &xslString )
{
	if( !mPrfsChatWindow->styleList->findItem( styleName ) )
	{
		QString filePath = locateLocal("appdata", QString::fromLatin1("styles/%1.xsl").arg( styleName ) );
		QFile out( filePath );
		if ( out.open( IO_WriteOnly ) )
		{
			QTextStream stream( &out );
			stream << xslString;
			out.close();

			mPrfsChatWindow->styleList->insertItem( styleName, 0 );
			itemMap.insert( mPrfsChatWindow->styleList->firstItem(), filePath );
			mPrfsChatWindow->styleList->setSelected( mPrfsChatWindow->styleList->firstItem(), true );
			mPrfsChatWindow->styleList->sort();
			styleChanged = true;
		}
		else
		{
			KMessageBox::error( this, i18n("Error saving file. Check access permissions to \"%1\".").arg( filePath ), i18n("Could Not Save") );
		}
	}
	else
	{
		KMessageBox::error( this, i18n("A style named \"%1\" already exists. Please rename the style.").arg( styleName ), i18n("Could Not Save") );
	}
}

	//reimplement KopeteContact and his abstract method
	class TestContact : public KopeteContact
	{
		public:
			TestContact (  const QString &id  ) : KopeteContact ( 0L, id, 0L ) {}
			virtual KopeteMessageManager* manager(bool) { return 0L; }
	};

void AppearanceConfig::slotUpdatePreview()
{
	QString model;
	QListBoxItem *style = mPrfsChatWindow->styleList->selectedItem();
	if( style && itemMap[style] != currentStyle )
	{
		currentStyle=itemMap[style];
		QString model = fileContents( currentStyle );

		if(!model.isEmpty())
		{
			KopeteContact *myself = new TestContact(i18n("Myself"));
			KopeteContact *jack = new TestContact(i18n("Jack") );

			KopeteMessage msgIn( jack, myself, i18n("Hello, This is an incoming message :-) "),KopeteMessage::Inbound );
			KopeteMessage msgOut( myself, jack, i18n("Ok, this is an outgoing message"),KopeteMessage::Outbound );
			KopeteMessage msgInt( jack, myself, i18n("This is an internal message"),KopeteMessage::Internal );
			//KopeteMessage msgHigh( jack, myself, i18n("This is a highlighted message"),KopeteMessage::Inbound );
			KopeteMessage msgAct( jack, myself, i18n("performed an action"),KopeteMessage::Action );

			preview->begin();
			preview->write( QString::fromLatin1( "<html><head><style>body{font-family:%1;color:%2;}td{font-family:%3;color:%4;}.highlight{color:%5;background-color:%6}</style></head><body bgcolor=\"%7\" vlink=\"%8\" link=\"%9\">" )
				.arg( mPrfsColors->fontFace->font().family() )
				.arg( mPrfsColors->textColor->color().name() )
				.arg( mPrfsColors->fontFace->font().family() )
				.arg( mPrfsColors->textColor->color().name() )
				.arg( mPrfsColors->foregroundColor->color().name() )
				.arg( mPrfsColors->backgroundColor->color().name() )
				.arg( mPrfsColors->bgColor->color().name() )
				.arg( mPrfsColors->linkColor->color().name() )
				.arg( mPrfsColors->linkColor->color().name() ) );

			//parsing a XSLT message is incredibly slow! that's why i commented out some preview messages
			preview->write( KopeteXSL::xsltTransform( msgIn.asXML().toString(), model )) ;
			preview->write( KopeteXSL::xsltTransform( msgOut.asXML().toString(), model)  );
			msgIn.setFg(QColor("DodgerBlue"));
			msgIn.setBg(QColor("LightSteelBlue"));
			msgIn.setBody( i18n("Here is an incoming colored message"));
			preview->write( KopeteXSL::xsltTransform( msgIn.asXML().toString(), model ) );
			preview->write( KopeteXSL::xsltTransform( msgInt.asXML().toString(), model ) );
			preview->write( KopeteXSL::xsltTransform( msgAct.asXML().toString(), model ) );
			//msgHigh.setImportance( KopeteMessage::Highlight );
			//preview->write( KopeteXSL::xsltTransform( msgHigh.asXML().toString(), model )) ;
			msgOut.setBody( i18n("Bye"));
			preview->write( KopeteXSL::xsltTransform( msgOut.asXML().toString(), model)  );

			preview->write( QString::fromLatin1( "</body></html>" ) );
			preview->end();

			delete myself;
			delete jack;
		} // END if(!model.isEmpty())
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

void AppearanceConfig::slotGreyIdleMetaContactsChanged(bool b)
{
	mPrfsColors->idleContactColor->setEnabled(b);
}


#include "appearanceconfig.moc"
// vim: set noet ts=4 sts=4 sw=4:
