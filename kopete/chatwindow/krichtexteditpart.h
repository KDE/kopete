// -*- c++ -*-

#ifndef KRICHTEXTEDITPART_H
#define KRICHTEXTEDITPART_H

#include <kparts/part.h>

class KTextEdit;
class KFontAction;
class KFontSizeAction;
class KToggleAction;

/**
 * KParts wrapper for QTextEdit.
 *
 * @version $Id$
 * @author Richard Moore, rich@kde.org
 */
class KRichTextEditPart : public KParts::ReadWritePart
{
    Q_OBJECT

public:
    KRichTextEditPart( QWidget *wparent, const char *wname,
		       QObject *parent, const char *name,
		       const QStringList &/*args*/ );
    virtual ~KRichTextEditPart();

    /**
     * Returns the current filename.
     */
    QString filename() const { return m_file; }

    /**
     * Returns the current editor widget.
     */
    KTextEdit *widget() const { return editor; }

    /**
     * Returns the data for the About dialog.
     */
    static KAboutData *createAboutData();

public slots:

    /**
     * Displays a file dialog and loads the selected file.
     */
    bool open();

    /**
     * Displays a file dialog and saves to the selected file.
     */
    bool saveAs();

    /**
     * Prints the current document
     */
    bool print();

    /**
     * Enables and disables edits.
     */
    virtual void setReadWrite( bool rw );

    /**
     * Displays a color dialog and sets the text color to the selected value.
     */
    void formatColor();

    void checkSpelling();

    /**
     * @internal
     */
    void setAlignLeft( bool yes );

    /**
     * @internal
     */
    void setAlignRight( bool yes );

    /**
     * @internal
     */
    void setAlignCenter( bool yes );

    /**
     * @internal
     */
    void setAlignJustify( bool yes );

protected:
    /**
     * Creates the part's actions in the specified action collection.
     */
    virtual void createActions( KActionCollection *ac );

protected slots:
    virtual bool openFile();
    virtual bool saveFile();

    /**
     * Creates the part's actions in the part's action collection.
     */
    void createActions();

    void updateActions();

    void updateFont();
    void updateCharFmt();
    void updateAligment();

private:
    KTextEdit *editor;
    KToggleAction *action_bold;
    KToggleAction *action_italic;
    KToggleAction *action_underline;

    KFontAction *action_font;
    KFontSizeAction *action_font_size;

    KToggleAction *action_align_left;
    KToggleAction *action_align_right;
    KToggleAction *action_align_center;
    KToggleAction *action_align_justify;

    struct Data *d;
};

#endif // KRICHTEXTEDITPART_H


// Local Variables:
// c-basic-offset: 4
// End:
