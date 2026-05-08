#pragma once
#include <QDialog>
#include "Book.h"

class QLineEdit;
class QTextEdit;
class QComboBox;
class QCheckBox;
class QPlainTextEdit;
class QNetworkAccessManager;

class AuthoringHelperDialog : public QDialog {
    Q_OBJECT
public:
    explicit AuthoringHelperDialog(QWidget *parent = nullptr);
    void setBook(const Book &book);

private slots:
    void generate();
    void copyOutput();

private:
    QString buildPrompt() const;
    QString localFallback(const QString &mode) const;
    void sendToAiEndpoint(const QString &prompt);

    Book book_;
    QComboBox *modeCombo_ = nullptr;
    QLineEdit *titleEdit_ = nullptr;
    QLineEdit *authorsEdit_ = nullptr;
    QLineEdit *subjectEdit_ = nullptr;
    QLineEdit *audienceEdit_ = nullptr;
    QLineEdit *toneEdit_ = nullptr;
    QLineEdit *keywordsEdit_ = nullptr;
    QPlainTextEdit *output_ = nullptr;
    QCheckBox *useRemoteCheck_ = nullptr;
    QLineEdit *endpointEdit_ = nullptr;
    QLineEdit *apiKeyEdit_ = nullptr;
    QLineEdit *modelEdit_ = nullptr;
    QPlainTextEdit *systemPromptEdit_ = nullptr;
    QNetworkAccessManager *nam_ = nullptr;
};
