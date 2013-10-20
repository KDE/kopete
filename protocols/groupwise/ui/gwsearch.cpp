/*
    Kopete Groupwise Protocol
    gwsearch.cpp - logic for server side search widget

    Copyright (c) 2006      Novell, Inc	 	 	 http://www.opensuse.org
    Copyright (c) 2004      SUSE Linux AG	 	 http://www.suse.com
    
    Kopete (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>
 
    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU General Public                   *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "gwsearch.h"
#include <QtCore/QAbstractItemModel>
#include <QtGui/QComboBox>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPainter>
#include <QtGui/QSortFilterProxyModel>

#include <kdebug.h>
#include <klocale.h>

#include <kopetemetacontact.h>

#include "client.h"
#include "gwfield.h"
#include "gwaccount.h"
#include "gwcontact.h"
#include "gwcontactproperties.h"
#include "gwprotocol.h"
#include "tasks/searchusertask.h"

//#include "../modeltest.h"

class GroupWiseContactSearchModel : public QAbstractItemModel
{
public:
	enum ContactDetailsRole { CnRole = Qt::UserRole+1, DnRole, GivenNameRole,
		SurnameRole, FullNameRole, AwayMessageRole, AuthAttributeRole,
		StatusRole, StatusOrderedRole, ArchiveRole, PropertiesRole };
	GroupWiseContactSearchModel( QList<GroupWise::ContactDetails> contents, GroupWiseAccount * account, QObject * parent )
	: QAbstractItemModel( parent ), m_account( account ), m_contents( contents )
	{
	}
	~GroupWiseContactSearchModel()
	{
	}
	QModelIndex index( int row, int column, const QModelIndex& index ) const
	{
		if ( row >= 0 && column >= 0 &&
				row < rowCount() &&
				column < columnCount()
				&& !index.isValid() ) {
			return createIndex( row, column );
		}
		else {
			return QModelIndex();
		}
	}
	QModelIndex parent( const QModelIndex & /*index*/ ) const
	{
		return QModelIndex();
	}
	int columnCount( const QModelIndex & parent = QModelIndex() ) const
	{
		if ( parent.isValid() )
			return 0;
		else
			return 4;
	}
	int rowCount( const QModelIndex & parent = QModelIndex() ) const
	{
		if ( parent.isValid() )
			return 0;
		else
			return m_contents.count();
	}

	QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const
	{
		if ( orientation == Qt::Horizontal && role == Qt::DisplayRole )
		{
			switch ( section )
			{
				case 0:
					return i18n( "Status" );
					break;
				case 1:
					return QVariant( i18n( "First Name" ) );
					break;
				case 2:
					return QVariant( i18n( "Last Name" ) );
					break;
				case 3:
					return QVariant( i18n( "User ID" ) );
					break;
			}
		}
		return QAbstractItemModel::headerData( section, orientation, role );
	}

	Qt::ItemFlags flags( const QModelIndex & index ) const
	{
		if ( !index.isValid() )
		{
			return 0;
		}
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
	}
	QVariant data( const QModelIndex & index, int role = Qt::DisplayRole ) const
	{
		if ( !index.isValid() ) return QVariant();
		GroupWise::ContactDetails contactDetails = m_contents.at( index.row() );
		switch ( role )
		{
			case Qt::DecorationRole:
				if ( index.column() == 0 ) {
					return QVariant( GroupWiseProtocol::protocol()->gwStatusToKOS( contactDetails.status ).iconFor( m_account ) );
				}
				else {
					return QVariant();
				}
				break;
			case StatusOrderedRole:
				int statusOrdered;
				switch ( contactDetails.status )
				{
					case 0: //unknown
						statusOrdered = 0;
						break;
					case 1: //offline
						statusOrdered = 1;
						break;
					case 2: //online
						statusOrdered = 5;
						break;
					case 3: //busy
						statusOrdered = 2;
						break;
					case 4: // away
						statusOrdered = 3;
						break;
					case 5: //idle
						statusOrdered = 4;
						break;
					default:
						statusOrdered = 0;
						break;
				}
				return QVariant( statusOrdered );
				break;
			case Qt::DisplayRole:
				switch ( index.column() )
				{
					case 0:
						return QVariant( GroupWiseProtocol::protocol()->gwStatusToKOS( contactDetails.status ).description() );
						break;
					case 1:
						return QVariant( contactDetails.givenName );
					case 2:
						return QVariant( contactDetails.surname );
					case 3:
						return QVariant(GroupWiseProtocol::protocol()->dnToDotted( contactDetails.dn ) );
				}
				return QVariant();
				break;
			case CnRole:
				return QVariant( contactDetails.cn );
				break;
			case DnRole:
				return QVariant( contactDetails.dn );
				break;
			case GivenNameRole:
				return QVariant( contactDetails.givenName );
				break;
			case SurnameRole:
				return QVariant( contactDetails.surname );
				break;
			case FullNameRole:
				return QVariant( contactDetails.fullName );
				break;
			case AwayMessageRole:
				return QVariant( contactDetails.awayMessage );
				break;
			case AuthAttributeRole:
				return QVariant( contactDetails.authAttribute );
				break;
			case StatusRole:
				return QVariant( contactDetails.status );
				break;
			case ArchiveRole:
				return QVariant( contactDetails.archive );
				break;
			case PropertiesRole:
				return QVariant( contactDetails.properties );
				break;
			default:
				return QVariant();
		}
		return QVariant();
	}
	GroupWiseAccount * m_account;
	QList<GroupWise::ContactDetails> m_contents;
};

class GroupWiseContactSearchSortProxyModel : public QSortFilterProxyModel
{
public:
	GroupWiseContactSearchSortProxyModel( QObject * parent = 0 ) : QSortFilterProxyModel( parent )
	{}
	bool lessThan( const QModelIndex &left, const QModelIndex &right ) const
	{
		if ( left.column() == 0 && right.column() == 0 ) {
			return left.data( GroupWiseContactSearchModel::StatusOrderedRole ).toInt() < right.data( GroupWiseContactSearchModel::StatusOrderedRole ).toInt();
		} else {
			return QSortFilterProxyModel::lessThan( left, right );
		}
	}
};

class OnlineOnlyGroupWiseContactSearchSortProxyModel : public GroupWiseContactSearchSortProxyModel
{
public:
	OnlineOnlyGroupWiseContactSearchSortProxyModel( QObject * parent = 0 ) : GroupWiseContactSearchSortProxyModel( parent )
	{}
	bool filterAcceptsRow( int source_row, const QModelIndex & source_parent ) const
	{
		QModelIndex index = sourceModel()->index( source_row, 0, source_parent );
		int statusOrdered = sourceModel()->data( index, GroupWiseContactSearchModel::StatusOrderedRole ).toInt();
		return ( statusOrdered > 1 );
	}
};

GroupWiseContactSearch::GroupWiseContactSearch( GroupWiseAccount * account, QAbstractItemView::SelectionMode mode, bool onlineOnly,  QWidget *parent )
 : QWidget( parent ), m_account( account )
{
	setupUi( this );
	connect( m_details, SIGNAL(clicked()), SLOT(slotShowDetails()) );
	connect( m_search, SIGNAL(clicked()), SLOT(slotDoSearch()) );
	connect( m_clear, SIGNAL(clicked()), SLOT(slotClear()) );
	if ( onlineOnly ) {
		m_proxyModel = new OnlineOnlyGroupWiseContactSearchSortProxyModel( this );
	} else {
		m_proxyModel = new GroupWiseContactSearchSortProxyModel( this );
	}
	m_proxyModel->setDynamicSortFilter(true);

	m_results->header()->setClickable( true );
	m_results->header()->setSortIndicator( 0, Qt::DescendingOrder );
	m_results->header()->setSortIndicatorShown( true );
	m_results->setSelectionMode( mode );
	m_details->setEnabled( false );
}

GroupWiseContactSearch::~GroupWiseContactSearch()
{
}

void GroupWiseContactSearch::slotClear()
{
	m_firstName->clear();
	m_lastName->clear();
	m_userId->clear();
	m_title->clear();
	m_dept->clear();
}

void GroupWiseContactSearch::slotDoSearch()
{
	// build a query
	QList< GroupWise::UserSearchQueryTerm > searchTerms;
	if ( !m_firstName->text().isEmpty() )
	{
		GroupWise::UserSearchQueryTerm arg;
		arg.argument = m_firstName->text();
		arg.field = Field::KOPETE_NM_USER_DETAILS_GIVEN_NAME;
		arg.operation = searchOperation( m_firstNameOperation->currentIndex() );
		searchTerms.append( arg );
	}
	if ( !m_lastName->text().isEmpty() )
	{
		GroupWise::UserSearchQueryTerm arg;
		arg.argument = m_lastName->text();
		arg.field = Field::KOPETE_NM_USER_DETAILS_SURNAME;
		arg.operation = searchOperation( m_lastNameOperation->currentIndex() );
		searchTerms.append( arg );
	}
	if ( !m_userId->text().isEmpty() )
	{
		GroupWise::UserSearchQueryTerm arg;
		arg.argument = m_userId->text();
		arg.field = Field::NM_A_SZ_USERID;
		arg.operation = searchOperation( m_userIdOperation->currentIndex() );
		searchTerms.append( arg );
	}
	if ( !m_title->text().isEmpty() )
	{
		GroupWise::UserSearchQueryTerm arg;
		arg.argument = m_title->text();
		arg.field = Field::NM_A_SZ_TITLE;
		arg.operation = searchOperation( m_titleOperation->currentIndex() );
		searchTerms.append( arg );
	}
	if ( !m_dept->text().isEmpty() )
	{
		GroupWise::UserSearchQueryTerm arg;
		arg.argument = m_dept->text();
		arg.field = Field::NM_A_SZ_DEPARTMENT;
		arg.operation = searchOperation( m_deptOperation->currentIndex() );
		searchTerms.append( arg );
	}
	if ( !searchTerms.isEmpty() )
	{
		// start a search task
		SearchUserTask * st = new SearchUserTask( m_account->client()->rootTask() );
		st->search( searchTerms );
		connect( st, SIGNAL(finished()), SLOT(slotGotSearchResults()) );
		st->go( true );
		m_matchCount->setText( i18n( "Searching" ) );
		m_details->setEnabled( false );
		emit selectionValidates( false );
	}
	else
	{
		kDebug() << "no query to perform!";
	}
	
}

void GroupWiseContactSearch::slotShowDetails()
{
	kDebug() ;
	// get the first selected result
	QModelIndexList selected = m_results->selectionModel()->selectedIndexes();
	if ( !selected.empty() )
	{
		// if they are already in our contact list, show that version
		QModelIndex selectedIndex = selected.first();
		QString dn = m_proxyModel->data( selectedIndex, GroupWiseContactSearchModel::DnRole ).toString();
		
		GroupWiseContact * c = m_account->contactForDN( dn );
		GroupWiseContactProperties * p;
		if ( c )
			p = new GroupWiseContactProperties( c, this );
		else
		{
			p = new GroupWiseContactProperties( detailsAtIndex( selectedIndex ), this );
		}
		p->setObjectName( "gwcontactproperties" );
	}

}

GroupWise::ContactDetails GroupWiseContactSearch::detailsAtIndex( const QModelIndex & index ) const
{
	GroupWise::ContactDetails dt;
	dt.dn = m_proxyModel->data( index, GroupWiseContactSearchModel::DnRole ).toString();
	dt.givenName = m_proxyModel->data( index, GroupWiseContactSearchModel::GivenNameRole ).toString();
	dt.surname = m_proxyModel->data( index, GroupWiseContactSearchModel::SurnameRole ).toString();
	dt.fullName = m_proxyModel->data( index, GroupWiseContactSearchModel::FullNameRole ).toString();
	dt.awayMessage = m_proxyModel->data( index, GroupWiseContactSearchModel::AwayMessageRole ).toString();
	dt.authAttribute = m_proxyModel->data( index, GroupWiseContactSearchModel::AuthAttributeRole ).toString();
	dt.status = m_proxyModel->data( index, GroupWiseContactSearchModel::StatusRole ).toInt();
	dt.archive = m_proxyModel->data( index, GroupWiseContactSearchModel::ArchiveRole ).toBool();
	dt.properties = m_proxyModel->data( index, GroupWiseContactSearchModel::PropertiesRole ).toMap();
	return dt;
}
void GroupWiseContactSearch::slotGotSearchResults()
{
	kDebug() ;
	SearchUserTask * st = ( SearchUserTask * ) sender();
	m_lastSearchResults.clear();
	m_lastSearchResults = st->results();

	m_model = new GroupWiseContactSearchModel( m_lastSearchResults, m_account, this );
	//new ModelTest( m_model, this );
	m_proxyModel->setSourceModel( m_model );
	m_results->setModel( m_proxyModel );
	m_results->resizeColumnToContents( 0 );
	connect( m_results->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), SLOT(slotValidateSelection()) );

	m_matchCount->setText( i18np( "1 matching user found", "%1 matching users found", m_proxyModel->rowCount() ) );
	// if there was only one hit, select it
	if ( m_lastSearchResults.count() == 1 )
	{
		QItemSelectionModel * selectionModel = m_results->selectionModel();
		QItemSelection rowSelection;
		rowSelection.select( m_proxyModel->index( 0, 0, QModelIndex() ), m_proxyModel->index(0, 3, QModelIndex() ) );
		selectionModel->select( rowSelection, QItemSelectionModel::Select );
	}
	m_results->selectionModel()->selectedRows();
}

QList< GroupWise::ContactDetails > GroupWiseContactSearch::selectedResults()
{
	QList< GroupWise::ContactDetails> lst;
	if (m_results->selectionModel()) {
		foreach( QModelIndex index, m_results->selectionModel()->selectedRows() )
		{
			lst.append( detailsAtIndex( index ) );
		}
	} else {
		kDebug() << "called when no model was set!";
		kBacktrace();
	}
	return lst;
}

unsigned char GroupWiseContactSearch::searchOperation( int comboIndex )
{
	switch ( comboIndex )
	{
		case 0:
			return NMFIELD_METHOD_SEARCH;
		case 1:
			return NMFIELD_METHOD_MATCHBEGIN;
		case 2:
			return NMFIELD_METHOD_EQUAL;
	}
	return NMFIELD_METHOD_IGNORE;
}

void GroupWiseContactSearch::slotValidateSelection()
{
	int selectedCount = m_results->selectionModel()->selectedRows().count();
	m_details->setEnabled( selectedCount == 1 );
	emit selectionValidates( selectedCount != 0 );
}

#include "gwsearch.moc"
