/*
 * SPDX-FileCopyrightText: 2026 Mr-Drinking
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef BUNDLEDFONT_H
#define BUNDLEDFONT_H

#include <QLocale>
#include <QString>
#include <QStringList>

namespace ghostwriter::BundledFont
{
inline const QString ResourcePath =
    QStringLiteral(":/fonts/NotoSansCJK-Regular.ttc");
inline const QString LegacyPrivateFamily =
    QStringLiteral("Ghostwriter Mojikumi CJK SC");
inline const QString Sha256 = QStringLiteral(
    "39fb47c543da50618ab99e8b9e5529e54566bdbef41719308165975f627d5c93");

QString privateFilePath();
QString webFontFileUrl();
bool installPrivateFile();
bool verifyRendering(const QString &localeName, QString *report);

inline QString regionForLocale(const QString &localeName)
{
    const QLocale locale(localeName);

    switch (locale.language()) {
    case QLocale::Japanese:
        return QStringLiteral("JP");
    case QLocale::Korean:
        return QStringLiteral("KR");
    case QLocale::Chinese:
        if (locale.territory() == QLocale::HongKong
            || locale.territory() == QLocale::Macau) {
            return QStringLiteral("HK");
        }
        if (locale.territory() == QLocale::Taiwan
            || locale.script() == QLocale::TraditionalHanScript) {
            return QStringLiteral("TC");
        }
        return QStringLiteral("SC");
    default:
        // Ghostwriter Mojikumi primarily targets Chinese writing. Keep its
        // previous SC default when the interface language is not CJK.
        return QStringLiteral("SC");
    }
}

inline QString familyForRegion(const QString &region)
{
    return QStringLiteral("Noto Sans CJK %1").arg(region);
}

inline QString familyForLocale(const QString &localeName)
{
    return familyForRegion(regionForLocale(localeName));
}

inline QString postScriptNameForRegion(const QString &region)
{
    QString suffix = region.toLower();
    return QStringLiteral("NotoSansCJK%1-Regular").arg(suffix);
}

inline QString postScriptNameForLocale(const QString &localeName)
{
    return postScriptNameForRegion(regionForLocale(localeName));
}

inline QString regionForFamily(const QString &family)
{
    for (const QString &region : {QStringLiteral("JP"),
                                  QStringLiteral("KR"),
                                  QStringLiteral("SC"),
                                  QStringLiteral("TC"),
                                  QStringLiteral("HK")}) {
        if (family.compare(familyForRegion(region), Qt::CaseInsensitive) == 0) {
            return region;
        }
    }
    return QStringLiteral("SC");
}

inline QString postScriptNameForFamily(const QString &family)
{
    return postScriptNameForRegion(regionForFamily(family));
}

inline QString htmlLanguageForLocale(const QString &localeName)
{
    const QString region = regionForLocale(localeName);
    if (region == QStringLiteral("JP")) {
        return QStringLiteral("ja");
    }
    if (region == QStringLiteral("KR")) {
        return QStringLiteral("ko");
    }
    if (region == QStringLiteral("TC")) {
        return QStringLiteral("zh-Hant-TW");
    }
    if (region == QStringLiteral("HK")) {
        return QStringLiteral("zh-Hant-HK");
    }
    return QStringLiteral("zh-Hans");
}

inline QStringList proportionalFamilies()
{
    return {
        familyForRegion(QStringLiteral("JP")),
        familyForRegion(QStringLiteral("KR")),
        familyForRegion(QStringLiteral("SC")),
        familyForRegion(QStringLiteral("TC")),
        familyForRegion(QStringLiteral("HK")),
    };
}

inline QStringList collectionFamilies()
{
    QStringList families = proportionalFamilies();
    for (const QString &region : {QStringLiteral("JP"),
                                  QStringLiteral("KR"),
                                  QStringLiteral("SC"),
                                  QStringLiteral("TC"),
                                  QStringLiteral("HK")}) {
        families.append(QStringLiteral("Noto Sans Mono CJK %1").arg(region));
    }
    return families;
}

inline bool isCompatibleFamily(const QString &family)
{
    return proportionalFamilies().contains(family, Qt::CaseInsensitive);
}

inline bool isBundledSelection(const QString &family)
{
    return family.compare(LegacyPrivateFamily, Qt::CaseInsensitive) == 0
        || isCompatibleFamily(family);
}
}

#endif // BUNDLEDFONT_H
