/*
 * This file is part of telepathy-accounts-kcm
 *
 * Copyright (C) 2009 Collabora Ltd. <info@collabora.co.uk>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "parameter-edit-delegate.h"

#include "integer-edit.h"
#include "parameter-edit-model.h"
#include "unsigned-integer-edit.h"

#include <KDebug>

#include <QtGui/QApplication>
#include <QtGui/QCheckBox>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPainter>

ParameterEditDelegate::ParameterEditDelegate(QAbstractItemView *itemView, QObject *parent)
 : KWidgetItemDelegate(itemView, parent)
{
    kDebug();

    // TODO: Implement me!
}

ParameterEditDelegate::~ParameterEditDelegate()
{
    kDebug();

    // TODO: Implement me!
}


QList<QWidget*> ParameterEditDelegate::createItemWidgets() const
{
    QList<QWidget*> widgets;

    // Create all the possible widgets for displaying the parameter.
    QLabel *nameLabel = new QLabel();
    QLineEdit *lineEdit = new QLineEdit();
    QCheckBox *checkBox = new QCheckBox();
    IntegerEdit *integerEdit = new IntegerEdit();
    UnsignedIntegerEdit *unsignedIntegerEdit = new UnsignedIntegerEdit();

    // Connect to the slots from the widgets that we are interested in.
    connect(lineEdit, SIGNAL(textChanged(QString)), SLOT(onLineEditTextChanged(QString)));
    connect(checkBox, SIGNAL(toggled(bool)), SLOT(onCheckBoxToggled(bool)));
    connect(integerEdit, SIGNAL(textChanged(QString)), SLOT(onIntegerEditTextChanged(QString)));
    connect(unsignedIntegerEdit,
            SIGNAL(textChanged(QString)),
            SLOT(onUnsignedIntegerEditTextChanged(QString)));

    widgets << nameLabel << lineEdit << checkBox << integerEdit << unsignedIntegerEdit;

    return widgets;
}

void ParameterEditDelegate::updateItemWidgets(const QList<QWidget*> widgets,
                                              const QStyleOptionViewItem &option,
                                              const QPersistentModelIndex &index) const
{
    int margin = option.fontMetrics.height() / 2;
    int right = option.rect.width();

    // Draw the label showing the name of the parameter
    QLabel *nameLabel = qobject_cast<QLabel*>(widgets.at(0));

    nameLabel->setText(index.model()->data(index, ParameterEditModel::LocalizedNameRole).toString());
    nameLabel->move(margin, 0);
    nameLabel->resize(QSize(((right - (4 * margin)) / 2), option.rect.height()));

    // Get all the optional input widgets.
    QLineEdit *lineEdit = qobject_cast<QLineEdit*>(widgets.at(1));
    QCheckBox *checkBox = qobject_cast<QCheckBox*>(widgets.at(2));
    IntegerEdit *integerEdit = qobject_cast<IntegerEdit*>(widgets.at(3));
    UnsignedIntegerEdit *unsignedIntegerEdit = qobject_cast<UnsignedIntegerEdit*>(widgets.at(4));

    // See what type the parameter is, and draw the appropriate widget for it.
    // FIXME: Support uint types with appropriate validation.
    if (index.model()->data(index, ParameterEditModel::TypeRole).type() == QVariant::Bool) {
        // Bool type. Draw a checkbox.
        checkBox->move((right / 2) + margin, (option.rect.height() - checkBox->size().height()) / 2);

        checkBox->setFocus(Qt::OtherFocusReason);
        // NB: We must update the value of the widget AFTER setting it as focused, otherwise
        // focusedItem() returns the wrong value and we end up setting the data of the wrong item
        // in the model.
        checkBox->setChecked(index.model()->data(index, ParameterEditModel::ValueRole).toBool());

        // Hide all the other input widgets for this item. This must be done in each condition
        // to avoid them losing focus (and cursor position) when updating the content of them.
        lineEdit->hide();
        integerEdit->hide();
        unsignedIntegerEdit->hide();

    } else if (index.model()->data(index, ParameterEditModel::TypeRole).type() == QVariant::Int) {
        // Integer type. Draw a integer edit.
        integerEdit->move((right / 2) + margin, (option.rect.height() - integerEdit->size().height()) / 2);
        integerEdit->resize(QSize(((right - (4 * margin)) / 2), integerEdit->size().height()));

        // Save the cursor position within the widget so we can restore it after altering the data
        int cursorPosition = integerEdit->cursorPosition();

        integerEdit->setFocus(Qt::OtherFocusReason);
        // NB: We must update the value of the widget AFTER setting it as focused, otherwise
        // focusedItem() returns the wrong value and we end up setting the data of the wrong item
        // in the model.
        integerEdit->setText(index.model()->data(index, ParameterEditModel::ValueRole).toString());

        // Restore the cursor position now the data has been changed.
        integerEdit->setCursorPosition(cursorPosition);

        // Hide all the other input widgets for this item. This must be done in each condition
        // to avoid them losing focus (and cursor position) when updating the content of them.
        lineEdit->hide();
        checkBox->hide();
        unsignedIntegerEdit->hide();

    } else if (index.model()->data(index, ParameterEditModel::TypeRole).type() == QVariant::UInt) {
        // Integer type. Draw a integer edit.
        unsignedIntegerEdit->move((right / 2) + margin,
                                  (option.rect.height() - unsignedIntegerEdit->size().height()) / 2);
        unsignedIntegerEdit->resize(QSize(((right - (4 * margin)) / 2),
                                          unsignedIntegerEdit->size().height()));

        // Save the cursor position within the widget so we can restore it after altering the data
        int cursorPosition = unsignedIntegerEdit->cursorPosition();

        integerEdit->setFocus(Qt::OtherFocusReason);
        // NB: We must update the value of the widget AFTER setting it as focused, otherwise
        // focusedItem() returns the wrong value and we end up setting the data of the wrong item
        // in the model.
        unsignedIntegerEdit->setText(index.model()->data(index, ParameterEditModel::ValueRole).toString());

        // Restore the cursor position now the data has been changed.
        unsignedIntegerEdit->setCursorPosition(cursorPosition);

        // Hide all the other input widgets for this item. This must be done in each condition
        // to avoid them losing focus (and cursor position) when updating the content of them.
        lineEdit->hide();
        checkBox->hide();
        integerEdit->hide();

    } else {
        // For any other type, treat it as a string type.
        // FIXME: Support asterisking out the entry in secret parameters
        lineEdit->move((right / 2) + margin, (option.rect.height() - lineEdit->size().height()) / 2);
        lineEdit->resize(QSize(((right - (4 * margin)) / 2), lineEdit->size().height()));

        // If the parameter is secret, we should replace the contents with asterisks
        if (index.model()->data(index, ParameterEditModel::SecretRole).toBool()) {
            lineEdit->setEchoMode(QLineEdit::Password);
        }

        // Save the cursor position within the widget so we can restore it after altering the data
        int cursorPosition = lineEdit->cursorPosition();

        lineEdit->setFocus(Qt::OtherFocusReason);
        // NB: We must update the value of the widget AFTER setting it as focused, otherwise
        // focusedItem() returns the wrong value and we end up setting the data of the wrong item
        // in the model.
        lineEdit->setText(index.model()->data(index, ParameterEditModel::ValueRole).toString());

        // Restore the cursor position now the data has been changed.
        lineEdit->setCursorPosition(cursorPosition);

        // Hide all the other input widgets for this item. This must be done in each condition
        // to avoid them losing focus (and cursor position) when updating the content of them.
        checkBox->hide();
        integerEdit->hide();
        unsignedIntegerEdit->hide();

    }
}

void ParameterEditDelegate::paint(QPainter *painter,
                                  const QStyleOptionViewItem &option,
                                  const QModelIndex &index) const
{
    Q_UNUSED(index);

    QStyle *style = QApplication::style();
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter, 0);

    painter->save();

    if (option.state & QStyle::State_Selected) {
        painter->setPen(QPen(option.palette.highlightedText().color()));
    } else {
        painter->setPen(QPen(option.palette.text().color()));
    }

    painter->restore();
}

QSize ParameterEditDelegate::sizeHint(const QStyleOptionViewItem &option,
                                      const QModelIndex &index) const
{
    Q_UNUSED(index);

    // FIXME: There must be a better way of calculating these?!?!

    QSize size;

    size.setWidth(option.fontMetrics.height() * 4);
    size.setHeight(option.fontMetrics.height() * 2);

    return size;
}

void ParameterEditDelegate::onLineEditTextChanged(QString text)
{
    kDebug();

    QModelIndex index = focusedIndex();

    Q_EMIT dataChanged(index, QVariant(text), ParameterEditModel::ValueRole);
}

void ParameterEditDelegate::onCheckBoxToggled(bool checked)
{
    kDebug();

    QModelIndex index = focusedIndex();

    Q_EMIT dataChanged(index, QVariant(checked), ParameterEditModel::ValueRole);
}

void ParameterEditDelegate::onIntegerEditTextChanged(const QString &text)
{
    kDebug();

    QModelIndex index = focusedIndex();

    Q_EMIT dataChanged(index, QVariant(text), ParameterEditModel::ValueRole);
}

void ParameterEditDelegate::onUnsignedIntegerEditTextChanged(const QString &text)
{
    kDebug();

    QModelIndex index = focusedIndex();

    Q_EMIT dataChanged(index, QVariant(text), ParameterEditModel::ValueRole);
}


#include "parameter-edit-delegate.moc"

