/*
 * SPDX-FileCopyrightText: 2026 Mr-Drinking
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef MOJIKUMI_DECORATOR_H
#define MOJIKUMI_DECORATOR_H

#include <QObject>
#include <QScopedPointer>
#include <QTextCharFormat>
#include <QTextFormat>

class QPlainTextEdit;

namespace ghostwriter
{
class MojikumiDecoratorPrivate;

/**
 * Adds the line-start half-width OpenType feature to fullwidth opening
 * punctuation at visual line starts.
 *
 * The font decides which glyphs are affected by `halt`; this class does not
 * carry a punctuation table. Contextual punctuation compression is handled
 * independently by the font's `chws` feature.
 */
class MojikumiDecorator : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(MojikumiDecorator)

public:
    static constexpr int FormatMarkerProperty = QTextFormat::UserProperty + 77;

    explicit MojikumiDecorator(QPlainTextEdit *editor);
    ~MojikumiDecorator() override;

    static bool isMojikumiFormat(const QTextCharFormat &format);

public slots:
    void scheduleAll();
    void scheduleBlock(int position);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void applyPendingFormats();
    void onContentsChange(int position, int charsRemoved, int charsAdded);

private:
    QScopedPointer<MojikumiDecoratorPrivate> d_ptr;
};
} // namespace ghostwriter

#endif
