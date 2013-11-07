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

#include <collapsiblewidget.h>
#include <QtGui>
#include <QTimeLine>

/******************************************************************
 * Helper classes
 *****************************************************************/

ClickableLabel::ClickableLabel( QWidget* parent )
    : QLabel( parent )
{
}

ClickableLabel::~ClickableLabel()
{
}

void ClickableLabel::mouseReleaseEvent( QMouseEvent *e )
{
    Q_UNUSED( e );
    emit clicked();
}

ArrowButton::ArrowButton( QWidget *parent )
: QAbstractButton( parent )
{
}


ArrowButton::~ArrowButton()
{
}

void ArrowButton::paintEvent( QPaintEvent *event )
{
  Q_UNUSED( event );
  QPainter p( this );
  QStyleOption opt;
  int h = sizeHint().height();
  opt.rect = QRect(0,( height()- h )/2, h, h);
  opt.palette = palette();
  opt.state = QStyle::State_Children;
  if (isChecked())
    opt.state |= QStyle::State_Open;

  style()->drawPrimitive(QStyle::PE_IndicatorBranch, &opt, &p);
  p.end();
}


/******************************************************************
 * Private classes
 *****************************************************************/

class CollapsibleWidget::Private
{
  public:
    QGridLayout    *gridLayout;
    QWidget        *innerWidget;
    ClickableLabel *label;
    ArrowButton    *colButton;
    QTimeLine      *timeline;
    QWidget        *expander;
    QVBoxLayout    *expanderLayout;
};

class SettingsContainer::Private
{
  public:
    QVBoxLayout *layout;
};

/******************************************************************
 * Implementation
 *****************************************************************/

SettingsContainer::SettingsContainer(QWidget *parent)
 : QScrollArea( parent ), d(new SettingsContainer::Private)
{
  QWidget *w = new QWidget;
  QVBoxLayout *helperLay = new QVBoxLayout(w);
  d->layout = new QVBoxLayout;
  helperLay->addLayout( d->layout );
  helperLay->addStretch(1);
  setWidget(w);
  setWidgetResizable(true);
}

SettingsContainer::~SettingsContainer()
{
  delete d;
}

CollapsibleWidget* SettingsContainer::insertWidget( QWidget *w, const QString& name )
{
   if (w && w->layout()) {
     QLayout *lay = w->layout();
     lay->setMargin(2);
     lay->setSpacing(0);
   }

   CollapsibleWidget *cw = new CollapsibleWidget( name );
   d->layout->addWidget( cw );
   cw->setInnerWidget( w );
   return cw;
}

CollapsibleWidget::CollapsibleWidget(QWidget *parent)
 : QWidget(parent), d(new CollapsibleWidget::Private)
{
  init();
}
CollapsibleWidget::CollapsibleWidget(const QString& caption, QWidget *parent)
 : QWidget(parent), d(new CollapsibleWidget::Private)
{
  init();
  setCaption(caption);
}

void CollapsibleWidget::init()
{
  d->expander = 0;
  d->expanderLayout = 0;
  d->timeline = new QTimeLine( 150, this );
  d->timeline->setCurveShape( QTimeLine::EaseInOutCurve );
  connect( d->timeline, SIGNAL(valueChanged(qreal)),
           this, SLOT(animateCollapse(qreal)) );

  d->innerWidget = 0;
  d->gridLayout = new QGridLayout( this );
  d->gridLayout->setMargin(0);

  d->colButton = new ArrowButton;
  d->colButton->setCheckable(true);

  d->label = new ClickableLabel;
  d->label->setSizePolicy(QSizePolicy::MinimumExpanding, 
                        QSizePolicy::Preferred);

  d->gridLayout->addWidget(d->colButton, 1, 1);
  d->gridLayout->addWidget(d->label, 1, 2);


  connect(d->label, SIGNAL(clicked()), 
          d->colButton, SLOT(click()));

  connect(d->colButton, SIGNAL(toggled(bool)), 
          SLOT(setExpanded(bool)));

  setExpanded(false);
  setEnabled(false);
}

CollapsibleWidget::~CollapsibleWidget()
{
  delete d;
}

QWidget* CollapsibleWidget::innerWidget() const
{
  return d->innerWidget;
}

//#define SIMPLE

void CollapsibleWidget::setInnerWidget(QWidget *w)
{
  if (!w) {
    return;
  }

  //bool first = d->innerWidget == 0;
  d->innerWidget = w;

#ifdef SIMPLE
  if ( !isExpanded() ) {
      d->innerWidget->hide();
  }
  d->gridLayout->addWidget( d->innerWidget, 2, 2 );
  d->gridLayout->setRowStretch( 2, 1 );
#else
  if ( !d->expander ) {
      d->expander = new QWidget( this );
      d->expander->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum));
      d->gridLayout->addWidget( d->expander, 2, 2 );
      d->gridLayout->setRowStretch( 2, 1 );
      d->expanderLayout = new QVBoxLayout( d->expander );
      d->expanderLayout->setMargin( 0 );
      d->expanderLayout->setSpacing( 0 );
      d->expander->setFixedHeight( 0 );
  }

  d->innerWidget->setParent( d->expander );
  d->innerWidget->show();
  d->expanderLayout->addWidget( d->innerWidget );
#endif

  setEnabled( true );

  if ( isExpanded() ) {
    setExpanded( true );
  }
}

void CollapsibleWidget::setCaption(const QString& caption)
{
  d->label->setText(QString("<b>%1</b>").arg(caption));
}

QString CollapsibleWidget::caption() const
{
  return d->label->text();
}


void CollapsibleWidget::setExpanded(bool expanded)
{
  if ( !d->innerWidget ) {
    return;
  }

#ifdef SIMPLE
  if ( !expanded ) {
    d->innerWidget->setVisible( false );
  }
#else
  if ( expanded ) {
      d->expander->setVisible( true );
  }
  d->innerWidget->setVisible( expanded );
#endif
  d->colButton->setChecked( expanded );
  d->timeline->setDirection( expanded ? QTimeLine::Forward
                                      : QTimeLine::Backward );
  d->timeline->start();
}

void CollapsibleWidget::animateCollapse( qreal showAmount )
{
  int pixels = d->innerWidget->sizeHint().height() * showAmount;
  d->gridLayout->setRowMinimumHeight( 2, pixels );

#ifdef SIMPLE
  d->gridLayout->setRowMinimumHeight( 2, pixels );

  if ( showAmount == 1 ) {
      d->innerWidget->setVisible( true );
  }
#else
  d->expander->setFixedHeight( pixels );
  if (parentWidget() && parentWidget()->layout())
	  parentWidget()->layout()->update();
#endif
}

bool CollapsibleWidget::isExpanded() const
{
  return d->colButton->isChecked();
}


#include "collapsiblewidget.moc"
