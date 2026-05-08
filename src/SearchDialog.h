#pragma once
#include <QDialog>
#include "Book.h"

class QLineEdit;
class QComboBox;
class QListWidget;
class QTextBrowser;
class QCheckBox;

class SearchDialog : public QDialog {
    Q_OBJECT
public:
    explicit SearchDialog(const QVector<LibraryData> *libraries, QWidget *parent = nullptr);

private slots:
    void runSearch();
    void showSelection();

private:
    struct Result {
        const LibraryData *library = nullptr;
        const Book *book = nullptr;
        int score = 0;
        QString snippet;
    };

    QVector<Result> searchBooks(const QString &query, bool allLibraries, bool favoritesOnly, bool exactPhrase) const;
    QString scoreSnippet(const Book &book, const QString &query) const;
    QString buildBookSummary(const Book &book) const;

    const QVector<LibraryData> *libraries_ = nullptr;
    QLineEdit *queryEdit_ = nullptr;
    QComboBox *scopeCombo_ = nullptr;
    QCheckBox *favoritesOnlyCheck_ = nullptr;
    QCheckBox *exactPhraseCheck_ = nullptr;
    QListWidget *resultsList_ = nullptr;
    QTextBrowser *details_ = nullptr;
};
