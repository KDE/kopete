/*
    Kopete Contactlist Model

    Copyright (c) 2007      by Aleix Pol              <aleixpol@gmail.com>
    Copyright     2009      by Roman Jarosz           <kedgedev@gmail.com>

    Kopete    (c) 2002-2009 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef KOPETE_UI_CONTACTLISTMODEL_H
#define KOPETE_UI_CONTACTLISTMODEL_H

#include <QAbstractItemModel>
#include <QPair>
#include <QHash>

#include <kopete_export.h>

class QDomDocument;
class QDomElement;

namespace Kopete {

class Group;
class MetaContact;
class MessageEvent;

namespace UI {

/**
@author Aleix Pol <aleixpol@gmail.com>
*/
class KOPETE_CONTACT_LIST_EXPORT ContactListModel : public QAbstractItemModel
{
Q_OBJECT
public:
	ContactListModel(QObject* parent = 0);

	void init();

	virtual int columnCount ( const QModelIndex & parent = QModelIndex() ) const;

	/* drag-n-drop stuff */
	virtual Qt::DropActions supportedDropActions() const;
	virtual QMimeData* mimeData(const QModelIndexList &indexes) const;
	virtual QStringList mimeTypes() const;
	virtual bool setData(const QModelIndex &index, const QVariant &value, const int role);

	bool loadModelSettings( const QString& modelType );
	bool saveModelSettings( const QString& modelType );

public Q_SLOTS:
	virtual void addMetaContact( Kopete::MetaContact* );
	virtual void removeMetaContact( Kopete::MetaContact* );

	virtual void addGroup( Kopete::Group* );
	virtual void removeGroup( Kopete::Group* );

	virtual void addMetaContactToGroup( Kopete::MetaContact*, Kopete::Group* );
	virtual void removeMetaContactFromGroup( Kopete::MetaContact*, Kopete::Group* );
	virtual void moveMetaContactToGroup( Kopete::MetaContact*, Kopete::Group*, Kopete::Group*);

protected Q_SLOTS:
	virtual void appearanceConfigChanged() = 0;
	virtual void handleContactDataChange( Kopete::MetaContact* ) = 0;
	virtual void loadContactList();
	void handleContactDataChange();
	void newMessageEvent( Kopete::MessageEvent *event );
	void newMessageEventDone( Kopete::MessageEvent *event );

protected:
	bool dropUrl( const QMimeData *data, int row, const QModelIndex &parent, Qt::DropAction action );
	typedef QPair<Kopete::Group*, Kopete::MetaContact*> GroupMetaContactPair;
	virtual bool dropMetaContacts( int row, const QModelIndex &parent, Qt::DropAction action, const QList<GroupMetaContactPair> &items );

	QList<QVariant> emoticonStringToList( const QString &msg ) const;

	QVariant metaContactData( const Kopete::MetaContact* mc, int role ) const;
	QVariant metaContactImage( const Kopete::MetaContact* mc ) const;
	QString metaContactTooltip( const Kopete::MetaContact* metaContact ) const;

	virtual void loadModelSettingsImpl( QDomElement& rootElement ) = 0;
	virtual void saveModelSettingsImpl( QDomDocument& doc, QDomElement& rootElement ) = 0;

	bool m_manualGroupSorting;
	bool m_manualMetaContactSorting;

private:
	QHash< const Kopete::MetaContact*, QSet<Kopete::MessageEvent*> > m_newMessageMetaContactSet;
};
}

}

#endif
//kate: tab-width 4
