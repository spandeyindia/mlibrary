#pragma once
#include <QDialog>
class QLineEdit;
class QTextEdit;
class QLabel;

class MetadataDownloadDialog : public QDialog {
    Q_OBJECT
public:
    explicit MetadataDownloadDialog(QWidget *parent = nullptr);

private slots:
    void runLookup();

private:
    QLineEdit *isbnEdit_ = nullptr;
    QLineEdit *titleEdit_ = nullptr;
    QLineEdit *authorsEdit_ = nullptr;
    QLineEdit *publisherEdit_ = nullptr;
    QLineEdit *publishedEdit_ = nullptr;
    QLineEdit *coverPathEdit_ = nullptr;
    QLabel *coverPreview_ = nullptr;
    QTextEdit *log_ = nullptr;
};
