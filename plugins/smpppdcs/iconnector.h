/*
    iconnector.h
 
    Copyright (c) 2005-2006 by Heiko Schaefer        <heiko@rangun.de>
 
    Kopete    (c) 2002-2006 by the Kopete developers <kopete-devel@kde.org>
 
    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; version 2 of the License.               *
    *                                                                       *
    *************************************************************************
*/

#ifndef ICONNECTOR_H
#define ICONNECTOR_H

/**
 * @brief Interface to an object setting a connection status.
 *
 * @author Heiko Sch&auml;fer <heiko@rangun.de>
 */
class IConnector {
    IConnector(const IConnector&);
    IConnector& operator=(const IConnector&);

public:
    IConnector() {}

    virtual ~IConnector() {}

    /**
     * @brief Set the connection status.
     *
     * This method needs to get reimplemented at classes which implement
     * this interface.
     *
     * @param newStatus the status of the internet connection, <code>TRUE</code> if there is a connection, otherwise <code>FALSE</code>
     */
    virtual void setConnectedStatus(bool newStatus) = 0;
};

#endif
