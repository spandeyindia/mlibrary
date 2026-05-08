#include "CopyMoveDialog.h"
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

CopyMoveDialog::CopyMoveDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Copy / Move Books");
    resize(640, 360);

    auto *layout = new QVBoxLayout(this);
    auto *form = new QFormLayout();

    actionCombo_ = new QComboBox(this);
    actionCombo_->addItems({"Copy to another library", "Move to another library"});
    libraryCombo_ = new QComboBox(this);
    libraryCombo_->addItems({"Main Library", "Research Library", "Comics Library"});

    form->addRow("Action:", actionCombo_);
    form->addRow("Target library:", libraryCombo_);
    layout->addLayout(form);

    log_ = new QTextEdit(this);
    log_->setReadOnly(true);
    layout->addWidget(log_);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    auto *runBtn = new QPushButton("Execute", this);
    buttons->addButton(runBtn, QDialogButtonBox::ActionRole);
    connect(runBtn, &QPushButton::clicked, this, &CopyMoveDialog::runAction);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
}

void CopyMoveDialog::runAction() {
    log_->append(QString("%1 selected for %2.")
                 .arg(actionCombo_->currentText(), libraryCombo_->currentText()));
    log_->append("In the full project, selected books would be duplicated or moved across library roots, while keeping metadata and file references consistent.");
}
