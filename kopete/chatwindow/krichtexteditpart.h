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
		KopeteRichTextEditPart( QWidget *wparent, const char *wname, int capabilities );

		/**
		* Returns the current editor widget.
		*/
		KTextEdit *widget() const { return (KTextEdit*)editor; }

		const QString text( Qt::TextFormat = Qt::AutoText ) const;

		const QFont &font() { return mFont; }

		const QColor &fgColor() { return mFgColor; }

		const QColor &bgColor() { return mBgColor; }

		void clear();

		int capabilities() { return m_capabilities; }

		bool richTextEnabled() { return m_richTextAvailable && m_richTextEnabled; }

		bool buttonsEnabled() { return !m_richTextAvailable || m_richTextEnabled; }

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

	signals:
		void toggleToolbar( const bool &enabled );

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
		void updateActions();

		void updateFont();
		void updateCharFmt();
		void updateAligment();

		void slotSetRichTextEnabled( bool enable );

	private:
		void readConfig();
		void writeConfig();

		KopeteTextEdit *editor;
		KAction *checkSpelling;
		KToggleAction *enableRichText;

		KAction *actionFgColor;
		KAction *actionBgColor;

		KToggleAction *action_bold;
		KToggleAction *action_italic;
		KToggleAction *action_underline;

		KFontAction *action_font;
		KFontSizeAction *action_font_size;

		KToggleAction *action_align_left;
		KToggleAction *action_align_right;
		KToggleAction *action_align_center;
		KToggleAction *action_align_justify;

		int m_capabilities;
		bool m_richTextAvailable;
		bool m_richTextEnabled;

		QFont mFont;
		QColor mBgColor;
		QColor mFgColor;
};

#endif // KRICHTEXTEDITPART_H
