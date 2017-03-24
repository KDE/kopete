/***************************************************************************
 *   Copyright (c) 2008  Nikolaj Hald Nielsen <nhnFreespirit@gmail.com>    *
 *   Copyright (c) 2009  Roman Jarosz         <kedgedev@gmail.com>         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include "contactlistlayoutmanager.h"

#include <KMessageBox>
#include <KStandardDirs>
#include <QUrl>
#include <KGlobal>
#include <KConfig>
#include <KConfigGroup>
#include <KDebug>
#include <KLocale>
#include <KStandardDirs>
#include <kio/global.h>
#include <kio/job.h>

#include <QDir>
#include <QDomDocument>
#include <QFile>
#include <QStringList>

#include "kopeteitembase.h"

namespace ContactList {
LayoutManager *LayoutManager::s_instance = 0;

const QString DefaultStyleName = QStringLiteral("Default");

LayoutManager *LayoutManager::instance()
{
    if (s_instance == 0) {
        s_instance = new LayoutManager();
    }

    return s_instance;
}

LayoutManager::LayoutManager()
    : QObject()
{
    m_tokens << ContactListTokenConfig(-1, QStringLiteral("Placeholder"), i18n("Placeholder"), QStringLiteral("transform-move"));
    m_tokens << ContactListTokenConfig(Qt::DisplayRole, QStringLiteral("DisplayName"), i18n("Display Name"), QStringLiteral("user-identity"));
    m_tokens << ContactListTokenConfig(Kopete::Items::StatusTitleRole, QStringLiteral("StatusTitle"), i18n("Status Title"), QStringLiteral("im-status-message-edit"));
    m_tokens << ContactListTokenConfig(Kopete::Items::StatusMessageRole, QStringLiteral("StatusMessage"), i18n("Status Message"), QStringLiteral("im-status-message-edit"));
    m_tokens << ContactListTokenConfig(-1, QStringLiteral("ContactIcons"), i18n("Contact Icons"), QStringLiteral("user-online"));

    loadDefaultLayouts();
    loadUserLayouts();

    KConfigGroup config(KSharedConfig::openConfig(), "ContactList Layout");
    m_activeLayout = config.readEntry("CurrentLayout", DefaultStyleName);

    // Fallback to default
    if (!m_layouts.contains(m_activeLayout)) {
        setActiveLayout(DefaultStyleName);
    }
}

LayoutManager::~LayoutManager()
{
}

QStringList LayoutManager::layouts() const
{
    return m_layouts.keys();
}

void LayoutManager::setActiveLayout(const QString &layout)
{
    qDebug() << layout;
    m_activeLayout = layout;
    KConfigGroup config(KSharedConfig::openConfig(), "ContactList Layout");
    config.writeEntry("CurrentLayout", m_activeLayout);
    emit(activeLayoutChanged());
}

void LayoutManager::setPreviewLayout(const ContactListLayout &layout)
{
    m_activeLayout = QStringLiteral("%%PREVIEW%%");
    m_previewLayout = layout;
    emit(activeLayoutChanged());
}

ContactListLayout LayoutManager::activeLayout()
{
    if (m_activeLayout == QLatin1String("%%PREVIEW%%")) {
        return m_previewLayout;
    }
    return m_layouts.value(m_activeLayout);
}

void LayoutManager::loadUserLayouts()
{
    QDir layoutsDir = QDir(KStandardDirs::locateLocal("appdata", QStringLiteral("contactlistlayouts")));

    layoutsDir.setSorting(QDir::Name);

    QStringList filters;
    filters << QStringLiteral("*.xml") << QStringLiteral("*.XML");
    layoutsDir.setNameFilters(filters);
    layoutsDir.setSorting(QDir::Name);

    QFileInfoList list = layoutsDir.entryInfoList();

    for (int i = 0; i < list.size(); ++i) {
        QFileInfo fileInfo = list.at(i);
        qDebug() << "found user file: " << fileInfo.fileName();
        loadLayouts(layoutsDir.filePath(fileInfo.fileName()), true);
    }
}

void LayoutManager::loadDefaultLayouts()
{
    loadLayouts(KStandardDirs::locate("data", QStringLiteral("kopete/DefaultContactListLayouts.xml")), false);
    loadLayouts(KStandardDirs::locate("data", QStringLiteral("kopete/CompactContactListLayouts.xml")), false);
}

void LayoutManager::loadLayouts(const QString &fileName, bool user)
{
    QDomDocument doc(QStringLiteral("layouts"));
    if (!QFile::exists(fileName)) {
        qDebug() << "file " << fileName << "does not exist";
        return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "error reading file " << fileName;
        return;
    }
    if (!doc.setContent(&file)) {
        qDebug() << "error parsing file " << fileName;
        return;
    }
    file.close();

    QDomElement layouts_element = doc.firstChildElement(QStringLiteral("contactlist_layouts"));
    QDomNodeList layouts = layouts_element.elementsByTagName(QStringLiteral("layout"));

    int index = 0;
    while (index < layouts.size())
    {
        QDomNode layout = layouts.item(index);
        index++;

        QString layoutName = layout.toElement().attribute(QStringLiteral("name"), QLatin1String(""));
        ContactListLayout currentLayout;
        currentLayout.setIsEditable(user);

        currentLayout.setLayout(parseItemConfig(layout.toElement()));

        if (!layoutName.isEmpty()) {
            m_layouts.insert(layoutName, currentLayout);
        }
    }
}

LayoutItemConfig LayoutManager::parseItemConfig(const QDomElement &elem)
{
    const bool showMetaContactIcon = (elem.attribute(QStringLiteral("show_metacontact_icon"), QStringLiteral("false")).compare(QLatin1String("true"), Qt::CaseInsensitive) == 0);

    LayoutItemConfig config;
    config.setShowIcon(showMetaContactIcon);

    QDomNodeList rows = elem.elementsByTagName(QStringLiteral("row"));

    int index = 0;
    while (index < rows.size())
    {
        QDomNode rowNode = rows.item(index);
        index++;

        LayoutItemConfigRow row;

        QDomNodeList elements = rowNode.toElement().elementsByTagName(QStringLiteral("element"));

        int index2 = 0;
        while (index2 < elements.size())
        {
            QDomNode elementNode = elements.item(index2);
            index2++;

            int value = 0; // Placeholder as default
            QString configName = elementNode.toElement().attribute(QStringLiteral("value"));
            if (!configName.isEmpty()) {
                for (int i = 0; i < m_tokens.size(); i++) {
                    if (m_tokens.at(i).mConfigName == configName) {
                        value = i;
                        break;
                    }
                }
            }

            QString prefix = elementNode.toElement().attribute(QStringLiteral("prefix"), QString());
            QString sufix = elementNode.toElement().attribute(QStringLiteral("suffix"), QString());
            qreal size = elementNode.toElement().attribute(QStringLiteral("size"), QStringLiteral("0.0")).toDouble();
            bool bold = (elementNode.toElement().attribute(QStringLiteral("bold"), QStringLiteral("false")).compare(QLatin1String("true"), Qt::CaseInsensitive) == 0);
            bool italic = (elementNode.toElement().attribute(QStringLiteral("italic"), QStringLiteral("false")).compare(QLatin1String("true"), Qt::CaseInsensitive) == 0);
            bool small = (elementNode.toElement().attribute(QStringLiteral("small"), QStringLiteral("false")).compare(QLatin1String("true"), Qt::CaseInsensitive) == 0);
            bool optimalSize = (elementNode.toElement().attribute(QStringLiteral("optimalSize"), QStringLiteral("false")).compare(QLatin1String("true"), Qt::CaseInsensitive) == 0);
            QString alignmentString = elementNode.toElement().attribute(QStringLiteral("alignment"), QStringLiteral("left"));
            Qt::Alignment alignment;

            if (alignmentString.compare(QLatin1String("left"), Qt::CaseInsensitive) == 0) {
                alignment = Qt::AlignLeft | Qt::AlignVCenter;
            } else if (alignmentString.compare(QLatin1String("right"), Qt::CaseInsensitive) == 0) {
                alignment = Qt::AlignRight| Qt::AlignVCenter;
            } else {
                alignment = Qt::AlignCenter| Qt::AlignVCenter;
            }

            row.addElement(LayoutItemConfigRowElement(value, size, bold, italic, small, optimalSize, alignment, prefix, sufix));
        }

        config.addRow(row);
    }

    return config;
}

ContactListLayout LayoutManager::layout(const QString &layout)
{
    return m_layouts.value(layout);
}

bool LayoutManager::addUserLayout(const QString &name, ContactListLayout layout)
{
    layout.setIsEditable(true);

    QDomDocument doc(QStringLiteral("layouts"));
    QDomElement layouts_element = doc.createElement(QStringLiteral("contactlist_layouts"));
    QDomElement newLayout = createItemElement(doc, QStringLiteral("layout"), layout.layout());
    newLayout.setAttribute(QStringLiteral("name"), name);

    doc.appendChild(layouts_element);
    layouts_element.appendChild(newLayout);

    QString dirName = KStandardDirs::locateLocal("appdata", QStringLiteral("contactlistlayouts"));
    QDir layoutsDir = QDir(dirName);

    //make sure that this dir exists
    if (!layoutsDir.exists()) {
        if (!layoutsDir.mkpath(dirName)) {
            KMessageBox::sorry(0, KIO::buildErrorString(KIO::ERR_COULD_NOT_MKDIR, dirName));
            return false;
        }
    }

    QFile file(layoutsDir.filePath(name + ".xml"));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        KMessageBox::sorry(0, KIO::buildErrorString(KIO::ERR_CANNOT_OPEN_FOR_WRITING, file.fileName()));
        return false;
    }

    QTextStream out(&file);
    out << doc.toString();
    file.close();

    m_layouts.insert(name, layout);
    emit(layoutListChanged());
    return true;
}

QDomElement LayoutManager::createItemElement(QDomDocument doc, const QString &name, const LayoutItemConfig &item) const
{
    QDomElement element = doc.createElement(name);

    QString showIcon = item.showIcon() ? "true" : "false";
    element.setAttribute(QStringLiteral("show_metacontact_icon"), showIcon);

    for (int i = 0; i < item.rows(); i++) {
        LayoutItemConfigRow row = item.row(i);

        QDomElement rowElement = doc.createElement(QStringLiteral("row"));
        element.appendChild(rowElement);

        for (int j = 0; j < row.count(); j++) {
            LayoutItemConfigRowElement element = row.element(j);
            QDomElement elementElement = doc.createElement(QStringLiteral("element"));

            elementElement.setAttribute(QStringLiteral("value"), m_tokens.at(element.value()).mConfigName);
            elementElement.setAttribute(QStringLiteral("size"), QString::number(element.size()));
            elementElement.setAttribute(QStringLiteral("bold"), element.bold() ? "true" : "false");
            elementElement.setAttribute(QStringLiteral("italic"), element.italic() ? "true" : "false");
            elementElement.setAttribute(QStringLiteral("small"), element.small() ? "true" : "false");
            elementElement.setAttribute(QStringLiteral("optimalSize"), element.optimalSize() ? "true" : "false");

            QString alignmentString;
            if (element.alignment() & Qt::AlignLeft) {
                alignmentString = QStringLiteral("left");
            } else if (element.alignment() & Qt::AlignRight) {
                alignmentString = QStringLiteral("right");
            } else {
                alignmentString = QStringLiteral("center");
            }

            elementElement.setAttribute(QStringLiteral("alignment"), alignmentString);

            rowElement.appendChild(elementElement);
        }
    }

    return element;
}

bool LayoutManager::isDefaultLayout(const QString &layout) const
{
    if (m_layouts.keys().contains(layout)) {
        return !m_layouts.value(layout).isEditable();
    }

    return false;
}

QString LayoutManager::activeLayoutName() const
{
    return m_activeLayout;
}

bool LayoutManager::deleteLayout(const QString &layout)
{
    //check if layout is editable
    if (m_layouts.value(layout).isEditable()) {
        QDir layoutsDir = QDir(KStandardDirs::locateLocal("appdata", QStringLiteral("contactlistlayouts")));
        QString xmlFile = layoutsDir.path() + '/' + layout + ".xml";
        qDebug() << "deleting file: " << xmlFile;

        if (!QFile::remove(xmlFile)) {
            KMessageBox::sorry(0, KIO::buildErrorString(KIO::ERR_CANNOT_DELETE, xmlFile));
            return false;
        }

        m_layouts.remove(layout);
        emit(layoutListChanged());

        if (layout == m_activeLayout) {
            setActiveLayout(DefaultStyleName);
        }

        return true;
    }

    KMessageBox::sorry(0, i18n("The layout '%1' is one of the default layouts and cannot be deleted.", layout),
                       i18n("Cannot Delete Default Layouts"));
    return false;
}
} //namespace ContactList
