#include "PhysicalCopyDialog.h"

#include <QComboBox>
#include <QDateEdit>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

PhysicalCopyDialog::PhysicalCopyDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Physical Copy Catalog");
    resize(760, 520);

    auto *layout = new QVBoxLayout(this);
    auto *form = new QFormLayout();

    locationEdit_ = new QLineEdit(this);
    sectionEdit_ = new QLineEdit(this);
    shelfEdit_ = new QLineEdit(this);
    statusCombo_ = new QComboBox(this);
    statusCombo_->addItems({"On shelf", "Checked out", "Reserved", "In transit"});
    checkedOutToEdit_ = new QLineEdit(this);
    checkedOutContactEdit_ = new QLineEdit(this);
    checkedOutOnEdit_ = new QDateEdit(this);
    checkedOutOnEdit_->setCalendarPopup(true);
    checkedOutOnEdit_->setDisplayFormat("yyyy-MM-dd");
    checkedOutOnEdit_->setSpecialValueText("Not set");
    checkedOutOnEdit_->setDate(QDate());
    dueDateEdit_ = new QDateEdit(this);
    dueDateEdit_->setCalendarPopup(true);
    dueDateEdit_->setDisplayFormat("yyyy-MM-dd");
    dueDateEdit_->setSpecialValueText("Not set");
    dueDateEdit_->setDate(QDate());
    accessionEdit_ = new QLineEdit(this);
    barcodeEdit_ = new QLineEdit(this);
    notesEdit_ = new QTextEdit(this);

    form->addRow("Physical location:", locationEdit_);
    form->addRow("Section / room:", sectionEdit_);
    form->addRow("Shelf / rack:", shelfEdit_);
    form->addRow("Status:", statusCombo_);
    form->addRow("Checked out to:", checkedOutToEdit_);
    form->addRow("Borrower contact:", checkedOutContactEdit_);
    form->addRow("Checked out on:", checkedOutOnEdit_);
    form->addRow("Due date:", dueDateEdit_);
    form->addRow("Accession number:", accessionEdit_);
    form->addRow("Barcode:", barcodeEdit_);
    form->addRow("Loan notes:", notesEdit_);
    layout->addLayout(form);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
}

void PhysicalCopyDialog::setBook(const Book &book) {
    locationEdit_->setText(book.physicalLocation);
    sectionEdit_->setText(book.physicalSection);
    shelfEdit_->setText(book.physicalShelf);
    const int idx = statusCombo_->findText(book.physicalStatus.isEmpty() ? "On shelf" : book.physicalStatus);
    if (idx >= 0) statusCombo_->setCurrentIndex(idx);
    checkedOutToEdit_->setText(book.checkedOutTo);
    checkedOutContactEdit_->setText(book.checkedOutContact);
    if (book.checkedOutOn.isValid()) checkedOutOnEdit_->setDate(book.checkedOutOn); else checkedOutOnEdit_->setDate(QDate());
    if (book.dueDate.isValid()) dueDateEdit_->setDate(book.dueDate); else dueDateEdit_->setDate(QDate());
    accessionEdit_->setText(book.accessionNumber);
    barcodeEdit_->setText(book.barcode);
    notesEdit_->setPlainText(book.loanNotes);
}

void PhysicalCopyDialog::applyToBook(Book &book) const {
    book.physicalLocation = locationEdit_->text().trimmed();
    book.physicalSection = sectionEdit_->text().trimmed();
    book.physicalShelf = shelfEdit_->text().trimmed();
    book.physicalStatus = statusCombo_->currentText();
    book.checkedOutTo = checkedOutToEdit_->text().trimmed();
    book.checkedOutContact = checkedOutContactEdit_->text().trimmed();
    book.checkedOutOn = checkedOutOnEdit_->date().isValid() ? checkedOutOnEdit_->date() : QDate();
    book.dueDate = dueDateEdit_->date().isValid() ? dueDateEdit_->date() : QDate();
    book.accessionNumber = accessionEdit_->text().trimmed();
    book.barcode = barcodeEdit_->text().trimmed();
    book.loanNotes = notesEdit_->toPlainText().trimmed();
}
