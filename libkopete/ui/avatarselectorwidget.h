/*
    avatarselectorwidget.h - Widget to manage and select user avatar

    Copyright (c) 2007      by Michaël Larouche      <larouche@kde.org>

    Kopete    (c) 2002-2007 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/
#ifndef KOPETE_AVATARSELECTORWIDGET_H
#define KOPETE_AVATARSELECTORWIDGET_H

#include <QtGui/QWidget>

// Kopete includes
#include <kopete_export.h>
#include <kopeteavatarmanager.h>

class QListWidgetItem;
class KJob;

namespace Kopete
{

namespace UI
{

/**
 * @brief Widget to select and manager user avatar.
 *
 * Do not use this widget alone, use AvatarSelectorDialog instead.
 * 
 * @sa AvatarSelectorDialog
 * @author Michaël Larouche <larouche@kde.org>
 */
class KOPETE_EXPORT AvatarSelectorWidget : public QWidget
{
	Q_OBJECT
public:
	/**
	 * @brief Create a new AvatarSelectorWidget
	 * @param parent parent widget
	 */
	AvatarSelectorWidget(QWidget *parent = 0);
	/**
	 * @brief Destructor
	 */
	virtual ~AvatarSelectorWidget();

	/**
	 * @brief Get the selected AvatarEntry
	 *
	 * This is a convience method to retrieve the AvatarEntry
	 * from current selected avatar.
	 *
	 * @return AvatarEntry of current selected avatar in list.
	 */
	Kopete::AvatarManager::AvatarEntry selectedEntry() const;

	/**
	 * @brief Set the avatar currently being used
	 *
	 * This is used to select the avatar in the avatar list
	 */
	void setCurrentAvatar(const QString &path);
Q_SIGNALS:
	/**
	 * User has selected another avatar in the list.
	 */
	void avatarChanged();
	
private Q_SLOTS:
	/**
	 * @internal
	 * Add Avatar button was clicked
	 */
	void buttonAddAvatarClicked();
	
	/**
	 * @internal
	 * Remove Avatar button was clicked
	 */
	void buttonRemoveAvatarClicked();
	
	/**
	 * @internal
	 * Avatar query job was finished
	 */
	void queryJobFinished(KJob *job);

	/**
	 * @internal
	 * A new avatar was added into storage
	 * @param newEntry new avatar Entry
	 */
	void avatarAdded(Kopete::AvatarManager::AvatarEntry newEntry);

	/**
	 * @internal
	 * An avatar has been removed from storage
	 * @param entryRemoved Avatar entry removed
	 */
	void avatarRemoved(Kopete::AvatarManager::AvatarEntry entryRemoved);

	/**
	 * @internal
	 * A new item was selected in lists
	 * @param item new selected QListWidgetItem
	 */
	void listSelectionChanged(QListWidgetItem *item);

	/**
	 * @internal
	 * The current tab has changed
	 * @param index the index of the selected tab
	 */
	void currentTabChanged(int index);

private:
	Q_DISABLE_COPY(AvatarSelectorWidget)

	class Private;
	Private *d;
};

}

}

#endif
