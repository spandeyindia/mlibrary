#pragma once
#include <QDialog>
class QLineEdit;
class QTextEdit;

class SubjectEditDialog : public QDialog {
    Q_OBJECT
public:
    explicit SubjectEditDialog(QWidget *parent = nullptr);

private slots:
    void apply();

private:
    QLineEdit *subjectEdit_ = nullptr;
    QTextEdit *log_ = nullptr;
};
