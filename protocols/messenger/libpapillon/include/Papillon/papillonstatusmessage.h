/*
   papillonstatusmessage.h - Dynamic Personal Status Message for Windows Live Messenger.

   Copyright (c) 2006 by Michaël Larouche <larouche@kde.org>

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
 */
#ifndef PAPILLONSTATUSMESSAGE_H
#define PAPILLONSTATUSMESSAGE_H

#include <Papillon/Macros>
#include <Papillon/Enums>

#include <QtCore/QSharedDataPointer>
#include <QtCore/QList>
#include <QtCore/QVariant>

namespace Papillon
{

/**
 * @class StatusMessage papillonstatusmessage.h <Papillon/StatusMessage>
 * @brief Dynamic Personal Status Message for Windows Live Messenger.
 * A status message can contain a plain status message and can also
 * contain metadata about the current song played and more.
 *
 * Set the plain status message using setMessage().
 * 
 * Example of code for setCurrentMedia():
 * @code
 * Papillon::StatusMessage mediaStatus;
 * QList<QVariant> mediaArgs;
 * QString formatterString;
 *
 * if( musicData.contains("title") )
 * {
 *    formatterString += "{0}";
 *    mediaArgs.append( musicData.value("title") );
 * }
 * if( musicData.contains("artist") )
 * {
 *   formatterString += "- {1}";
 *   mediaArgs.append( musicData.value("artist") );
 * }
 *
 * mediaStatus.setCurrentMediaType( Papillon::Presence::MediaMusic );
 * mediaStatus.setCurrentMediaFormatterString( formatterString );
 * mediaStatus.setCurrentMediaArguments( mediaArgs );
 * @endcode
 *
 * You can use fromXml() to create a StatusMessage from a XML string and
 * toXml() to get the XML string for the current StatusMessage.
 *
 * This class is implicit shared.
 *
 * @author Michaël Larouche <larouche@kde.org>
 */
class PAPILLON_EXPORT StatusMessage
{
public:
	/**
	 * @brief Create a new empty status message.
	 */
	StatusMessage();
	/**
	 * @brief Create a new status message with plain status message already set.
	 * @param plainStatusMessage The plain status message.
	 */
	explicit StatusMessage(const QString &plainStatusMessage);
	/**
	 * @brief Copy constructor
	 * Do not create a deep copy, just increase the reference count.
	 */
	StatusMessage(const StatusMessage &copy);
	/**
	 * @brief Copy-assignment operator
	 * Do not create a deep copy, just increate the reference count.
	 */
	StatusMessage &operator=(const StatusMessage &other);
	~StatusMessage();

	/**
	 * @brief Clear all values of the current StatusMessage.
	 * Use this method to reset all values.
	 */
	void clear();

	/**
	 * @brief Get the current plain status message.
	 * @return Plain status message (also know as personal status message)
	 */
	QString message() const;
	/**
	 * @brief Set the current plain status message.
	 * @param message New status message to set.
	 */
	void setMessage(const QString &message);

	/**
	 * @brief Set if current media should be enabled. (optional)
	 *
	 * This option is optional and it's true by default.
	 * @param enabled Enabled or not.
	 */
	void setCurrentMediaEnabled( bool enabled );
	/**
	 * @brief Set the application for the current media. (optional)
	 *
	 * This option is optional and it's empty by default.
	 * @param application Application name for the current media (ex: Amarok, Kaffeine)
	 */
	void setCurrentMediaApplication( const QString &application );
	/**
	 * @brief Set the current media type. (required)
	 *
	 * Set mediaType to Papillon::OnlineStatus::MediaNone to ignore CurrentMedia.
	 *
	 * @see Papillon::OnlineStatus::MediaType for more details.
	 * @param mediaType Current media type
	 */
	void setCurrentMediaType( Papillon::Presence::MediaType mediaType );
	/**
	 * @brief Set the formatter string. (required)
	 *
	 * The formatter string looks like this(without the quotes): "{0} - {1} ({2})".
	 * Each argument is represented by its index. The formatter string
	 * is how the client application should display the current media.
	 *
	 * @param formatterString the formatter string.
	 */
	void setCurrentMediaFormatterString( const QString &formatterString );
	/**
	 * @brief Set the current media arguments. (required)
	 *
	 * The arguments is the values used to replace the placeholder in the
	 * formatter string.
	 *
	 * @param arguments Argument list.
	 */
	void setCurrentMediaArguments( const QList<QVariant> &arguments );

	/**
	 * @brief Get if the current media should be enabled/displayed.
	 * @return Current media enabled or not.
	 */
	bool isCurrentMediaEnabled() const;
	/**
	 * @brief Get the current media application.
	 * @return Current media application.
	 */
	QString currentMediaApplication() const;
	/**
	 * @brief Get the current media type.
	 *
	 * If this return Papillon::OnlineStatus::MediaNone then you should
	 * ignore all information about current media.
	 * @return Current media type (MediaNone if no type is defined)
	 */
	Papillon::Presence::MediaType currentMediaType() const;
	/**
	 * @brief Get the current media formatter string.
	 *
	 * Formatter string is in the form "{0} - {1} ({2})"
	 * @return Current media formatter string.
	 */
	QString currentMediaFormatterString() const;
	/**
	 * @brief Get the current media arguments.
	 * @return Current media argument list
	 */
	QList<QVariant> currentMediaArguments() const;
	/**
	 * @brief Return a formatted string representation of current media.
	 *
	 * This will result a string such as "Acid Rain - Liquid Tension Experiment (Liquid Tension Experiment 2)",
	 * where the formatter string was "{0} - {1} ({2})".
	 *
	 * @return a formatted represenation of the current media.
	 */
	QString formattedMediaString() const;

	/**
	 * @brief Parse a XML represenation of the personal status message.
	 *
	 * Input should look like: "<Data><PSM>libpapillon rules</PSM><CurrentMedia></CurrentMedia></Data>".
	 *
	 * @param xml XML document as a string.
	 * @return New StatusMessage with information parsed from the XML document.
	 */
	static StatusMessage fromXml(const QString &xml);
	/**
	 * @brief Return a XML representation of current StatusMessage.
	 * This string will be ready to be send to the server.
	 *
	 * The resulting string look like "<Data><PSM>libpapillon rules</PSM><CurrentMedia></CurrentMedia></Data>".
	 *
	 * @return resulting XML document as a string.
	 */
	QString toXml() const;

private:
	class Private;
	QSharedDataPointer<Private> d;
};

}
#endif
