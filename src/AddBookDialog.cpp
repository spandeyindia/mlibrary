#include "AddBookDialog.h"
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QTextEdit>
#include <QVBoxLayout>

AddBookDialog::AddBookDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Add Book");
    auto *layout = new QVBoxLayout(this);
    auto *form = new QFormLayout();
    titleEdit_ = new QLineEdit(this);
    authorsEdit_ = new QLineEdit(this);
    subjectEdit_ = new QLineEdit(this);
    bookTypeEdit_ = new QLineEdit(this);
    formatsEdit_ = new QLineEdit(this);
    publisherEdit_ = new QLineEdit(this);
    publishedEdit_ = new QLineEdit(this);
    remarksEdit_ = new QTextEdit(this);
    favoriteCheck_ = new QCheckBox("Favorite", this);
    formatsEdit_->setPlaceholderText("EPUB, PDF, MOBI");
    publishedEdit_->setPlaceholderText("YYYY-MM-DD");
    form->addRow("Title:", titleEdit_);
    form->addRow("Authors:", authorsEdit_);
    form->addRow("Subject:", subjectEdit_);
    form->addRow("Book type:", bookTypeEdit_);
    form->addRow("Formats:", formatsEdit_);
    form->addRow("Publisher:", publisherEdit_);
    form->addRow("Published date:", publishedEdit_);
    form->addRow("Remarks:", remarksEdit_);
    layout->addLayout(form);
    layout->addWidget(favoriteCheck_);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
}
void AddBookDialog::setNextBookId(int id) { nextBookId_ = id; }
Book AddBookDialog::book() const {
    Book b;
    b.id = nextBookId_;
    b.title = titleEdit_->text().trimmed();
    b.authors = authorsEdit_->text().split(',', Qt::SkipEmptyParts);
    for (auto &a : b.authors) a = a.trimmed();
    b.subject = subjectEdit_->text().trimmed();
    b.bookType = bookTypeEdit_->text().trimmed();
    b.formats = formatsEdit_->text().split(',', Qt::SkipEmptyParts);
    for (auto &f : b.formats) f = f.trimmed();
    b.publisher = publisherEdit_->text().trimmed();
    const auto pd = QDate::fromString(publishedEdit_->text().trimmed(), Qt::ISODate);
    if (pd.isValid()) b.publishedDate = pd;
    b.remarks = remarksEdit_->toPlainText().trimmed();
    b.favorite = favoriteCheck_->isChecked();
    b.addedDate = QDateTime::currentDateTime();
    b.description = QString("Added from dialog.");
    return b;
}
