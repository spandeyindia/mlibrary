#include "SearchDialog.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QTextBrowser>
#include <QVBoxLayout>
#include <algorithm>

static QString escapeHtml(QString s) {
    return s.toHtmlEscaped();
}

SearchDialog::SearchDialog(const QVector<LibraryData> *libraries, QWidget *parent)
    : QDialog(parent), libraries_(libraries) {
    setWindowTitle("Search Engine");
    resize(1000, 700);

    auto *layout = new QVBoxLayout(this);
    auto *topRow = new QWidget(this);
    auto *h = new QHBoxLayout(topRow);
    h->setContentsMargins(0, 0, 0, 0);

    queryEdit_ = new QLineEdit(this);
    queryEdit_->setPlaceholderText("Search books by title, author, subject, tags, remarks, publisher...");
    scopeCombo_ = new QComboBox(this);
    scopeCombo_->addItems({"Current library", "All libraries"});
    favoritesOnlyCheck_ = new QCheckBox("Favorites only", this);
    exactPhraseCheck_ = new QCheckBox("Exact phrase", this);
    auto *searchBtn = new QPushButton("Search", this);
    connect(searchBtn, &QPushButton::clicked, this, &SearchDialog::runSearch);

    h->addWidget(queryEdit_);
    h->addWidget(scopeCombo_);
    h->addWidget(favoritesOnlyCheck_);
    h->addWidget(exactPhraseCheck_);
    h->addWidget(searchBtn);
    layout->addWidget(topRow);

    auto *split = new QWidget(this);
    auto *splitH = new QHBoxLayout(split);
    splitH->setContentsMargins(0, 0, 0, 0);

    resultsList_ = new QListWidget(this);
    details_ = new QTextBrowser(this);
    splitH->addWidget(resultsList_, 2);
    splitH->addWidget(details_, 3);
    layout->addWidget(split);

    connect(resultsList_, &QListWidget::currentRowChanged, this, [this](int) { showSelection(); });
    connect(queryEdit_, &QLineEdit::returnPressed, this, &SearchDialog::runSearch);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
}

QVector<SearchDialog::Result> SearchDialog::searchBooks(const QString &query, bool allLibraries, bool favoritesOnly, bool exactPhrase) const {
    QVector<Result> results;
    if (!libraries_) return results;

    const QString q = query.trimmed().toLower();
    if (q.isEmpty()) return results;

    for (const auto &lib : *libraries_) {
        if (!allLibraries && lib.name != "Main Library") continue;

        for (const auto &book : lib.books) {
            if (favoritesOnly && !book.favorite) continue;

            const QString hay = QStringList{
                book.title, book.subtitle, book.authors.join(" "), book.originalAuthor, book.translatedBy,
                book.subject, book.extraSubjects.join(" "), book.tags.join(" "),
                book.series, book.publisher, book.publisherCountry, book.bookType,
                book.description, book.remarks, book.storagePath,
                book.physicalLocation, book.physicalSection, book.physicalShelf, book.physicalStatus,
                book.checkedOutTo, book.checkedOutContact, book.accessionNumber, book.barcode, book.loanNotes
            }.join(" ").toLower();

            bool match = exactPhrase ? hay.contains(q) : false;
            int score = 0;

            auto addScore = [&](const QString &field, int base) {
                if (field.isEmpty()) return;
                const QString f = field.toLower();
                if (exactPhrase) {
                    if (f.contains(q)) score += base * 2;
                } else {
                    for (const QString &tok : q.split(' ', Qt::SkipEmptyParts)) {
                        if (f.contains(tok)) score += base;
                    }
                }
            };

            addScore(book.title, 30);
            addScore(book.authors.join(" "), 24);
            addScore(book.subject, 18);
            addScore(book.tags.join(" "), 14);
            addScore(book.publisher, 10);
            addScore(book.description, 6);
            addScore(book.remarks, 5);
            addScore(book.storagePath, 3);
            score += book.favorite ? 2 : 0;

            if (exactPhrase) {
                match = hay.contains(q);
            } else {
                match = score > 0;
            }

            if (match) {
                Result r;
                r.library = &lib;
                r.book = &book;
                r.score = score;
                r.snippet = scoreSnippet(book, q);
                results.push_back(r);
            }
        }
    }

    std::sort(results.begin(), results.end(), [](const Result &a, const Result &b) {
        if (a.score != b.score) return a.score > b.score;
        return a.book->title.toLower() < b.book->title.toLower();
    });
    return results;
}

QString SearchDialog::scoreSnippet(const Book &book, const QString &query) const {
    const QString body = QStringList{
        book.title, book.authors.join(", "), book.subject, book.tags.join(", "),
        book.publisher, book.description, book.remarks
    }.join(" | ");
    const int idx = body.toLower().indexOf(query);
    if (idx < 0) return body.left(180);
    const int start = std::max(0, idx - 60);
    return body.mid(start, 180);
}

QString SearchDialog::buildBookSummary(const Book &book) const {
    return QString("%1 — %2 — %3")
        .arg(book.title, book.authors.join(", "), book.subject);
}

void SearchDialog::runSearch() {
    resultsList_->clear();
    const bool allLibraries = scopeCombo_->currentIndex() == 1;
    auto results = searchBooks(queryEdit_->text(), allLibraries, favoritesOnlyCheck_->isChecked(), exactPhraseCheck_->isChecked());

    for (const auto &r : results) {
        const QString line = QString("%1  [%2]  %3")
            .arg(buildBookSummary(*r.book))
            .arg(r.library ? r.library->name : "Library")
            .arg(r.score);
        auto *item = new QListWidgetItem(line, resultsList_);
        item->setData(Qt::UserRole, r.book->id);
        item->setToolTip(r.snippet);
    }

    if (results.isEmpty()) {
        details_->setHtml("<p>No results found.</p>");
    } else {
        resultsList_->setCurrentRow(0);
    }
}

void SearchDialog::showSelection() {
    const int row = resultsList_->currentRow();
    if (row < 0) return;
    auto *item = resultsList_->item(row);
    if (!item) return;
    const int bookId = item->data(Qt::UserRole).toInt();
    for (const auto &lib : *libraries_) {
        for (const auto &book : lib.books) {
            if (book.id == bookId) {
                details_->setHtml(QString(
                    "<h2>%1</h2>"
                    "<p><b>Library:</b> %2</p>"
                    "<p><b>Authors:</b> %3</p>"
                    "<p><b>Subject:</b> %4</p>"
                    "<p><b>Book Type:</b> %5</p>"
                    "<p><b>Tags:</b> %6</p>"
                    "<p><b>Publisher:</b> %7</p>"
                    "<p><b>Remarks:</b> %8</p>"
                    "<p><b>Physical location:</b> %9</p>"
                    "<p><b>Status:</b> %10</p>"
                    "<p><b>Checked out to:</b> %11</p>"
                    "<p><b>Due date:</b> %12</p>"
                    "<p><b>Accession:</b> %13</p>"
                    "<p><b>Formats:</b> %14</p>")
                    .arg(book.title.toHtmlEscaped(),
                         lib.name.toHtmlEscaped(),
                         book.authors.join(", ").toHtmlEscaped(),
                         book.subject.toHtmlEscaped(),
                         book.bookType.toHtmlEscaped(),
                         book.tags.join(", ").toHtmlEscaped(),
                         book.publisher.toHtmlEscaped(),
                         book.remarks.toHtmlEscaped(),
                         book.physicalLocation.toHtmlEscaped(),
                         book.physicalStatus.toHtmlEscaped(),
                         book.checkedOutTo.toHtmlEscaped(),
                         (book.dueDate.isValid() ? book.dueDate.toString(Qt::ISODate) : QString("—")).toHtmlEscaped(),
                         book.accessionNumber.toHtmlEscaped(),
                         book.formats.join(", ").toHtmlEscaped()));
                return;
            }
        }
    }
}
