// -*- c++ -*-

#ifndef KRICHTEXTEDITPART_H
#define KRICHTEXTEDITPART_H

#include <kparts/part.h>

#include <qfont.h>
#include <qcolor.h>

class KAboutData;
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

		QString text( Qt::TextFormat = Qt::AutoText ) const;

		QFont font() { return mFont; }

		QColor fgColor();

		QColor bgColor();

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

		void setFontSize( int );

		void setUnderline( bool );
		void setBold( bool );
		void setItalic( bool );

		void setAlignLeft( bool yes );
		void setAlignRight( bool yes );
		void setAlignCenter( bool yes );
		void setAlignJustify( bool yes );

		void checkToolbarEnabled();
		void reloadConfig();
		void slotSetRichTextEnabled( bool enable );

	signals:
		void toggleToolbar( bool enabled );

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

		bool m_configWriteLock;

		QFont mFont;
		QColor mBgColor;
		QColor mFgColor;
};

#endif // KRICHTEXTEDITPART_H
