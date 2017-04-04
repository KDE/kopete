/*
    highlightconfig.cpp

    Copyright (c) 2003      by Olivier Goffart       <ogoffart@kde.org>
    Copyright (c) 2003      by Matt Rogers           <matt@matt.rogers.name>

    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "highlightconfig.h"

#include <qfile.h>
#include <qregexp.h>
#include <qdir.h>
#include <qdom.h>
#include <QTextCodec>
#include <QTextStream>

#include <QSaveFile>

#include <KLocalizedString>
#include <QStandardPaths>

#include "filter.h"

HighlightConfig::HighlightConfig()
{
}

HighlightConfig::~HighlightConfig()
{
    qDeleteAll(m_filters);
    m_filters.clear();
}

void HighlightConfig::removeFilter(Filter *f)
{
    m_filters.removeAll(f);
    delete f;
}

void HighlightConfig::appendFilter(Filter *f)
{
    m_filters.append(f);
}

QList<Filter *> HighlightConfig::filters() const
{
    return m_filters;
}

Filter *HighlightConfig::newFilter()
{
    Filter *filtre = new Filter();
    filtre->caseSensitive = false;
    filtre->isRegExp = false;
    filtre->setImportance = false;
    filtre->importance = 1;
    filtre->setBG = false;
    filtre->setFG = false;
    filtre->raiseView = false;
    filtre->displayName = i18n("-New filter-");
    m_filters.append(filtre);
    return filtre;
}

void HighlightConfig::load()
{
    m_filters.clear(); //clear filters

    const QString filename = QStandardPaths::writableLocation(QStandardPaths::DataLocation) + QLatin1Char('/') + QStringLiteral("highlight.xml");
    if (filename.isEmpty()) {
        return;
    }

    QDomDocument filterList(QLatin1String("highlight-plugin"));

    QFile filterListFile(filename);
    filterListFile.open(QIODevice::ReadOnly);
    filterList.setContent(&filterListFile);

    QDomElement list = filterList.documentElement();

    QDomNode node = list.firstChild();
    while (!node.isNull())
    {
        QDomElement element = node.toElement();
        if (!element.isNull()) {
//			if( element.tagName() == QString::fromLatin1("filter")
//			{
            Filter *filtre = newFilter();
            QDomNode filterNode = node.firstChild();

            while (!filterNode.isNull())
            {
                QDomElement filterElement = filterNode.toElement();
                if (!filterElement.isNull()) {
                    if (filterElement.tagName() == QLatin1String("display-name")) {
                        filtre->displayName = filterElement.text();
                    } else if (filterElement.tagName() == QLatin1String("search")) {
                        filtre->search = filterElement.text();

                        filtre->caseSensitive = (filterElement.attribute(QStringLiteral("caseSensitive"), QStringLiteral("1")) == QLatin1String("1"));
                        filtre->isRegExp = (filterElement.attribute(QStringLiteral("regExp"), QStringLiteral("0")) == QLatin1String("1"));
                    } else if (filterElement.tagName() == QLatin1String("FG")) {
                        filtre->FG = filterElement.text();
                        filtre->setFG = (filterElement.attribute(QStringLiteral("set"), QStringLiteral("0")) == QLatin1String("1"));
                    } else if (filterElement.tagName() == QLatin1String("BG")) {
                        filtre->BG = filterElement.text();
                        filtre->setBG = (filterElement.attribute(QStringLiteral("set"), QStringLiteral("0")) == QLatin1String("1"));
                    } else if (filterElement.tagName() == QLatin1String("importance")) {
                        filtre->importance = filterElement.text().toUInt();
                        filtre->setImportance = (filterElement.attribute(QStringLiteral("set"), QStringLiteral("0")) == QLatin1String("1"));
                    } else if (filterElement.tagName() == QLatin1String("raise")) {
                        filtre->raiseView = (filterElement.attribute(QStringLiteral("set"), QStringLiteral("0")) == QLatin1String("1"));
                    }
                }
                filterNode = filterNode.nextSibling();
            }
//			}
        }
        node = node.nextSibling();
    }
    filterListFile.close();
}

void HighlightConfig::save()
{
    const QString fileName = QStandardPaths::writableLocation(QStandardPaths::DataLocation) + QLatin1Char('/') + QStringLiteral("highlight.xml");

    QSaveFile file(fileName);
    if (file.open(QIODevice::WriteOnly)) {
        QTextStream stream(&file);
        stream.setCodec(QTextCodec::codecForName("UTF-8"));

        QString xml = QString::fromLatin1(
            "<?xml version=\"1.0\"?>\n"
            "<!DOCTYPE kopete-highlight-plugin>\n"
            "<highlight-plugin>\n");

        // Save metafilter information.
        foreach (Filter *filtre, m_filters) {
            xml += QLatin1String("  <filter>\n    <display-name>")
                   + filtre->displayName.toHtmlEscaped()
                   + QLatin1String("</display-name>\n");

            xml += QLatin1String("    <search caseSensitive=\"") + QString::number(static_cast<int>(filtre->caseSensitive))
                   +QLatin1String("\" regExp=\"") + QString::number(static_cast<int>(filtre->isRegExp))
                   +QLatin1String("\">") + filtre->search.toHtmlEscaped() + QLatin1String("</search>\n");

            xml += QLatin1String("    <BG set=\"") + QString::number(static_cast<int>(filtre->setBG))
                   +QLatin1String("\">") + filtre->BG.name().toHtmlEscaped() + QLatin1String("</BG>\n");
            xml += QLatin1String("    <FG set=\"") + QString::number(static_cast<int>(filtre->setFG))
                   +QLatin1String("\">") + filtre->FG.name().toHtmlEscaped() + QLatin1String("</FG>\n");

            xml += QLatin1String("    <importance set=\"") + QString::number(static_cast<int>(filtre->setImportance))
                   +QLatin1String("\">") + QString::number(filtre->importance) + QLatin1String("</importance>\n");

            xml += QLatin1String("    <raise set=\"") + QString::number(static_cast<int>(filtre->raiseView))
                   +QLatin1String("\"></raise>\n");

            xml += QLatin1String("  </filter>\n");
        }

        xml += QLatin1String("</highlight-plugin>\n");

        stream << xml;
    }
}

// vim: set noet ts=4 sts=4 sw=4:
