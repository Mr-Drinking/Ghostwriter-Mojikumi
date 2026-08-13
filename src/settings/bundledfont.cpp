/*
 * SPDX-FileCopyrightText: 2026 Mr-Drinking
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "bundledfont.h"

#include <QCryptographicHash>
#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QFont>
#include <QFontDatabase>
#include <QFontMetricsF>
#include <QJsonParseError>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QStandardPaths>
#include <QTimer>
#include <QUrl>
#include <QWebEnginePage>
#include <QWebEngineSettings>

#include <functional>

namespace ghostwriter::BundledFont
{
namespace
{
QByteArray fileSha256(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return {};
    }

    QCryptographicHash hash(QCryptographicHash::Sha256);
    if (!hash.addData(&file)) {
        return {};
    }
    return hash.result().toHex();
}
}

QString privateFilePath()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation)
        + QStringLiteral("/fonts/NotoSansCJK-Regular.ttc");
}

QString webFontFileUrl()
{
    return QUrl::fromLocalFile(privateFilePath()).toString(QUrl::FullyEncoded);
}

bool installPrivateFile()
{
    const QByteArray expectedHash = Sha256.toLatin1();
    const QString destinationPath = privateFilePath();
    if (fileSha256(destinationPath) == expectedHash) {
        return true;
    }

    QFile source(ResourcePath);
    if (!source.open(QIODevice::ReadOnly)) {
        return false;
    }
    if (!QDir().mkpath(QFileInfo(destinationPath).absolutePath())) {
        return false;
    }

    QSaveFile destination(destinationPath);
    if (!destination.open(QIODevice::WriteOnly)) {
        return false;
    }

    QByteArray buffer(1024 * 1024, '\0');
    while (!source.atEnd()) {
        const qint64 bytesRead = source.read(buffer.data(), buffer.size());
        if (bytesRead < 0 || destination.write(buffer.constData(), bytesRead) != bytesRead) {
            destination.cancelWriting();
            return false;
        }
    }

    if (!destination.commit()) {
        return false;
    }
    return fileSha256(destinationPath) == expectedHash;
}

bool verifyRendering(const QString &localeName, QString *report)
{
    const QString family = familyForLocale(localeName);
    if (!QFontDatabase::families().contains(family)) {
        if (report) {
            *report = QStringLiteral("Qt font family is unavailable: %1").arg(family);
        }
        return false;
    }
    if (!installPrivateFile()) {
        if (report) {
            *report = QStringLiteral("Application-private font file failed its SHA-256 check");
        }
        return false;
    }

    const QString sample = QStringLiteral("句，「『正文』」。");
    QFont qtNormal(family);
    qtNormal.setPixelSize(40);
    qtNormal.setFeature(QFont::Tag("chws"), 0);
    QFont qtCompressed(qtNormal);
    qtCompressed.setFeature(QFont::Tag("chws"), 1);
    const qreal qtNormalWidth = QFontMetricsF(qtNormal).horizontalAdvance(sample);
    const qreal qtCompressedWidth =
        QFontMetricsF(qtCompressed).horizontalAdvance(sample);
    if (qtCompressedWidth >= qtNormalWidth - 1.0) {
        if (report) {
            *report = QStringLiteral("Qt did not apply chws for %1 (%2 >= %3)")
                          .arg(family)
                          .arg(qtCompressedWidth)
                          .arg(qtNormalWidth);
        }
        return false;
    }

    const QString fontUrl =
        webFontFileUrl() + QLatin1Char('#') + postScriptNameForLocale(localeName);
    QString fontSource = QStringLiteral("url(\"$font-url\") format(\"collection\")");
    fontSource.replace(QStringLiteral("$font-url"), fontUrl);
    QJsonArray sourceArray;
    sourceArray.append(fontSource);
    QByteArray quotedSourceBytes =
        QJsonDocument(sourceArray).toJson(QJsonDocument::Compact);
    quotedSourceBytes.remove(0, 1);
    quotedSourceBytes.chop(1);
    const QString quotedSource = QString::fromUtf8(quotedSourceBytes);

    const QString html = QStringLiteral(R"HTML(
<!doctype html><meta charset="utf-8"><span id="sample">句，「『正文』」。</span>
<script>
window.__mojikumiFontResult = "";
(async () => {
  const face = new FontFace(
    "GhostwriterMojikumiSmoke",
    %1);
  document.fonts.add(face);
  await face.load();
  const sample = document.getElementById("sample");
  sample.style.cssText =
    "font-family:GhostwriterMojikumiSmoke;font-size:40px;" +
    "display:inline-block;white-space:nowrap;text-spacing-trim:space-all";
  sample.style.fontFeatureSettings = '"chws" 0';
  const normal = sample.getBoundingClientRect().width;
  sample.style.fontFeatureSettings = '"chws" 1';
  const compressed = sample.getBoundingClientRect().width;
  window.__mojikumiFontResult = JSON.stringify({
    status: face.status,
    available: document.fonts.check("40px GhostwriterMojikumiSmoke"),
    trimSupported: CSS.supports("text-spacing-trim", "trim-start"),
    normal,
    compressed
  });
})().catch(error => {
  window.__mojikumiFontResult = JSON.stringify({error: String(error)});
});
</script>)HTML").arg(quotedSource);

    QWebEnginePage page;
    page.settings()->setAttribute(
        QWebEngineSettings::LocalContentCanAccessFileUrls, true);

    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);
    bool loaded = false;
    QString webResult;

    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    QObject::connect(&page, &QWebEnginePage::loadFinished, &loop, [&](bool ok) {
        if (!ok) {
            webResult = QStringLiteral("{\"error\":\"preview page load failed\"}");
            loop.quit();
            return;
        }
        loaded = true;
    });

    std::function<void()> poll;
    poll = [&]() {
        page.runJavaScript(
            QStringLiteral("window.__mojikumiFontResult || ''"),
            [&](const QVariant &value) {
                webResult = value.toString();
                if (!webResult.isEmpty()) {
                    loop.quit();
                } else {
                    QTimer::singleShot(100, &page, poll);
                }
            });
    };

    QTimer pollStarter;
    pollStarter.setInterval(100);
    QObject::connect(&pollStarter, &QTimer::timeout, &page, [&]() {
        if (loaded) {
            pollStarter.stop();
            poll();
        }
    });

    timeout.start(45000);
    pollStarter.start();
    page.setHtml(
        html,
        QUrl::fromLocalFile(QFileInfo(privateFilePath()).absolutePath() + QLatin1Char('/')));
    loop.exec();

    QJsonParseError parseError;
    const QJsonDocument resultDocument =
        QJsonDocument::fromJson(webResult.toUtf8(), &parseError);
    const QJsonObject result = resultDocument.object();
    const qreal webNormalWidth = result.value(QStringLiteral("normal")).toDouble();
    const qreal webCompressedWidth = result.value(QStringLiteral("compressed")).toDouble();
    const bool webOk = timeout.isActive()
        && result.value(QStringLiteral("error")).toString().isEmpty()
        && result.value(QStringLiteral("status")).toString() == QStringLiteral("loaded")
        && parseError.error == QJsonParseError::NoError
        && result.value(QStringLiteral("available")).toBool()
        && result.value(QStringLiteral("trimSupported")).toBool()
        && webCompressedWidth < webNormalWidth - 1.0;

    if (report) {
        *report = webOk
            ? QStringLiteral("bundled-font-ok family=%1 qt=%2->%3 web=%4->%5 sha256=%6")
                  .arg(family)
                  .arg(qtNormalWidth)
                  .arg(qtCompressedWidth)
                  .arg(webNormalWidth)
                  .arg(webCompressedWidth)
                  .arg(Sha256)
            : QStringLiteral("WebEngine font verification failed: %1").arg(webResult);
    }
    return webOk;
}
}
