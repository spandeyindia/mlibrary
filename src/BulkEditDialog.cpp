#include "BulkEditDialog.h"
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

BulkEditDialog::BulkEditDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Bulk Edit Metadata");
    resize(680, 420);

    auto *layout = new QVBoxLayout(this);
    auto *form = new QFormLayout();

    subjectEdit_ = new QLineEdit(this);
    publisherEdit_ = new QLineEdit(this);
    remarksEdit_ = new QLineEdit(this);
    tagsEdit_ = new QLineEdit(this);
    favoriteCheck_ = new QCheckBox("Mark selected books as favorite", this);

    form->addRow("Subject:", subjectEdit_);
    form->addRow("Publisher:", publisherEdit_);
    form->addRow("Remarks:", remarksEdit_);
    form->addRow("Tags (comma separated):", tagsEdit_);
    layout->addLayout(form);
    layout->addWidget(favoriteCheck_);

    log_ = new QTextEdit(this);
    log_->setReadOnly(true);
    layout->addWidget(log_);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    auto *applyBtn = new QPushButton("Apply", this);
    buttons->addButton(applyBtn, QDialogButtonBox::ActionRole);
    connect(applyBtn, &QPushButton::clicked, this, &BulkEditDialog::apply);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
}

void BulkEditDialog::apply() {
    log_->append("Bulk edit changes would be applied to selected books.");
    if (!subjectEdit_->text().trimmed().isEmpty()) log_->append("Subject update queued.");
    if (!publisherEdit_->text().trimmed().isEmpty()) log_->append("Publisher update queued.");
    if (!remarksEdit_->text().trimmed().isEmpty()) log_->append("Remarks update queued.");
    if (!tagsEdit_->text().trimmed().isEmpty()) log_->append("Tags update queued.");
    if (favoriteCheck_->isChecked()) log_->append("Favorite flag update queued.");
}
