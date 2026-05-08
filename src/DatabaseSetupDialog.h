#pragma once
#include <QDialog>
class QLineEdit;
class QTextEdit;

class DatabaseSetupDialog : public QDialog {
    Q_OBJECT
public:
    explicit DatabaseSetupDialog(QWidget *parent = nullptr);
    QString resultText() const;
private slots:
    void createTables();
private:
    QString connectionString() const;
    QLineEdit *connEdit_ = nullptr;
    QTextEdit *output_ = nullptr;
    QString resultText_;
};
