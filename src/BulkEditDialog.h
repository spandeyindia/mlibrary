#pragma once
#include <QDialog>
class QLineEdit;
class QTextEdit;
class QCheckBox;

class BulkEditDialog : public QDialog {
    Q_OBJECT
public:
    explicit BulkEditDialog(QWidget *parent = nullptr);

private slots:
    void apply();

private:
    QLineEdit *subjectEdit_ = nullptr;
    QLineEdit *publisherEdit_ = nullptr;
    QLineEdit *remarksEdit_ = nullptr;
    QLineEdit *tagsEdit_ = nullptr;
    QCheckBox *favoriteCheck_ = nullptr;
    QTextEdit *log_ = nullptr;
};
