/*
 * SPDX-FileCopyrightText: 2026 Mr-Drinking
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "mojikumidecorator.h"

#include <algorithm>

#include <QAbstractTextDocumentLayout>
#include <QChar>
#include <QEvent>
#include <QPlainTextEdit>
#include <QPointer>
#include <QSet>
#include <QTextBlock>
#include <QTextDocument>
#include <QTextLayout>
#include <QTimer>

namespace ghostwriter
{
namespace
{
int codePointLengthAt(const QString &text, int position)
{
    if (position < 0 || position >= text.size()) {
        return 0;
    }

    return text.at(position).isHighSurrogate()
            && position + 1 < text.size()
            && text.at(position + 1).isLowSurrogate()
        ? 2
        : 1;
}

bool isFullwidthOpeningPunctuation(const QString &text, int position)
{
    if (position < 0 || position >= text.size()) {
        return false;
    }

    uint codePoint = text.at(position).unicode();
    if (text.at(position).isHighSurrogate()
        && position + 1 < text.size()
        && text.at(position + 1).isLowSurrogate()) {
        codePoint = QChar::surrogateToUcs4(
            text.at(position), text.at(position + 1));
    }

    // Match CSS Text 4's fullwidth-opening class: U+2018/U+201C, opening
    // punctuation in CJK Symbols and Punctuation, or with East Asian
    // Width=Fullwidth.
    // Qt exposes general category but not East Asian Width, so the latter is
    // the four fullwidth open brackets below. Vertical/small presentation
    // forms are intentionally not included: in horizontal text they are wide,
    // not Fullwidth, and the bundled font has no horizontal `halt` alternate.
    if (codePoint == 0x2018 || codePoint == 0x201c) {
        return true;
    }

    if (QChar::category(codePoint) != QChar::Punctuation_Open) {
        return false;
    }

    return (codePoint >= 0x3008 && codePoint <= 0x301d)
        || codePoint == 0xff08
        || codePoint == 0xff3b
        || codePoint == 0xff5b
        || codePoint == 0xff5f;
}
}

class MojikumiDecoratorPrivate
{
    Q_DECLARE_PUBLIC(MojikumiDecorator)

public:
    MojikumiDecoratorPrivate(MojikumiDecorator *q, QPlainTextEdit *textEditor)
        : q_ptr(q), editor(textEditor)
    {
    }

    void queueApply();
    bool decorateBlock(const QTextBlock &block);

    MojikumiDecorator *const q_ptr;
    QPointer<QPlainTextEdit> editor;
    QSet<int> pendingBlockPositions;
    bool applyQueued = false;
    bool applying = false;
};

MojikumiDecorator::MojikumiDecorator(QPlainTextEdit *editor)
    : QObject(editor), d_ptr(new MojikumiDecoratorPrivate(this, editor))
{
    Q_ASSERT(editor);

    editor->installEventFilter(this);
    editor->viewport()->installEventFilter(this);

    connect(
        editor->document(),
        &QTextDocument::contentsChange,
        this,
        &MojikumiDecorator::onContentsChange);

    scheduleAll();
}

MojikumiDecorator::~MojikumiDecorator() = default;

bool MojikumiDecorator::isMojikumiFormat(const QTextCharFormat &format)
{
    return format.property(FormatMarkerProperty).toBool();
}

void MojikumiDecoratorPrivate::queueApply()
{
    Q_Q(MojikumiDecorator);

    if (applyQueued) {
        return;
    }

    applyQueued = true;
    QTimer::singleShot(0, q, &MojikumiDecorator::applyPendingFormats);
}

void MojikumiDecorator::scheduleAll()
{
    Q_D(MojikumiDecorator);

    if (!d->editor) {
        return;
    }

    for (QTextBlock block = d->editor->document()->begin();
         block.isValid();
         block = block.next()) {
        d->pendingBlockPositions.insert(block.position());
    }

    d->queueApply();
}

void MojikumiDecorator::scheduleBlock(int position)
{
    Q_D(MojikumiDecorator);

    if (!d->editor) {
        return;
    }

    const QTextBlock block = d->editor->document()->findBlock(position);
    if (!block.isValid()) {
        return;
    }

    d->pendingBlockPositions.insert(block.position());
    d->queueApply();
}

bool MojikumiDecorator::eventFilter(QObject *watched, QEvent *event)
{
    Q_D(MojikumiDecorator);

    if (d->editor
        && ((watched == d->editor->viewport() && event->type() == QEvent::Resize)
            || (watched == d->editor && event->type() == QEvent::FontChange))) {
        scheduleAll();
    }

    return QObject::eventFilter(watched, event);
}

void MojikumiDecorator::onContentsChange(
    int position,
    int charsRemoved,
    int charsAdded)
{
    Q_UNUSED(charsRemoved)

    Q_D(MojikumiDecorator);

    if (!d->editor) {
        return;
    }

    QTextDocument *document = d->editor->document();
    QTextBlock block = document->findBlock(std::max(0, position));
    const int endPosition = std::min(
        document->characterCount() - 1,
        position + std::max(1, charsAdded));
    const QTextBlock lastBlock = document->findBlock(endPosition);

    while (block.isValid()) {
        d->pendingBlockPositions.insert(block.position());

        if (!lastBlock.isValid() || block == lastBlock) {
            break;
        }
        block = block.next();
    }

    if (lastBlock.isValid() && lastBlock.next().isValid()) {
        d->pendingBlockPositions.insert(lastBlock.next().position());
    }

    d->queueApply();
}

bool MojikumiDecoratorPrivate::decorateBlock(const QTextBlock &block)
{
    if (!editor || !block.isValid() || !block.layout()) {
        return false;
    }

    QTextLayout *layout = block.layout();
    if (layout->lineCount() == 0 && editor->document()->documentLayout()) {
        editor->document()->documentLayout()->blockBoundingRect(block);
    }

    if (layout->lineCount() == 0) {
        return false;
    }

    QVector<QTextLayout::FormatRange> retainedFormats;
    bool hadMojikumiFormats = false;

    for (const QTextLayout::FormatRange &range : layout->formats()) {
        if (MojikumiDecorator::isMojikumiFormat(range.format)) {
            hadMojikumiFormats = true;
        } else {
            retainedFormats.append(range);
        }
    }

    // Freeze the breaks produced by the base `chws` layout. Applying `halt`
    // can itself change wrapping, and feeding that result back into another
    // pass can oscillate at width boundaries. Always remove our old ranges,
    // lay out once with `chws`, then decorate those frozen line starts once.
    if (hadMojikumiFormats) {
        layout->setFormats(retainedFormats);
        editor->document()->markContentsDirty(block.position(), block.length());
        editor->document()->documentLayout()->blockBoundingRect(block);
    }

    QVector<QTextLayout::FormatRange> mojikumiFormats;
    const QString text = block.text();

    for (int lineIndex = 0; lineIndex < layout->lineCount(); ++lineIndex) {
        const QTextLine line = layout->lineAt(lineIndex);
        const int start = line.textStart();
        const int length = codePointLengthAt(text, start);

        if (length == 0 || !isFullwidthOpeningPunctuation(text, start)) {
            continue;
        }

        QTextCharFormat format;
        format.setProperty(MojikumiDecorator::FormatMarkerProperty, true);
        format.setFontFeatures({{QFont::Tag("chws"), 0},
                                {QFont::Tag("halt"), 1}});

        QTextLayout::FormatRange range;
        range.start = start;
        range.length = length;
        range.format = format;
        mojikumiFormats.append(range);
    }

    retainedFormats.append(mojikumiFormats);
    layout->setFormats(retainedFormats);
    editor->document()->markContentsDirty(block.position(), block.length());
    return !mojikumiFormats.isEmpty() || hadMojikumiFormats;
}

void MojikumiDecorator::applyPendingFormats()
{
    Q_D(MojikumiDecorator);

    d->applyQueued = false;
    if (!d->editor || d->applying) {
        return;
    }

    d->applying = true;
    const QSet<int> positions = d->pendingBlockPositions;
    d->pendingBlockPositions.clear();

    for (int position : positions) {
        const QTextBlock block = d->editor->document()->findBlock(position);
        if (!block.isValid()) {
            continue;
        }

        d->decorateBlock(block);
    }

    d->applying = false;

    // A request can arrive while setFormats()/markContentsDirty() is
    // notifying other presentation decorators. Do not strand that work just
    // because this pass was guarded against re-entry.
    if (!d->pendingBlockPositions.isEmpty()) {
        d->queueApply();
    }
}
} // namespace ghostwriter
