#ifndef _KOPETEEMOTICONACTION_H_
#define _KOPETEEMOTICONACTION_H_

#include <kaction.h>

#include <qkeysequence.h>
#include <qobject.h>
#include <kguiitem.h>
#include <kicontheme.h>

/**
 * @author Stefan Gehn <sgehn@gmx.net>
 */

class KopeteEmoticonAction : public KAction
{
	Q_OBJECT
	Q_PROPERTY( bool delayed READ delayed WRITE setDelayed )
	Q_PROPERTY( bool stickyMenu READ stickyMenu WRITE setStickyMenu )

public:
	KopeteEmoticonAction( const QString& text, QObject* parent = 0,
					const char* name = 0 );
	KopeteEmoticonAction( const QString& text, const QIconSet& icon,
					QObject* parent = 0, const char* name = 0 );
	KopeteEmoticonAction( const QString& text, const QString& icon,
					QObject* parent = 0, const char* name = 0 );
	KopeteEmoticonAction( QObject* parent = 0, const char* name = 0 );
	virtual ~KopeteEmoticonAction();

	KPopupMenu* popupMenu() const;
	void popup( const QPoint& global );

	/**
	* Returns true if this action creates a delayed popup menu
	* when plugged in a KToolbar.
	*/
	bool delayed() const;
	/**
	* If set to true, this action will create a delayed popup menu
	* when plugged in a KToolbar. Otherwise it creates a normal popup.
	* Default: delayed
	*
	* Remember that if the "main" action (the toolbar button itself)
	* cannot be clicked, then you should call setDelayed(false).
	*
	* On the opposite, if the main action can be clicked, it can only happen
	* in a toolbar: in a menu, the parent of a submenu can't be activated.
	* To get a "normal" menu item when plugged a menu (and no submenu)
	* use KToolBarPopupAction.
	*/
	void setDelayed(bool _delayed);

	/**
	* Returns true if this action creates a sticky popup menu.
	* See @ref setStickyMenu.
	*/
	bool stickyMenu() const;
	/**
	* If set to true, this action will create a sticky popup menu
	* when plugged in a KToolbar.
	* "Sticky", means it's visible until a selection is made or the mouse is
	* clicked elsewhere. This feature allows you to make a selection without
	* having to press and hold down the mouse while making a selection.
	* Default: sticky.
	*/
	void setStickyMenu(bool sticky);

	virtual int plug( QWidget* widget, int index = -1 );

signals:
	void activated(const QString &);

private:
	class KopeteEmoticonActionPrivate;
	KopeteEmoticonActionPrivate *d;
private:
    void init();
};

#endif
