/*
    kopeteplugindataobject.h - Kopete Plugin Data Object

    Copyright (c) 2003 by Olivier Goffart        <ogoffart@tiscalinet.be>
    Kopete    (c) 2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/



#ifndef KOPETEPLUGINDATAOBJECT_H
#define KOPETEPLUGINDATAOBJECT_H

#include <qobject.h>
#include <qmap.h>
#include <qdom.h>

class KopetePlugin;
class QDomElement;

/**
 * @author Olivier Goffart  <ogoffart@tiscalinet.be>
 * this class hanlde the saving to xml's file plugin data.
 * KopeteMetaContact, KopeteGroup, and KopeteAccount inherits from it
 */

class KopetePluginDataObject : public QObject
{
public:
    KopetePluginDataObject(QObject *parent=0l, const char *name=0L);

    ~KopetePluginDataObject();

	/**
	 * Set the plugin-specific data.
	 * The data in the provided QMap is a set of key/value pairs.
	 * Note that protocol plugins usually shouldn't use this method, but
	 * reimplement @ref KopeteContact::serialize() instead. This method
	 * is called by @ref KopeteProtocol for those classes.
	 * It is fine to call this method from non-protocol plugins.
	 */
	void setPluginData( KopetePlugin *p, const QMap<QString, QString> &value );

	/**
	 * Get the settings as stored previously by calls to @ref setPluginData()
	 *
	 * Note that calling this method for protocol plugins that use the
	 * @ref KopeteContact::serialize() API may yield unexpected results.
	 */
	QMap<QString, QString> pluginData( KopetePlugin *p ) const;

	/**
	 * Convenience method to store or change only a single field of the
	 * plugin data. As with the other @ref setPluginData() method, protocols
	 * are advised not to use this method and reimplement
	 * @ref KopeteContact::serialize() instead. This method is meant for use
	 * by non-protocol plugins.
	 */
	void setPluginData( KopetePlugin *p, const QString &key, const QString &value );

	/**
	 * Convenience method to retrieve only a single field from the plugin
	 * data. See @ref setPluginData().
	 *
	 * Note that calling this method for protocol plugins that use the
	 * @ref KopeteContact::serialize() API may yield unexpected results.
	 */
	QString pluginData( KopetePlugin *p, const QString &key ) const;

protected:

	/**
	 * Return a XML representation of plugin datas
	 */
	const QValueList<QDomElement> toXML();

	/**
	 * Load plugin datas from one Dom Element:
	 * It has to be a <plugin-data> element
	 */
	void fromXML( const QDomElement& element );


private:
	QMap<QString, QMap<QString, QString> > m_pluginData;

};

#endif
