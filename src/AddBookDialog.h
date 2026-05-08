#pragma once
#include <QDialog>
#include "Book.h"

class QLineEdit;
class QTextEdit;
class QCheckBox;

class AddBookDialog : public QDialog {
    Q_OBJECT
public:
    explicit AddBookDialog(QWidget *parent = nullptr);
    Book book() const;
    void setNextBookId(int id);
private:
    int nextBookId_ = 1;
    QLineEdit *titleEdit_ = nullptr;
    QLineEdit *authorsEdit_ = nullptr;
    QLineEdit *subjectEdit_ = nullptr;
    QLineEdit *bookTypeEdit_ = nullptr;
    QLineEdit *formatsEdit_ = nullptr;
    QLineEdit *publisherEdit_ = nullptr;
    QLineEdit *publishedEdit_ = nullptr;
    QTextEdit *remarksEdit_ = nullptr;
    QCheckBox *favoriteCheck_ = nullptr;
};
