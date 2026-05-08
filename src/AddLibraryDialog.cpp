#include "AddLibraryDialog.h"
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

AddLibraryDialog::AddLibraryDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Add Library");
    auto *layout = new QVBoxLayout(this);
    auto *form = new QFormLayout();
    nameEdit_ = new QLineEdit(this);
    rootEdit_ = new QLineEdit(this);
    form->addRow("Library name:", nameEdit_);
    form->addRow("Root path:", rootEdit_);
    layout->addLayout(form);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
}
QString AddLibraryDialog::libraryName() const { return nameEdit_->text().trimmed(); }
QString AddLibraryDialog::rootPath() const { return rootEdit_->text().trimmed(); }
