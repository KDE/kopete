 /*

    Copyright (c) 2011      by Tobias Koenig  <tokoe at kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
 */

#include "dlgjabberbookmarkeditor.h"

#include <kinputdialog.h>

#include <QtCore/QAbstractListModel>

class JabberBookmarkModel : public QAbstractListModel
{
  public:
    enum Role
    {
      NameRole = Qt::UserRole,
      AutoJoinRole
    };

    JabberBookmarkModel( QObject *parent = 0 )
      : QAbstractListModel( parent )
    {
    }

    void setBookmarks( const JabberBookmark::List &bookmarks )
    {
      beginResetModel();
      m_bookmarks = bookmarks;
      endResetModel();
    }

    JabberBookmark::List bookmarks() const
    {
      return m_bookmarks;
    }

    int rowCount( const QModelIndex &parent ) const
    {
      if ( parent.isValid() )
        return 0;

      return m_bookmarks.count();
    }

    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const
    {
      if ( index.row() >= m_bookmarks.count() )
        return QVariant();

      const JabberBookmark bookmark = m_bookmarks.at( index.row() );
      switch ( role ) {
        case Qt::DisplayRole:
          return QString( "%1 (%2)" ).arg( bookmark.fullJId() ).arg( bookmark.name() );
        case Qt::DecorationRole:
          return bookmark.autoJoin() ? KIcon( "irc-join-channel" ) : QVariant();
        case NameRole:
          return bookmark.name();
        case AutoJoinRole:
          return bookmark.autoJoin();
      }

      return QVariant();
    }

    bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole )
    {
      if ( index.row() >= m_bookmarks.count() )
        return false;

      JabberBookmark &bookmark = m_bookmarks[ index.row() ];
      if ( role == NameRole ) {
        bookmark.setName( value.toString() );
        emit dataChanged( index, index );
        return true;
      } else if ( role == AutoJoinRole ) {
        bookmark.setAutoJoin( value.toBool() );
        emit dataChanged( index, index );
        return true;
      }

      return false;
    }

    bool removeRows( int row, int count, const QModelIndex &parent = QModelIndex() )
    {
      beginRemoveRows( parent, row, row + count - 1 );
      for ( int i = 0; i < count; ++i )
        m_bookmarks.removeAt( row );
      endRemoveRows();

      return true;
    }

    JabberBookmark::List m_bookmarks;
};

DlgJabberBookmarkEditor::DlgJabberBookmarkEditor( const JabberBookmark::List &bookmarks, QWidget *parent )
  : KDialog( parent )
{
  m_ui.setupUi( mainWidget() );

  m_model = new JabberBookmarkModel( this );

  m_model->setBookmarks( bookmarks );
  m_ui.listView->setModel( m_model );

  connect( m_ui.renameButton, SIGNAL( clicked() ), SLOT( renameBookmark() ) );
  connect( m_ui.autoJoinButton, SIGNAL( clicked() ), SLOT( toggleAutoJoin() ) );
  connect( m_ui.removeButton, SIGNAL( clicked() ), SLOT( removeBookmark() ) );
}

DlgJabberBookmarkEditor::~DlgJabberBookmarkEditor()
{
}

JabberBookmark::List DlgJabberBookmarkEditor::bookmarks() const
{
  return m_model->bookmarks();
}

void DlgJabberBookmarkEditor::renameBookmark()
{
  if ( !m_ui.listView->selectionModel()->hasSelection() )
    return;

  const QModelIndex index = m_ui.listView->selectionModel()->selectedRows().first();

  const QString name = KInputDialog::getText( i18n( "Group Chat Name" ),
                                              i18n( "Enter a name for the group chat:" ),
                                              index.data( JabberBookmarkModel::NameRole ).toString() );

  if ( !name.isEmpty() ) {
    m_model->setData( index, name, JabberBookmarkModel::NameRole );
  }
}

void DlgJabberBookmarkEditor::toggleAutoJoin()
{
  if ( !m_ui.listView->selectionModel()->hasSelection() )
    return;

  const QModelIndex index = m_ui.listView->selectionModel()->selectedRows().first();

  m_model->setData( index, QVariant( !index.data( JabberBookmarkModel::AutoJoinRole ).toBool() ), JabberBookmarkModel::AutoJoinRole );
}

void DlgJabberBookmarkEditor::removeBookmark()
{
  if ( !m_ui.listView->selectionModel()->hasSelection() )
    return;

  const QModelIndex index = m_ui.listView->selectionModel()->selectedRows().first();

  m_model->removeRow( index.row() );
}

#include "dlgjabberbookmarkeditor.moc"
