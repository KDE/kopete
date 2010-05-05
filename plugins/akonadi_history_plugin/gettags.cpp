/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*/

#include "gettags.h"
#include <KDebug>
#include <Nepomuk/ResourceManager>
#include <Soprano/QueryResultIterator>
#include <Soprano/Query/QueryLanguage>
#include <Soprano/Model>

GetTags::GetTags(QObject* parent): KJob(parent)
{
    m_query = "select distinct ?r ?p1 where \
	{ ?r <http://www.semanticdesktop.org/ontologies/2007/08/15/nao#hasTag> ?tag  . \
	?tag <http://www.semanticdesktop.org/ontologies/2007/08/15/nao#identifier> ?p1 . \
	FILTER REGEX(STR(?p1),'^kopete:','i') . \
	} " ;
}

void GetTags::start()
{
    Nepomuk::ResourceManager::instance()->init();
    Soprano::QueryResultIterator it
    = Nepomuk::ResourceManager::instance()->mainModel()->executeQuery( m_query , 
                          Soprano::Query::QueryLanguageSparql );
    while( it.next() )
    {
	kDebug() << "found" << it.bindingNames() << it.binding( 0 ).toString();
	QString tag = it.binding( 1 ).toString();
	QStringList splitList = tag.split(":");
	m_tags << splitList.at(1) ;
	kDebug() << m_tags;
    }
    
    emitResult();
}

QStringList GetTags::tags()
{
    kDebug() << m_tags ;
    return m_tags;
}

