/*
 * SPDX-FileCopyrightText: 2017-2023 Megan Conkle <megan.conkle@kdemail.net>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QComboBox>
#include <QPushButton>
#include <QFileInfo>
#include <QLineEdit>

#include "../export/exporterfactory.h"

#include "appsettings.h"
#include "previewoptionsdialog.h"
#include "simplefontdialog.h"

namespace ghostwriter
{
class PreviewOptionsDialogPrivate
{
public:
    PreviewOptionsDialogPrivate()
    {
        ;
    }

    ~PreviewOptionsDialogPrivate()
    {
        ;
    }

    void onExporterChanged(int index) const;
    QString fontToString(const QFont &font) const;

    AppSettings *appSettings;
    ExporterFactory *exporterFactory;
    QComboBox *previewerComboBox;
    QLineEdit *paramsLineEdit;
};

PreviewOptionsDialog::PreviewOptionsDialog(QWidget *parent)
    : QDialog(parent),
      d_ptr(new PreviewOptionsDialogPrivate())
{
    Q_D(PreviewOptionsDialog);

    this->setWindowTitle(tr("Preview Options"));
    d->appSettings = AppSettings::instance();
    d->exporterFactory = ExporterFactory::instance();

    QWidget *mainContents = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout();
    this->setLayout(layout);
    layout->addWidget(mainContents);

    QFormLayout *optionsLayout = new QFormLayout();
    mainContents->setLayout(optionsLayout);

    d->previewerComboBox = new QComboBox(this);

    QList<Exporter *> exporters = d->exporterFactory->htmlExporters();
    Exporter *currentExporter = d->appSettings->currentHtmlExporter();

    int currentExporterIndex = 0;

    for (int i = 0; i < exporters.length(); i++) {
        Exporter *exporter = exporters.at(i);
        d->previewerComboBox->addItem(exporter->name(), QVariant::fromValue((void *) exporter));

        if (exporter == currentExporter) {
            currentExporterIndex = i;
        }
    }

    d->previewerComboBox->setCurrentIndex(currentExporterIndex);

    connect
    (
        d->previewerComboBox,
        static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        [d](int index) {
            d->onExporterChanged(index);
            d->paramsLineEdit->setDisabled(!d->appSettings->currentHtmlExporter()->supportsUserOptions());
        }
    );

    optionsLayout->addRow(tr("Markdown Flavor"), d->previewerComboBox);

    QHBoxLayout *fontLayout = new QHBoxLayout();

    QLineEdit *currentFont = new QLineEdit(d->fontToString(d->appSettings->editorFont()));
    currentFont->setReadOnly(true);
    fontLayout->addWidget(currentFont);

    QPushButton *chooseButton = new QPushButton(tr("Choose..."));
    fontLayout->addWidget(chooseButton);

    connect(chooseButton,
        &QPushButton::clicked,
        [this, d, currentFont]() {
            bool success = false;

            QFont font = SimpleFontDialog::font(&success,
                d->appSettings->editorFont(),
                this);

            if (success) {
                currentFont->setText(d->fontToString(font));
                d->appSettings->setUnifiedFont(font);
            }
        });

    optionsLayout->addRow(tr("Editor and Preview Font:"), fontLayout);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Horizontal, this);
    buttonBox->addButton(QDialogButtonBox::Close);
    layout->addWidget(buttonBox);

    d->paramsLineEdit = new QLineEdit();
    d->paramsLineEdit->setText(d->appSettings->currentHtmlExporter()->options());
    optionsLayout->addRow(tr("Command line options:"), d->paramsLineEdit);
    connect(d->paramsLineEdit, &QLineEdit::textChanged, [=](const QString& obj) {
        Exporter *exporter = d->appSettings->currentHtmlExporter();
        exporter->setOptions(obj);
        d->appSettings->setCurrentHtmlExporter(exporter);
    });
    d->paramsLineEdit->setDisabled(!d->appSettings->currentHtmlExporter()->supportsUserOptions());

    connect(buttonBox->button(QDialogButtonBox::Close), SIGNAL(clicked()), this, SLOT(close()));
}

PreviewOptionsDialog::~PreviewOptionsDialog()
{
    ;
}

void PreviewOptionsDialogPrivate::onExporterChanged(int index) const
{
    QVariant exporterVariant = this->previewerComboBox->itemData(index);
    Exporter *exporter = (Exporter *) exporterVariant.value<void *>();
    exporter->setOptions(this->appSettings->currentHtmlExporter()->options());
    appSettings->setCurrentHtmlExporter(exporter);
}

QString PreviewOptionsDialogPrivate::fontToString(const QFont &font) const
{
    return PreviewOptionsDialog::tr("%1 %2pt")
        .arg(font.family())
        .arg(font.pointSize());
}

} // namespace ghostwriter
