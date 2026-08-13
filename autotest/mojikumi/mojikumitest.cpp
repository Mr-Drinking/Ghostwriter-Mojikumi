/*
 * SPDX-FileCopyrightText: 2026 Mr-Drinking
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <QAbstractTextDocumentLayout>
#include <algorithm>
#include <QApplication>
#include <QColor>
#include <QFontDatabase>
#include <QFontMetricsF>
#include <QPlainTextEdit>
#include <QSignalSpy>
#include <QSyntaxHighlighter>
#include <QSet>
#include <QTest>
#include <QTextBlock>
#include <QTextLayout>

#include "../../src/editor/markdownstates.h"
#include "../../src/editor/mojikumidecorator.h"
#include "../../src/settings/appsettings.h"

using namespace ghostwriter;

namespace
{
constexpr QFont::Tag ChwsTag{"chws"};
constexpr QFont::Tag HaltTag{"halt"};

QVector<QTextLayout::FormatRange> mojikumiRanges(const QTextBlock &block)
{
    QVector<QTextLayout::FormatRange> result;
    if (!block.isValid() || !block.layout()) {
        return result;
    }

    for (const QTextLayout::FormatRange &range : block.layout()->formats()) {
        if (MojikumiDecorator::isMojikumiFormat(range.format)) {
            result.append(range);
        }
    }
    return result;
}

QVector<int> visualLineStarts(const QTextBlock &block)
{
    QVector<int> starts;
    if (!block.isValid() || !block.layout()) {
        return starts;
    }

    for (int i = 0; i < block.layout()->lineCount(); ++i) {
        starts.append(block.layout()->lineAt(i).textStart());
    }
    return starts;
}

void showAndLayOut(QPlainTextEdit &editor)
{
    editor.show();
    QApplication::processEvents();
    const QTextBlock block = editor.document()->begin();
    if (block.isValid()) {
        editor.document()->documentLayout()->blockBoundingRect(block);
    }
}

QVector<int> baselineLineStarts(
    const QString &text,
    const QFont &font,
    const QSize &size)
{
    QPlainTextEdit baseline;
    baseline.setLineWrapMode(QPlainTextEdit::WidgetWidth);
    baseline.setWordWrapMode(QTextOption::WrapAnywhere);
    baseline.setFixedSize(size);
    baseline.setFont(font);
    baseline.setPlainText(text);
    showAndLayOut(baseline);
    return visualLineStarts(baseline.document()->begin());
}

class UnderlineHighlighter : public QSyntaxHighlighter
{
public:
    explicit UnderlineHighlighter(QTextDocument *document)
        : QSyntaxHighlighter(document)
    {
    }

protected:
    void highlightBlock(const QString &text) override
    {
        QTextCharFormat format;
        format.setForeground(Qt::darkGreen);
        format.setFontUnderline(true);
        format.setUnderlineColor(Qt::red);
        format.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);
        setFormat(0, text.size(), format);
    }
};
}

class MojikumiTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void bundledFontCompressesAdjacentPunctuation();
    void softWrappedVisualLineStartsGetHalt();
    void onlyOpeningPunctuationAtExactLineStartGetsHalt();
    void codeBlocksAreDecorated();
    void decorationDoesNotChangeTextOrUndoHistory();
    void existingLayoutFormatsArePreserved();
    void resizeAndRehighlightDoNotLeaveStaleMarkers();

private:
    QString m_fontFamily;
};

void MojikumiTest::initTestCase()
{
    const int fontId = QFontDatabase::addApplicationFont(
        QString::fromUtf8(MOJIKUMI_TEST_FONT_PATH));
    QVERIFY2(fontId >= 0, "The bundled private-name CJK OTF did not load");

    const QStringList families = QFontDatabase::applicationFontFamilies(fontId);
    const QString expectedFamily = AppSettings::BUNDLED_CJK_FONT_FAMILY;
    QVERIFY2(families.contains(expectedFamily),
             qPrintable(QStringLiteral("Private SC family missing from OTF; found: %1")
                            .arg(families.join(QStringLiteral(", ")))));
    m_fontFamily = expectedFamily;
}

void MojikumiTest::bundledFontCompressesAdjacentPunctuation()
{
    const QString adjacentSample = QStringLiteral("甲」。「乙");
    const QString isolatedSample = QStringLiteral("甲，乙。丙");
    QFont normal(m_fontFamily);
    normal.setPixelSize(48);
    normal.setFeature(ChwsTag, 0);

    QFont compressed = normal;
    compressed.setFeature(ChwsTag, 1);

    const qreal normalWidth = QFontMetricsF(normal).horizontalAdvance(adjacentSample);
    const qreal compressedWidth = QFontMetricsF(compressed).horizontalAdvance(adjacentSample);
    QVERIFY2(compressedWidth < normalWidth - 1.0,
             qPrintable(QStringLiteral("chws did not compress adjacent punctuation: %1 >= %2")
                            .arg(compressedWidth)
                            .arg(normalWidth)));

    const qreal isolatedNormalWidth =
        QFontMetricsF(normal).horizontalAdvance(isolatedSample);
    const qreal isolatedCompressedWidth =
        QFontMetricsF(compressed).horizontalAdvance(isolatedSample);
    QVERIFY2(qAbs(isolatedCompressedWidth - isolatedNormalWidth) < 0.01,
             "chws unexpectedly compressed isolated punctuation");

    QFont halted = normal;
    halted.setFeature(HaltTag, 1);
    QVERIFY2(QFontMetricsF(halted).horizontalAdvance(QStringLiteral("「正文"))
                 < QFontMetricsF(normal).horizontalAdvance(QStringLiteral("「正文")) - 1.0,
             "The bundled font's halt feature did not trim opening punctuation");
}

void MojikumiTest::softWrappedVisualLineStartsGetHalt()
{
    QPlainTextEdit editor;
    editor.setLineWrapMode(QPlainTextEdit::WidgetWidth);
    editor.setWordWrapMode(QTextOption::WrapAnywhere);
    editor.setFixedSize(220, 180);
    QFont font(m_fontFamily);
    font.setPixelSize(20);
    font.setFeature(ChwsTag, 1);
    editor.setFont(font);
    editor.setPlainText(QStringLiteral("甲乙丙丁戊己庚辛壬癸，「『正文』」。甲乙丙丁戊己庚辛壬癸"));
    const QVector<int> frozenStarts = baselineLineStarts(
        editor.toPlainText(), editor.font(), editor.size());
    showAndLayOut(editor);

    MojikumiDecorator decorator(&editor);
    decorator.scheduleAll();
    QTRY_VERIFY_WITH_TIMEOUT(!mojikumiRanges(editor.document()->begin()).isEmpty(), 1000);

    const QTextBlock block = editor.document()->begin();
    const QVector<QTextLayout::FormatRange> ranges = mojikumiRanges(block);
    QVERIFY2(frozenStarts.size() > 1, "Fixture did not produce a visual soft wrap");

    bool foundSoftWrappedOpening = false;
    for (const QTextLayout::FormatRange &range : ranges) {
        const auto features = range.format.fontFeatures();
        QCOMPARE(features.value(ChwsTag, 99), quint32(0));
        QCOMPARE(features.value(HaltTag, 0), quint32(1));
        if (range.start > 0 && frozenStarts.contains(range.start)
            && block.text().at(range.start) == QChar(0x300c)) {
            foundSoftWrappedOpening = true;
        }
    }
    QVERIFY2(foundSoftWrappedOpening,
             "Fixture did not put opening punctuation at a soft-wrapped line start");
}

void MojikumiTest::onlyOpeningPunctuationAtExactLineStartGetsHalt()
{
    auto rangesFor = [&](const QString &text) {
        QPlainTextEdit editor;
        editor.setFixedSize(300, 120);
        QFont font(m_fontFamily);
        font.setPixelSize(28);
        font.setFeature(ChwsTag, 1);
        editor.setFont(font);
        editor.setPlainText(text);
        showAndLayOut(editor);
        MojikumiDecorator decorator(&editor);
        decorator.scheduleAll();
        QTest::qWait(25);
        return mojikumiRanges(editor.document()->begin());
    };

    QVERIFY(!rangesFor(QStringLiteral("「正文")).isEmpty());
    QVERIFY(!rangesFor(QStringLiteral("‘正文")).isEmpty());
    QVERIFY(!rangesFor(QStringLiteral("“正文")).isEmpty());
    QVERIFY(rangesFor(QStringLiteral("」正文")).isEmpty());
    QVERIFY(rangesFor(QStringLiteral("，正文")).isEmpty());
    QVERIFY(rangesFor(QStringLiteral("  「正文")).isEmpty());
    QVERIFY(rangesFor(QString::fromUcs4(U"😀「正文")).isEmpty());

    // Unicode vertical presentation forms are Ps, but are not horizontal
    // fullwidth-opening characters and the bundled font has no `halt` glyph.
    QVERIFY(rangesFor(QStringLiteral("︵正文")).isEmpty());
}

void MojikumiTest::codeBlocksAreDecorated()
{
    QPlainTextEdit editor;
    editor.setFixedHeight(180);
    editor.setLineWrapMode(QPlainTextEdit::WidgetWidth);
    editor.setWordWrapMode(QTextOption::WrapAnywhere);
    QFont font(m_fontFamily);
    font.setPixelSize(26);
    font.setFeature(ChwsTag, 1);
    editor.setFont(font);
    editor.setPlainText(QStringLiteral("```text\n代码块甲乙丙丁戊己庚辛壬癸，「『正文』」。\n```"));

    QTextBlock codeBlock = editor.document()->findBlockByNumber(1);
    QVERIFY(codeBlock.isValid());
    codeBlock.setUserState(MarkdownStateCodeBlock);
    const int opening = codeBlock.text().indexOf(QChar(0x300c));
    QVERIFY(opening > 0);

    // Select a width that puts the opening punctuation at a soft-wrapped
    // visual line start. A fixed widget width is not portable: frame and
    // scrollbar metrics vary between Qt platform themes.
    editor.show();
    int fixtureWidth = 0;
    for (int width = 64; width <= 400; ++width) {
        editor.setFixedWidth(width);
        QApplication::processEvents();
        codeBlock = editor.document()->findBlockByNumber(1);
        editor.document()->documentLayout()->blockBoundingRect(codeBlock);
        if (visualLineStarts(codeBlock).contains(opening)) {
            fixtureWidth = width;
            break;
        }
    }
    QVERIFY2(fixtureWidth > 0,
             "Could not construct a soft-wrapped code-block line start");

    MojikumiDecorator decorator(&editor);
    decorator.scheduleAll();
    codeBlock = editor.document()->findBlockByNumber(1);
    QTRY_VERIFY_WITH_TIMEOUT(([&]() {
        const auto ranges = mojikumiRanges(codeBlock);
        return std::any_of(
            ranges.cbegin(),
            ranges.cend(),
            [opening](const QTextLayout::FormatRange &range) {
                return range.start == opening;
            });
    })(), 1000);
    QVERIFY(codeBlock.userState() == MarkdownStateCodeBlock);
}

void MojikumiTest::decorationDoesNotChangeTextOrUndoHistory()
{
    QPlainTextEdit editor;
    editor.setPlainText(QStringLiteral("「甲乙丙丁戊己庚辛壬癸，「『正文』」。"));
    editor.document()->clearUndoRedoStacks();
    const QString before = editor.toPlainText();
    const int undoStepsBefore = editor.document()->availableUndoSteps();
    QSignalSpy undoSpy(editor.document(), &QTextDocument::undoCommandAdded);
    showAndLayOut(editor);

    MojikumiDecorator decorator(&editor);
    decorator.scheduleAll();
    QTRY_VERIFY_WITH_TIMEOUT(!mojikumiRanges(editor.document()->begin()).isEmpty(), 1000);

    QCOMPARE(editor.toPlainText(), before);
    QCOMPARE(editor.document()->availableUndoSteps(), undoStepsBefore);
    QCOMPARE(undoSpy.count(), 0);
}

void MojikumiTest::existingLayoutFormatsArePreserved()
{
    QPlainTextEdit editor;
    editor.setPlainText(QStringLiteral("「甲乙丙丁戊己庚辛壬癸，「『正文』」。"));
    UnderlineHighlighter highlighter(editor.document());
    highlighter.rehighlight();
    showAndLayOut(editor);

    const QTextBlock block = editor.document()->begin();
    const QVector<QTextLayout::FormatRange> originalFormats =
        block.layout()->formats();
    QCOMPARE(originalFormats.size(), 1);
    QCOMPARE(originalFormats.constFirst().format.underlineStyle(),
             QTextCharFormat::SpellCheckUnderline);
    QCOMPARE(originalFormats.constFirst().format.underlineColor(),
             QColor(Qt::red));
    QCOMPARE(originalFormats.constFirst().format.foreground().color(),
             QColor(Qt::darkGreen));

    MojikumiDecorator decorator(&editor);
    decorator.scheduleAll();
    QTRY_VERIFY_WITH_TIMEOUT(!mojikumiRanges(editor.document()->begin()).isEmpty(), 1000);

    QVector<QTextLayout::FormatRange> retainedFormats;
    for (const QTextLayout::FormatRange &range : block.layout()->formats()) {
        if (!MojikumiDecorator::isMojikumiFormat(range.format)) {
            retainedFormats.append(range);
        }
    }
    QCOMPARE(retainedFormats, originalFormats);
}

void MojikumiTest::resizeAndRehighlightDoNotLeaveStaleMarkers()
{
    QPlainTextEdit editor;
    editor.setWordWrapMode(QTextOption::WrapAnywhere);
    editor.setFixedSize(300, 180);
    QFont font(m_fontFamily);
    font.setPixelSize(26);
    font.setFeature(ChwsTag, 1);
    editor.setFont(font);
    editor.setPlainText(QStringLiteral("甲乙丙丁戊己庚辛壬癸，「『正文』」。甲乙丙丁戊己庚辛壬癸"));
    UnderlineHighlighter highlighter(editor.document());
    showAndLayOut(editor);

    MojikumiDecorator decorator(&editor);
    decorator.scheduleAll();
    QTRY_VERIFY_WITH_TIMEOUT(!mojikumiRanges(editor.document()->begin()).isEmpty(), 1000);

    QSignalSpy layoutUpdateSpy(
        editor.document()->documentLayout(),
        &QAbstractTextDocumentLayout::update);

    editor.resize(460, 180);
    const QVector<int> baselineStarts = baselineLineStarts(
        editor.toPlainText(), editor.font(), editor.size());
    QVector<int> expectedFrozenStarts;
    for (int start : baselineStarts) {
        if (start < editor.toPlainText().size()
            && editor.toPlainText().at(start) == QChar(0x300c)) {
            expectedFrozenStarts.append(start);
        }
    }

    QTRY_COMPARE_WITH_TIMEOUT(
        mojikumiRanges(editor.document()->begin()).size(),
        expectedFrozenStarts.size(),
        1000);

    auto verifyCurrentMarkers = [&]() {
        QSet<int> markedStarts;
        for (const QTextLayout::FormatRange &range :
             mojikumiRanges(editor.document()->begin())) {
            QVERIFY2(expectedFrozenStarts.contains(range.start),
                     "A stale mojikumi marker survived reflow");
            QVERIFY2(!markedStarts.contains(range.start),
                     "Duplicate mojikumi marker was installed");
            markedStarts.insert(range.start);
        }
        QCOMPARE(markedStarts.size(), expectedFrozenStarts.size());
    };
    verifyCurrentMarkers();

    highlighter.rehighlight();
    decorator.scheduleAll();
    QTRY_COMPARE_WITH_TIMEOUT(
        mojikumiRanges(editor.document()->begin()).size(),
        expectedFrozenStarts.size(),
        1000);
    verifyCurrentMarkers();

    const QVector<QTextLayout::FormatRange> stableRanges =
        mojikumiRanges(editor.document()->begin());
    const int stableUpdateCount = layoutUpdateSpy.count();
    QTest::qWait(100);
    QCOMPARE(mojikumiRanges(editor.document()->begin()), stableRanges);
    QCOMPARE(layoutUpdateSpy.count(), stableUpdateCount);
}

QTEST_MAIN(MojikumiTest)

#include "mojikumitest.moc"
