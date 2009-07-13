/*
   This file is part of the KDE libraries
   Copyright (C) 2005 Daniel Molkentin <molkentin@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

/*
 * file copied from playground/libs/ui/collabsiblewidget
 */
#ifndef COLLAPSIBLEWIDGET_H
#define COLLAPSIBLEWIDGET_H

#include <QScrollArea>
#include <QLabel>
#include <QDebug>
#include <QAbstractButton>
#include <kopete_export.h>

class QScrollArea;

class ClickableLabel : public QLabel
{
  Q_OBJECT
  public:
    ClickableLabel( QWidget* parent = 0 );
    ~ClickableLabel();

    void mouseReleaseEvent( QMouseEvent *e );

  signals:
    void clicked();
};

class ArrowButton : public QAbstractButton
{
  public:
    ArrowButton(QWidget *parent = 0);
    ~ArrowButton();

    QSize sizeHint() const { return QSize(16, 16); }

  protected:
    void paintEvent( QPaintEvent* );

  private:
};

/**
  @short A widget that has a caption and a collapsible widget
  @author Daniel Molkentin <molkentin@kde.org>
 */
class KOPETE_EXPORT CollapsibleWidget : public QWidget
{
    Q_OBJECT
  public:
    CollapsibleWidget(QWidget *parent = 0);
    explicit CollapsibleWidget(const QString& caption, QWidget *parent = 0);
    ~CollapsibleWidget();
    
    QString caption() const;
    bool isExpanded() const;

    QWidget* innerWidget() const;
    void setInnerWidget( QWidget *w);

  public slots:
    void setExpanded(bool collapsed);
    void setCaption(const QString& caption);


  protected:
    void init();

  private slots:
    void animateCollapse(qreal);

  private:
    Q_DISABLE_COPY( CollapsibleWidget )
    class Private;
    Private * const d;


};


/**
  @short A scrollable container that contains groups of settings,
         usually in the form of CollapsibleWidgets.
  @author Daniel Molkentin <molkentin@kde.org>
 */
class KOPETE_EXPORT SettingsContainer : public QScrollArea
{
    Q_ENUMS( CollapseState )
    Q_OBJECT
  public:
    enum CollapseState { Collapsed, Uncollapsed };
    SettingsContainer( QWidget *parent = 0 );
    ~SettingsContainer();

    CollapsibleWidget* insertWidget( QWidget* w, const QString& name );

 private:
    Q_DISABLE_COPY( SettingsContainer )
    class Private;
    Private * const d;
};



#endif // COLLAPSIBLEWIDGET_H

