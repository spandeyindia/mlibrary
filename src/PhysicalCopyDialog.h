#pragma once
#include <QDialog>
#include "Book.h"

class QLineEdit;
class QComboBox;
class QTextEdit;
class QDateEdit;

class PhysicalCopyDialog : public QDialog {
    Q_OBJECT
public:
    explicit PhysicalCopyDialog(QWidget *parent = nullptr);
    void setBook(const Book &book);
    void applyToBook(Book &book) const;

private:
    QLineEdit *locationEdit_ = nullptr;
    QLineEdit *sectionEdit_ = nullptr;
    QLineEdit *shelfEdit_ = nullptr;
    QComboBox *statusCombo_ = nullptr;
    QLineEdit *checkedOutToEdit_ = nullptr;
    QLineEdit *checkedOutContactEdit_ = nullptr;
    QDateEdit *checkedOutOnEdit_ = nullptr;
    QDateEdit *dueDateEdit_ = nullptr;
    QLineEdit *accessionEdit_ = nullptr;
    QLineEdit *barcodeEdit_ = nullptr;
    QTextEdit *notesEdit_ = nullptr;
};
