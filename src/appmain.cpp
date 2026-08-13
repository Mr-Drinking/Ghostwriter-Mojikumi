/*
 * SPDX-FileCopyrightText: 2014-2024 Megan Conkle <megan.conkle@kdemail.net>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <QApplication>
#include <QColor>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDate>
#include <QDateTime>
#include <QFileInfo>
#include <QGuiApplication>
#include <QLibraryInfo>
#include <QPair>
#include <QLocale>
#include <QPalette>
#include <QStyleHints>
#include <QTranslator>
#include <QWebEnginePage>
#include <QWindow>

#include <KAboutData>
#include <KToolTipHelper>

#include "logging.h"
#include "mainwindow.h"
#include "settings/appsettings.h"
#include "settings/bundledfont.h"

int main(int argc, char *argv[])
{
    // Set up customized logging.
    qSetMessagePattern(
        "[%{time process} %{pid} %{appname} %{if-category} %{category}%{endif}] "
        "%{if-debug}DEBUG   %{endif}"
        "%{if-info}INFO    %{endif}"
        "%{if-warning}WARNING %{endif}"
        "%{if-critical}CRITICAL%{endif}"
        "%{if-fatal}FATAL   %{endif}"
        "%{if-debug}  %{function}():%{endif}"
        "  %{message}"
        "%{if-debug} (%{file}:%{line})%{endif}");
    qInstallMessageHandler(ghostwriter::logMessage);

    bool disableGPU = false;

    // Unfortunately, we must preparse the arguments for the --disable-gpu
    // option rather than using QCommandLineParser since we must set the
    // software rendering attribute before creating the QApplication.
    //
    for (int i = 0; i < argc; i++) {
        if (0 == strcmp(argv[i], "--disable-gpu")) {
            disableGPU = true;
            break;
        }
    }

    if (disableGPU) {
        QCoreApplication::setAttribute(Qt::AA_UseSoftwareOpenGL);
    }

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

#if defined(Q_OS_WIN)
    // For Qt 5, use ANGLE instead of OpenGL to bypass bug where full screen
    // windows under Windows 10 and OpenGL will not show menus from the menu
    // bar (or any other popup menus).  For Qt 6, this is option is no longer
    // available, so allow the user to pass in the software OpenGL option if
    // desired. (Note: Software rendering can be buggy, so leave it optional).
    // Sadly, the full screen OpenGL workaround in Qt's documentation does not
    // actually work.
    //
    // Thank you, Microsoft (and now Qt for removing ANGLE), you made my day.
    //
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    if (!disableGPU) {
        QCoreApplication::setAttribute(Qt::AA_UseOpenGLES, true);
    }
#endif
#endif

    // Disable icons in menus for now, since matching their colors to the
    // current theme is not supported yet.
    // QCoreApplication::setAttribute(Qt::AA_DontShowIconsInMenus, true);

    QApplication app(argc, argv);

    qApp->installEventFilter(KToolTipHelper::instance());

#if defined(Q_OS_LINUX)
    QGuiApplication::setDesktopFileName(
        "io.github.mr_drinking.ghostwriter-mojikumi");
#endif

    KAboutData aboutData("ghostwritermojikumi",
                    QCoreApplication::translate("main",
                        "Ghostwriter Mojikumi"),
                    APPVERSION);

    aboutData.setOrganizationDomain("mr_drinking.github.io");
    aboutData.setShortDescription(QCoreApplication::translate("main",
        "A Markdown editor with CJK mojikumi support"));

    aboutData.addAuthor("Megan Conkle", "Developer",
        "megan.conkle@kdemail.net");
    aboutData.addCredit("Mr-Drinking",
        QCoreApplication::translate("main",
            "Maintainer of Ghostwriter Mojikumi and its CJK spacing changes"),
        QString(),
        "https://github.com/Mr-Drinking/Ghostwriter-Mojikumi");
    aboutData.addCredit("Graeme Gott",
        QCoreApplication::translate("main",
            "FocusWriter developer, whose Qt code mentored me"),
        "graeme@gottcode.org",
        "gottcode.org");
    aboutData.addCredit("Dmitry Shachnev",
        QCoreApplication::translate("main",
            "ReText developer, whose algorithms helped immensely"),
        QString(),
        "https://github.com/retext-project/retext");
    aboutData.addCredit("Gabriel M. Beddingfield",
        QCoreApplication::translate("main",
            "StretchPlayer developer, whose application showed me how to make frameless windows in Qt"),
        QString(),
        "https://www.teuton.org/~gabriel/stretchplayer/");
    aboutData.addCredit("Wolf Vollprecht",
        QCoreApplication::translate("main",
            "UberWriter (now Apostrophe) developer, for providing inspiration"),
        QString(),
        "https://www.wolfvollprecht.de");
    aboutData.addCredit(QCoreApplication::translate("main", "Other Contributors"),
        QCoreApplication::translate("main",
            "Everyone who provided translations, documentation, bug fixes, or new features over the years"),
        QString(),
        QString());
    aboutData.addComponent("cmark-gfm", 
        QCoreApplication::translate("main",
            "An extended version of the C reference implementation of CommonMark"),
        QString(),
        "https://github.com/github/cmark-gfm");
    aboutData.addComponent("React", QCoreApplication::translate("main", "A JavaScript library for building user interfaces"), QString(), "https://reactjs.org");
    aboutData.addComponent("MathJax", 
        QCoreApplication::translate("main",
            "A JavaScript display engine for mathematics"),
        QString(),
        "https://www.mathjax.org/");
    aboutData.addComponent("Noto Sans CJK",
        QCoreApplication::translate("main",
            "The complete, unmodified AOSP Noto Sans CJK static collection "
            "under SIL Open Font License 1.1. Ghostwriter Mojikumi selects "
            "its JP, KR, SC, TC, or HK proportional face from the configured "
            "interface language; those faces include chws and halt."),
        QString(),
        "https://android.googlesource.com/platform/external/noto-fonts/+/aa96a71129acdb7ad8005ab5de269cb506d29655/notosanscjk/");
    aboutData.setLicense(KAboutLicense::GPL_V3);
    aboutData.setCopyrightStatement(QCoreApplication::translate("main",
        "Copyright 2014-%1 The ghostwriter team\n"
        "Ghostwriter Mojikumi modifications copyright 2026 Mr-Drinking")
            .arg(QDateTime::currentDateTime().date().year()));
    aboutData.setOtherText(
        QStringLiteral("<img src=\":/resources/banner.png\"><p>")
        + QCoreApplication::translate("main",
            "Based on KDE ghostwriter commit "
            "db9690507e9ba9194af4ee0dbad66dc4b1507389. Modified on "
            "2026-08-13 to add CJK mojikumi support; independently "
            "maintained by Mr-Drinking and not a KDE release.")
        + QStringLiteral("</p>"));
    aboutData.setHomepage(
        "https://github.com/Mr-Drinking/Ghostwriter-Mojikumi");
    aboutData.setBugAddress(
        "https://github.com/Mr-Drinking/Ghostwriter-Mojikumi/issues");
    aboutData.setDesktopFileName(
        "io.github.mr_drinking.ghostwriter-mojikumi");

    // Set the application metadata.
    KAboutData::setApplicationData(aboutData);

    // Call this to force settings initialization before the application
    // fully launches.
    //
    ghostwriter::AppSettings::instance();

    QString filePath = QString();

    QCommandLineParser clParser;
    aboutData.setupCommandLine(&clParser);
    clParser.setApplicationDescription(QCoreApplication::translate("main",
        "Welcome to Ghostwriter Mojikumi!"));
    clParser.addPositionalArgument("file",
        QCoreApplication::translate("main", "(Optional) File to open."));

    QCommandLineOption renderingOption("disable-gpu",
        QCoreApplication::translate("main", "Disables GPU acceleration."));
    QCommandLineOption verifyBundledFontOption(
        "verify-bundled-font",
        QCoreApplication::translate(
            "main",
            "Verifies the bundled regional font in Qt and the live-preview engine."),
        QCoreApplication::translate("main", "locale"));
    QCommandLineOption verifyTranslationsOption(
        "verify-translations",
        QCoreApplication::translate(
            "main",
            "Verifies that the packaged interface translations can be found and loaded."));
    QCommandLineOption verifyColorSchemesOption(
        "verify-color-schemes",
        QCoreApplication::translate(
            "main",
            "Verifies that the application dark-mode setting reaches the native platform appearance."));
    QCommandLineOption verifyPreviewBackgroundsOption(
        "verify-preview-backgrounds",
        QCoreApplication::translate(
            "main",
            "Verifies that the live-preview surface follows light and dark theme backgrounds."));

    clParser.addOption(renderingOption);
    clParser.addOption(verifyBundledFontOption);
    clParser.addOption(verifyTranslationsOption);
    clParser.addOption(verifyColorSchemesOption);
    clParser.addOption(verifyPreviewBackgroundsOption);
    clParser.process(app);
    aboutData.processCommandLine(&clParser);

    QStringList posArgs = clParser.positionalArguments();

    app.setWindowIcon(QIcon::fromTheme(
        QStringLiteral("ghostwriter"),
        QIcon(QStringLiteral(":/resources/icons/sc-apps-ghostwriter.svg"))));

    if (clParser.isSet(verifyPreviewBackgroundsOption)) {
        const QColor initialBackground(QStringLiteral("#151719"));
        const QColor updatedBackground(QStringLiteral("#fff6ea"));
        ghostwriter::MarkdownDocument document;
        ghostwriter::HtmlPreview preview(
            &document,
            ghostwriter::AppSettings::instance()->currentHtmlExporter(),
            initialBackground);

        const QColor actualInitialBackground =
            preview.page()->backgroundColor();
        preview.setStyleSheet(
            QStringLiteral("body { background-color: #fff6ea; }"),
            updatedBackground);
        const QColor actualUpdatedBackground =
            preview.page()->backgroundColor();

        if ((actualInitialBackground != initialBackground)
            || (actualUpdatedBackground != updatedBackground)) {
            fprintf(stderr,
                "preview-backgrounds-failed initial=%s updated=%s\n",
                qPrintable(actualInitialBackground.name()),
                qPrintable(actualUpdatedBackground.name()));
            return 1;
        }

        fprintf(stdout,
            "preview-backgrounds-ok initial=%s updated=%s\n",
            qPrintable(actualInitialBackground.name()),
            qPrintable(actualUpdatedBackground.name()));
        return 0;
    }

    if (clParser.isSet(verifyColorSchemesOption)) {
        ghostwriter::AppSettings *settings = ghostwriter::AppSettings::instance();
        QStyleHints *styleHints = QGuiApplication::styleHints();

        if (!styleHints) {
            fprintf(stderr, "color-schemes-failed no-style-hints\n");
            return 1;
        }

        settings->setDarkModeEnabled(true);
        QCoreApplication::processEvents();
        const Qt::ColorScheme darkScheme = styleHints->colorScheme();
        const QPalette darkPalette = QGuiApplication::palette();
        const QColor darkWindow = darkPalette.color(QPalette::Window);
        const QColor darkText = darkPalette.color(QPalette::WindowText);

        settings->setDarkModeEnabled(false);
        QCoreApplication::processEvents();
        const Qt::ColorScheme lightScheme = styleHints->colorScheme();
        const QPalette lightPalette = QGuiApplication::palette();
        const QColor lightWindow = lightPalette.color(QPalette::Window);
        const QColor lightText = lightPalette.color(QPalette::WindowText);

        const bool darkPaletteIsDark =
            darkWindow.lightnessF() < darkText.lightnessF();
        const bool lightPaletteIsLight =
            lightWindow.lightnessF() > lightText.lightnessF();

        if ((darkScheme != Qt::ColorScheme::Dark)
            || (lightScheme != Qt::ColorScheme::Light)
            || !darkPaletteIsDark
            || !lightPaletteIsLight) {
            fprintf(stderr,
                "color-schemes-failed platform=%s dark=%d palette=%s/%s light=%d palette=%s/%s\n",
                qPrintable(QGuiApplication::platformName()),
                static_cast<int>(darkScheme),
                qPrintable(darkWindow.name()),
                qPrintable(darkText.name()),
                static_cast<int>(lightScheme),
                qPrintable(lightWindow.name()),
                qPrintable(lightText.name()));
            return 1;
        }

        fprintf(stdout,
            "color-schemes-ok platform=%s dark=%d palette=%s/%s light=%d palette=%s/%s\n",
            qPrintable(QGuiApplication::platformName()),
            static_cast<int>(darkScheme),
            qPrintable(darkWindow.name()),
            qPrintable(darkText.name()),
            static_cast<int>(lightScheme),
            qPrintable(lightWindow.name()),
            qPrintable(lightText.name()));
        return 0;
    }

    if (clParser.isSet(verifyTranslationsOption)) {
        ghostwriter::AppSettings *settings = ghostwriter::AppSettings::instance();
        const QStringList available = settings->availableTranslations();
        const QStringList required {
            QStringLiteral("en"),
            QStringLiteral("ja"),
            QStringLiteral("ko"),
            QStringLiteral("zh_CN"),
            QStringLiteral("zh_TW"),
        };
        QStringList failures;
        for (const QString &locale : required) {
            if (!available.contains(locale)) {
                failures.append(locale + QStringLiteral(":missing"));
            } else if (!settings->canLoadTranslation(locale)) {
                failures.append(locale + QStringLiteral(":unloadable"));
            }
        }
        const QList<QPair<QString, QString>> formatMenuTranslations {
            {QStringLiteral("zh_CN"), QStringLiteral("格式(&O)")},
            {QStringLiteral("zh_TW"), QStringLiteral("格式(&O)")},
        };
        for (const auto &[locale, expected] : formatMenuTranslations) {
            if (!settings->setLocale(locale)) {
                failures.append(locale + QStringLiteral(":not-installed"));
                continue;
            }
            const QString actual = QCoreApplication::translate(
                "ghostwriter::MainWindow",
                "&Format");
            if (actual != expected) {
                failures.append(
                    locale + QStringLiteral(":format-menu=") + actual);
            }
        }
        if (!failures.isEmpty()) {
            fprintf(stderr,
                "translations-failed app-dir=%s flatpak-id=%s flatpak-zh-cn=%d available=%s failures=%s\n",
                qPrintable(QCoreApplication::applicationDirPath()),
                qPrintable(qEnvironmentVariable("FLATPAK_ID")),
                QFileInfo(QStringLiteral(
                    "/app/share/locale/zh_CN/LC_MESSAGES/ghostwriter_qt.qm"))
                        .isFile(),
                qPrintable(available.join(QLatin1Char(','))),
                qPrintable(failures.join(QLatin1Char(','))));
            return 1;
        }
        fprintf(stdout,
            "translations-ok count=%lld locales=%s\n",
            static_cast<long long>(available.size()),
            qPrintable(available.join(QLatin1Char(','))));
        return 0;
    }

    if (clParser.isSet(verifyBundledFontOption)) {
        QString report;
        const QString requestedLocale = clParser.value(verifyBundledFontOption);
        const bool verified = ghostwriter::BundledFont::verifyRendering(
            requestedLocale.isEmpty()
                ? ghostwriter::AppSettings::instance()->locale()
                : requestedLocale,
            &report);
        fprintf(verified ? stdout : stderr, "%s\n", qPrintable(report));
        return verified ? 0 : 1;
    }

    if (posArgs.size() > 0) {
        filePath = posArgs.first();
    }

    // Note: --disable-gpu option was already processed. We added it here
    //       only so it is displayed in the help output.
    ghostwriter::MainWindow window(filePath);

    window.show();
    return app.exec();
}
