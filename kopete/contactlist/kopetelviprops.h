#ifndef KOPETELVIPROPS_H
#define KOPETELVIPROPS_H

#include <kdialogbase.h>

#include "kopetegvipropswidget.h"
#include "kopetemetalvipropswidget.h"

class KopeteGroupViewItem;
class KopeteMetaContactLVI;

class KopeteGVIProps: public KDialogBase
{
	Q_OBJECT

	public:
		KopeteGVIProps(KopeteGroupViewItem *gvi, QWidget *parent, const char *name=0L);
		~KopeteGVIProps();

	private:
		KopeteGVIPropsWidget *mainWidget;
		KopeteGroupViewItem *item;

	private slots:
		void slotOkClicked();
		void slotUseCustomIconsToggled(bool on);
};


class KopeteMetaLVIProps: public KDialogBase
{
	Q_OBJECT

	public:
		KopeteMetaLVIProps(KopeteMetaContactLVI *gvi, QWidget *parent, const char *name=0L);
		~KopeteMetaLVIProps();

	private:
		KopeteMetaLVIPropsWidget *mainWidget;
		KopeteMetaContactLVI *item;

	private slots:
		void slotOkClicked();
		void slotUseCustomIconsToggled(bool on);
};

#endif
