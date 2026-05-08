#include "WebsiteImportDialog.h"
#include "ConversionService.h"
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QTextEdit>
#include <QVBoxLayout>

WebsiteImportDialog::WebsiteImportDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Import Website Recursively");
    resize(820, 560);
    auto *layout = new QVBoxLayout(this);
    auto *form = new QFormLayout();

    urlEdit_ = new QLineEdit(this);
    mirrorEdit_ = new QLineEdit(this);
    outputPdfEdit_ = new QLineEdit(this);
    wgetEdit_ = new QLineEdit(this);
    wgetEdit_->setText(ConversionService::defaultWgetPath());
    ebookConvertEdit_ = new QLineEdit(this);
    ebookConvertEdit_->setText(ConversionService::defaultEbookConvertPath());
    depthSpin_ = new QSpinBox(this);
    depthSpin_->setMinimum(0);
    depthSpin_->setMaximum(20);
    depthSpin_->setValue(3);

    auto *mirrorBtn = new QPushButton("Browse", this);
    connect(mirrorBtn, &QPushButton::clicked, this, &WebsiteImportDialog::browseMirror);
    auto *mirrorRow = new QWidget(this);
    {
        auto *h = new QHBoxLayout(mirrorRow);
        h->setContentsMargins(0,0,0,0);
        h->addWidget(mirrorEdit_);
        h->addWidget(mirrorBtn);
    }

    auto *pdfBtn = new QPushButton("Browse", this);
    connect(pdfBtn, &QPushButton::clicked, this, &WebsiteImportDialog::browseOutputPdf);
    auto *pdfRow = new QWidget(this);
    {
        auto *h = new QHBoxLayout(pdfRow);
        h->setContentsMargins(0,0,0,0);
        h->addWidget(outputPdfEdit_);
        h->addWidget(pdfBtn);
    }

    form->addRow("Website URL:", urlEdit_);
    form->addRow("Mirror directory:", mirrorRow);
    form->addRow("Output PDF file:", pdfRow);
    form->addRow("Recursive depth:", depthSpin_);
    form->addRow("wget path:", wgetEdit_);
    form->addRow("ebook-convert path:", ebookConvertEdit_);
    layout->addLayout(form);

    log_ = new QTextEdit(this);
    log_->setReadOnly(true);
    layout->addWidget(log_);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    auto *runBtn = new QPushButton("Mirror and Convert", this);
    buttons->addButton(runBtn, QDialogButtonBox::ActionRole);
    connect(runBtn, &QPushButton::clicked, this, &WebsiteImportDialog::runMirror);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
}
void WebsiteImportDialog::browseMirror() { const auto d = QFileDialog::getExistingDirectory(this, "Select mirror directory"); if (!d.isEmpty()) mirrorEdit_->setText(d); }
void WebsiteImportDialog::browseOutputPdf() { const auto p = QFileDialog::getSaveFileName(this, "Select PDF output", QString(), "PDF files (*.pdf)"); if (!p.isEmpty()) outputPdfEdit_->setText(p); }
void WebsiteImportDialog::runMirror() {
    QString log; QString error;
    if (!ConversionService::mirrorWebsiteToPdf(wgetEdit_->text(), ebookConvertEdit_->text(), urlEdit_->text(), mirrorEdit_->text(), outputPdfEdit_->text(), depthSpin_->value(), &log, &error)) {
        log_->append("Website import failed:");
        log_->append(error);
        return;
    }
    log_->append("Website mirrored and converted to PDF.");
    log_->append(log);
}
