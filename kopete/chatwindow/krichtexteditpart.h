// -*- c++ -*-

#ifndef KRICHTEXTEDITPART_H
#define KRICHTEXTEDITPART_H

#include <kparts/part.h>

class KTextEdit;
class KFontAction;
class KFontSizeAction;
class KToggleAction;
class KopeteTextEdit;

/**
 * KParts wrapper for QTextEdit.
 *
 * Originally by Richard Moore, rich@kde.org
 * forked by Jason Keirstead
 */
class KopeteRichTextEditPart : public KParts::ReadOnlyPart
{
	Q_OBJECT

	public:
		KopeteRichTextEditPart( QWidget *wparent, const char *wname, QObject*, const char*, const QStringList& );
		KopeteRichTextEditPart( QWidget *wparent, const char *wname, bool supportsRichText );

		/**
		* Returns the current editor widget.
		*/
		KTextEdit *widget() const { return (KTextEdit*)editor; }

		const QString text( Qt::TextFormat = Qt::AutoText ) const;

		const QFont &font() { return mFont; }

		const QColor &fgColor() { return mFgColor; }

		const QColor &bgColor() { return mBgColor; }

		void clear();

		bool simple() { return simpleMode; }

		static KAboutData *createAboutData();

		virtual bool openFile() { return false; };

	public slots:

		void setFgColor();
		void setFgColor( const QColor & );

		void setBgColor();
		void setBgColor( const QColor & );

		void setFont();
		void setFont( const QFont & );
		void setFont( const QString & );

		void setAlignLeft( bool yes );
		void setAlignRight( bool yes );
		void setAlignCenter( bool yes );
		void setAlignJustify( bool yes );

	protected:
		/**
		* Creates the part's actions in the specified action collection.
		*/
		virtual void createActions( KActionCollection *ac );

	protected slots:

		/**
		* Creates the part's actions in the part's action collection.
		*/
		void createActions();

		void updateFont();
		void updateCharFmt();
		void updateAligment();

	private:
		KopeteTextEdit *editor;
		KToggleAction *action_bold;
		KToggleAction *action_italic;
		KToggleAction *action_underline;

		KFontAction *action_font;
		KFontSizeAction *action_font_size;

		KToggleAction *action_align_left;
		KToggleAction *action_align_right;
		KToggleAction *action_align_center;
		KToggleAction *action_align_justify;

		bool simpleMode;

		QFont mFont;
		QColor mBgColor;
		QColor mFgColor;
};

#endif // KRICHTEXTEDITPART_H
