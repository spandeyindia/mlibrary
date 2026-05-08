#include "SubjectEditDialog.h"
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

SubjectEditDialog::SubjectEditDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Modify Subject");
    resize(560, 280);

    auto *layout = new QVBoxLayout(this);
    auto *form = new QFormLayout();

    subjectEdit_ = new QLineEdit(this);
    form->addRow("New subject:", subjectEdit_);
    layout->addLayout(form);

    log_ = new QTextEdit(this);
    log_->setReadOnly(true);
    layout->addWidget(log_);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    auto *applyBtn = new QPushButton("Apply", this);
    buttons->addButton(applyBtn, QDialogButtonBox::ActionRole);
    connect(applyBtn, &QPushButton::clicked, this, &SubjectEditDialog::apply);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
}

void SubjectEditDialog::apply() {
    log_->append(QString("Subject will be updated to: %1").arg(subjectEdit_->text().trimmed()));
}
