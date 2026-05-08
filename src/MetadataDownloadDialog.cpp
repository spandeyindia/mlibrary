#include "MetadataDownloadDialog.h"
#include "MetadataService.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPixmap>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

MetadataDownloadDialog::MetadataDownloadDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Download Metadata / Cover");
    resize(820, 560);

    auto *layout = new QVBoxLayout(this);
    auto *form = new QFormLayout();

    isbnEdit_ = new QLineEdit(this);
    titleEdit_ = new QLineEdit(this);
    authorsEdit_ = new QLineEdit(this);
    publisherEdit_ = new QLineEdit(this);
    publishedEdit_ = new QLineEdit(this);
    coverPathEdit_ = new QLineEdit(this);

    form->addRow("ISBN:", isbnEdit_);
    form->addRow("Title:", titleEdit_);
    form->addRow("Authors:", authorsEdit_);
    form->addRow("Publisher:", publisherEdit_);
    form->addRow("Published date:", publishedEdit_);
    form->addRow("Cover path:", coverPathEdit_);
    layout->addLayout(form);

    auto *previewRow = new QHBoxLayout();
    coverPreview_ = new QLabel(this);
    coverPreview_->setMinimumSize(160, 220);
    coverPreview_->setFrameShape(QFrame::Box);
    coverPreview_->setAlignment(Qt::AlignCenter);
    coverPreview_->setText("Cover Preview");
    previewRow->addWidget(coverPreview_, 0, Qt::AlignLeft);
    previewRow->addStretch();
    layout->addLayout(previewRow);

    log_ = new QTextEdit(this);
    log_->setReadOnly(true);
    layout->addWidget(log_);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    auto *lookupBtn = new QPushButton("Lookup", this);
    buttons->addButton(lookupBtn, QDialogButtonBox::ActionRole);
    connect(lookupBtn, &QPushButton::clicked, this, &MetadataDownloadDialog::runLookup);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);

    auto *service = new MetadataService(this);
    connect(service, &MetadataService::lookupFinished, this,
            [this](const QJsonObject &metadata, const QPixmap &cover, const QString &log, const QString &error) {
                if (!log.isEmpty()) log_->append(log);
                if (!error.isEmpty()) log_->append(error);

                if (!metadata.isEmpty()) {
                    titleEdit_->setText(metadata.value("title").toString());
                    authorsEdit_->setText(metadata.value("authors").toString());
                    publisherEdit_->setText(metadata.value("publisher").toString());
                    publishedEdit_->setText(metadata.value("published_date").toString());
                    log_->append("Metadata fields filled from internet source.");
                }
                if (!cover.isNull()) {
                    coverPreview_->setPixmap(cover.scaled(160, 220, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                    log_->append("Cover downloaded.");
                }
            });
    connect(this, &MetadataDownloadDialog::destroyed, service, &QObject::deleteLater);
    connect(lookupBtn, &QPushButton::clicked, service, [service, this]() { service->lookupByIsbn(isbnEdit_->text()); });
}

void MetadataDownloadDialog::runLookup() {
    // Handled by service signal connections.
}
