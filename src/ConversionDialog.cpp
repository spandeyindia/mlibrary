#include "ConversionDialog.h"
#include "ConversionService.h"
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

ConversionDialog::ConversionDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Convert File");
    resize(760, 520);

    auto *layout = new QVBoxLayout(this);
    auto *form = new QFormLayout();

    sourceEdit_ = new QLineEdit(this);
    auto *srcBtn = new QPushButton("Browse", this);
    connect(srcBtn, &QPushButton::clicked, this, &ConversionDialog::browseSource);
    auto *srcRow = new QWidget(this);
    {
        auto *h = new QHBoxLayout(srcRow);
        h->setContentsMargins(0,0,0,0);
        h->addWidget(sourceEdit_);
        h->addWidget(srcBtn);
    }

    outputEdit_ = new QLineEdit(this);
    auto *outBtn = new QPushButton("Browse", this);
    connect(outBtn, &QPushButton::clicked, this, &ConversionDialog::browseOutput);
    auto *outRow = new QWidget(this);
    {
        auto *h = new QHBoxLayout(outRow);
        h->setContentsMargins(0,0,0,0);
        h->addWidget(outputEdit_);
        h->addWidget(outBtn);
    }

    ebookConvertEdit_ = new QLineEdit(this);
    ebookConvertEdit_->setText(ConversionService::defaultEbookConvertPath());
    targetFormat_ = new QComboBox(this);
    targetFormat_->addItems({"pdf", "epub", "mobi", "txt", "html"});

    form->addRow("Source file:", srcRow);
    form->addRow("Target format:", targetFormat_);
    form->addRow("Output file:", outRow);
    form->addRow("ebook-convert path:", ebookConvertEdit_);
    layout->addLayout(form);

    log_ = new QTextEdit(this);
    log_->setReadOnly(true);
    layout->addWidget(log_);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    auto *convertBtn = new QPushButton("Convert", this);
    buttons->addButton(convertBtn, QDialogButtonBox::ActionRole);
    connect(convertBtn, &QPushButton::clicked, this, &ConversionDialog::runConversion);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
}

void ConversionDialog::browseSource() { const auto p = QFileDialog::getOpenFileName(this, "Select source file"); if (!p.isEmpty()) sourceEdit_->setText(p); }
void ConversionDialog::browseOutput() { const auto p = QFileDialog::getSaveFileName(this, "Select output file"); if (!p.isEmpty()) outputEdit_->setText(p); }

void ConversionDialog::runConversion() {
    QString log; QString error;
    if (!ConversionService::convertFile(ebookConvertEdit_->text(), sourceEdit_->text(), outputEdit_->text(), &log, &error)) {
        log_->append("Conversion failed:");
        log_->append(error);
        return;
    }
    log_->append("Conversion completed.");
    log_->append(log);
}
