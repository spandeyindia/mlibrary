#include "MainWindow.h"
#include "AddBookDialog.h"
#include "AddLibraryDialog.h"
#include "BulkEditDialog.h"
#include "ConversionDialog.h"
#include "CopyMoveDialog.h"
#include "DatabaseSetupDialog.h"
#include "MetadataDownloadDialog.h"
#include "SubjectEditDialog.h"
#include "WebsiteImportDialog.h"
#include "WebServerDialog.h"
#include "BookViewerDialog.h"

#include <QAction>
#include <QDate>
#include <QDateTime>
#include <QDialog>
#include <QFormLayout>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QSplitter>
#include <QStackedWidget>
#include <QTableWidget>
#include <QTextBrowser>
#include <QToolBar>
#include <QVBoxLayout>
#include <QWidget>
#include <algorithm>

static QString htmlEscape(const QString &text) {
    QString s = text.toHtmlEscaped();
    s.replace("\n", "<br/>");
    return s;
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("Ebook Library");
    resize(1640, 940);
    loadSampleData();
    buildUi();
    refreshLibraryLists();
    refreshBooks();
    refreshRightPanel();
}

void MainWindow::buildUi() {
    auto *toolbar = addToolBar("Main");
    toolbar->setMovable(false);

    auto *setupAction = toolbar->addAction("Setup DB");
    connect(setupAction, &QAction::triggered, this, &MainWindow::onSetupDatabase);

    auto *addLibraryAction = toolbar->addAction("Add Library");
    connect(addLibraryAction, &QAction::triggered, this, &MainWindow::onAddLibrary);

    auto *addBookAction = toolbar->addAction("Add Book");
    connect(addBookAction, &QAction::triggered, this, &MainWindow::onAddBook);

    auto *editMetadataAction = toolbar->addAction("Edit Metadata");
    connect(editMetadataAction, &QAction::triggered, this, &MainWindow::onEditMetadata);

    auto *bulkEditAction = toolbar->addAction("Bulk Edit");
    connect(bulkEditAction, &QAction::triggered, this, &MainWindow::onBulkEditMetadata);

    auto *downloadAction = toolbar->addAction("Download Metadata/Cover");
    connect(downloadAction, &QAction::triggered, this, &MainWindow::onDownloadMetadata);

    auto *convertAction = toolbar->addAction("Convert File");
    connect(convertAction, &QAction::triggered, this, &MainWindow::onConvertFile);

    auto *websiteAction = toolbar->addAction("Import Website");
    connect(websiteAction, &QAction::triggered, this, &MainWindow::onImportWebsite);

    auto *webServerAction = toolbar->addAction("Start Web Server");
    connect(webServerAction, &QAction::triggered, this, &MainWindow::onStartWebServer);

    auto *copyMoveAction = toolbar->addAction("Copy / Move");
    connect(copyMoveAction, &QAction::triggered, this, &MainWindow::onCopyMoveBooks);

    auto *subjectAction = toolbar->addAction("Modify Subject");
    connect(subjectAction, &QAction::triggered, this, &MainWindow::onModifySubject);

    auto *viewerAction = toolbar->addAction("Book Viewer");
    connect(viewerAction, &QAction::triggered, this, &MainWindow::onOpenViewer);

    auto *mergeAction = toolbar->addAction("Merge Records");
    connect(mergeAction, &QAction::triggered, this, &MainWindow::onMergeDuplicates);

    auto *deleteAction = toolbar->addAction("Delete");
    connect(deleteAction, &QAction::triggered, this, &MainWindow::onDeleteBook);

    toolbar->addSeparator();

    searchEdit_ = new QLineEdit(this);
    searchEdit_->setPlaceholderText("Search title, author, tags, subject, remarks...");
    searchEdit_->setMinimumWidth(420);
    connect(searchEdit_, &QLineEdit::textChanged, this, &MainWindow::onSearchChanged);
    toolbar->addWidget(searchEdit_);

    auto *splitter = new QSplitter(this);
    splitter->setOrientation(Qt::Horizontal);
    setCentralWidget(splitter);

    auto *leftPane = new QWidget(splitter);
    auto *leftLayout = new QVBoxLayout(leftPane);
    leftLayout->setContentsMargins(8, 8, 8, 8);

    libraryList_ = new QListWidget(leftPane);
    connect(libraryList_, &QListWidget::currentRowChanged, this, [this](int) { onLibraryChanged(); });

    leftLayout->addWidget(new QLabel("Libraries"));
    leftLayout->addWidget(libraryList_);
    leftLayout->addWidget(new QLabel("Subjects"));
    leftLayout->addWidget(new QListWidget(leftPane));
    leftLayout->addWidget(new QLabel("Tags"));
    leftLayout->addWidget(new QListWidget(leftPane));
    leftLayout->addWidget(new QLabel("Series"));
    leftLayout->addWidget(new QListWidget(leftPane));
    leftLayout->addWidget(new QLabel("Publishers"));
    leftLayout->addWidget(new QListWidget(leftPane));
    leftLayout->addWidget(new QLabel("Book Types"));
    leftLayout->addWidget(new QListWidget(leftPane));
    leftLayout->addStretch();

    auto *centerPane = new QWidget(splitter);
    auto *centerLayout = new QVBoxLayout(centerPane);
    centerLayout->setContentsMargins(8, 8, 8, 8);

    statusLabel_ = new QLabel("Ready.", centerPane);
    centerLayout->addWidget(statusLabel_);

    bookTable_ = new QTableWidget(centerPane);
    bookTable_->setColumnCount(10);
    bookTable_->setHorizontalHeaderLabels({"★","Title","Authors","Subject","Book Type","Publisher","Series","Formats","Published","Added"});
    bookTable_->horizontalHeader()->setStretchLastSection(true);
    bookTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    bookTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    bookTable_->setSelectionMode(QAbstractItemView::SingleSelection);
    bookTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    connect(bookTable_, &QTableWidget::itemSelectionChanged, this, &MainWindow::onBookSelectionChanged);
    centerLayout->addWidget(bookTable_);

    auto *rightPane = new QWidget(splitter);
    auto *rightLayout = new QVBoxLayout(rightPane);
    rightLayout->setContentsMargins(8, 8, 8, 8);

    auto *toggleRow = new QWidget(rightPane);
    auto *toggleLayout = new QHBoxLayout(toggleRow);
    toggleLayout->setContentsMargins(0, 0, 0, 0);

    detailsButton_ = new QPushButton("Details", toggleRow);
    previewButton_ = new QPushButton("Preview", toggleRow);
    connect(detailsButton_, &QPushButton::clicked, this, &MainWindow::showDetailsTab);
    connect(previewButton_, &QPushButton::clicked, this, &MainWindow::showPreviewTab);

    toggleLayout->addWidget(detailsButton_);
    toggleLayout->addWidget(previewButton_);
    toggleLayout->addStretch();
    rightLayout->addWidget(toggleRow);

    rightStack_ = new QStackedWidget(rightPane);
    detailsView_ = new QTextBrowser(rightStack_);
    previewView_ = new QTextBrowser(rightStack_);
    rightStack_->addWidget(detailsView_);
    rightStack_->addWidget(previewView_);
    rightLayout->addWidget(rightStack_);

    splitter->addWidget(leftPane);
    splitter->addWidget(centerPane);
    splitter->addWidget(rightPane);
    splitter->setStretchFactor(1, 4);
    splitter->setStretchFactor(2, 2);

    showDetailsTab();
}

void MainWindow::loadSampleData() {
    LibraryData mainLib;
    mainLib.name = "Main Library";
    mainLib.rootPath = "/library/main";

    Book hobbit;
    hobbit.id = nextBookId_++;
    hobbit.title = "The Hobbit";
    hobbit.authors = {"J. R. R. Tolkien"};
    hobbit.originalAuthor = "J. R. R. Tolkien";
    hobbit.originalLanguage = "English";
    hobbit.subject = "Fantasy";
    hobbit.extraSubjects = {"Adventure", "Classic Literature"};
    hobbit.tags = {"favorite", "classic", "to-read"};
    hobbit.series = "Middle-earth";
    hobbit.seriesNumber = "1";
    hobbit.publisher = "George Allen & Unwin";
    hobbit.publisherCountry = "United Kingdom";
    hobbit.bookType = "Novel";
    hobbit.publishedDate = QDate(1937, 9, 21);
    hobbit.addedDate = QDateTime::currentDateTime();
    hobbit.remarks = "First edition scan plus rerelease EPUB.";
    hobbit.favorite = true;
    hobbit.description = "A fantasy novel and children's book by English author J. R. R. Tolkien.";
    hobbit.formats = {"EPUB", "PDF"};
    hobbit.storagePath = "Main Library/Fantasy/Tolkien_J_R_R/the_hobbit.epub";
    hobbit.checksum = "b8f7e1b9...";
    mainLib.books.push_back(hobbit);

    Book clean;
    clean.id = nextBookId_++;
    clean.title = "Clean Code";
    clean.authors = {"Robert C. Martin"};
    clean.originalAuthor = "Robert C. Martin";
    clean.originalLanguage = "English";
    clean.subject = "Computer Science";
    clean.extraSubjects = {"Programming", "Software Engineering"};
    clean.tags = {"reference", "important"};
    clean.publisher = "Prentice Hall";
    clean.publisherCountry = "United States";
    clean.bookType = "Text Book";
    clean.publishedDate = QDate(2008, 8, 1);
    clean.addedDate = QDateTime::currentDateTime();
    clean.remarks = "Useful for coding standards and code review guidelines.";
    clean.description = "A handbook of agile software craftsmanship.";
    clean.formats = {"EPUB", "PDF", "MOBI"};
    clean.storagePath = "Main Library/Computer_Science/Martin_Robert_C/clean_code.epub";
    clean.checksum = "c112fd17...";
    mainLib.books.push_back(clean);

    LibraryData research;
    research.name = "Research Library";
    research.rootPath = "/library/research";

    Book notes;
    notes.id = nextBookId_++;
    notes.title = "Distributed Systems Notes";
    notes.authors = {"Sanjay Pandey"};
    notes.originalLanguage = "English";
    notes.subject = "Computer Science";
    notes.extraSubjects = {"Distributed Systems", "Reference"};
    notes.tags = {"thesis", "draft"};
    notes.bookType = "Thesis";
    notes.publishedDate = QDate(2024, 12, 10);
    notes.addedDate = QDateTime::currentDateTime();
    notes.remarks = "Imported from scanned PDF and OCR text.";
    notes.description = "A research-oriented document containing notes and references.";
    notes.formats = {"PDF"};
    notes.storagePath = "Research Library/Computer_Science/Pandey_Sanjay/distributed_systems_notes.pdf";
    notes.checksum = "de9ca49a...";
    research.books.push_back(notes);

    LibraryData comics;
    comics.name = "Comics Library";
    comics.rootPath = "/library/comics";

    Book onePiece;
    onePiece.id = nextBookId_++;
    onePiece.title = "One Piece Vol. 1";
    onePiece.authors = {"Eiichiro Oda"};
    onePiece.originalAuthor = "Eiichiro Oda";
    onePiece.originalLanguage = "Japanese";
    onePiece.translatedBy = "VIZ Media";
    onePiece.subject = "Manga";
    onePiece.extraSubjects = {"Adventure", "Comic"};
    onePiece.tags = {"favorite", "manga"};
    onePiece.series = "One Piece";
    onePiece.seriesNumber = "1";
    onePiece.publisher = "Shueisha";
    onePiece.publisherCountry = "Japan";
    onePiece.bookType = "Comic";
    onePiece.publishedDate = QDate(1997, 12, 24);
    onePiece.addedDate = QDateTime::currentDateTime();
    onePiece.remarks = "Best to preview with page image mode.";
    onePiece.favorite = true;
    onePiece.description = "A manga series following Monkey D. Luffy and his crew.";
    onePiece.formats = {"CBZ", "PDF"};
    onePiece.storagePath = "Comics Library/Manga/Oda_Eiichiro/one_piece_vol_1.cbz";
    onePiece.checksum = "ab88c220...";
    comics.books.push_back(onePiece);

    libraries_ = {mainLib, research, comics};
    activeLibraryName_ = "Main Library";
}

void MainWindow::refreshLibraryLists() {
    libraryList_->clear();
    for (const auto &lib : libraries_) libraryList_->addItem(lib.name);
    for (int i = 0; i < libraryList_->count(); ++i) {
        if (libraryList_->item(i)->text() == activeLibraryName_) { libraryList_->setCurrentRow(i); break; }
    }
    if (libraryList_->currentRow() < 0 && libraryList_->count() > 0) {
        libraryList_->setCurrentRow(0);
        activeLibraryName_ = libraryList_->currentItem()->text();
    }
}

LibraryData *MainWindow::activeLibrary() { for (auto &lib : libraries_) if (lib.name == activeLibraryName_) return &lib; return libraries_.isEmpty() ? nullptr : &libraries_.first(); }
const LibraryData *MainWindow::activeLibrary() const { for (const auto &lib : libraries_) if (lib.name == activeLibraryName_) return &lib; return libraries_.isEmpty() ? nullptr : &libraries_.first(); }

QString MainWindow::joinStrings(const QStringList &values) { return values.join(", "); }
QString MainWindow::slugify(const QString &text) { QString s=text.trimmed().toLower(); for (QChar &ch : s) if (!ch.isLetterOrNumber()) ch='_'; while(s.contains("__")) s.replace("__","_"); return s.trimmed('_'); }

QVector<Book> MainWindow::filteredBooks() const {
    const auto *lib = activeLibrary();
    if (!lib) return {};
    QVector<Book> out;
    const QString q = searchText_.trimmed().toLower();
    for (const auto &book : lib->books) {
        if (q.isEmpty()) { out.push_back(book); continue; }
        const QString hay = QStringList{book.title, book.subtitle, book.authors.join(" "), book.originalAuthor, book.translatedBy, book.subject, book.extraSubjects.join(" "), book.tags.join(" "), book.series, book.publisher, book.bookType, book.remarks, book.description}.join(" ").toLower();
        if (hay.contains(q)) out.push_back(book);
    }
    return out;
}

void MainWindow::refreshBooks() {
    auto books = filteredBooks();
    bookTable_->setRowCount(books.size());
    for (int r=0;r<books.size();++r) {
        const auto &b = books[r];
        bookTable_->setItem(r,0,new QTableWidgetItem(b.favorite ? "★" : ""));
        bookTable_->setItem(r,1,new QTableWidgetItem(b.title));
        bookTable_->setItem(r,2,new QTableWidgetItem(joinStrings(b.authors)));
        bookTable_->setItem(r,3,new QTableWidgetItem(b.subject));
        bookTable_->setItem(r,4,new QTableWidgetItem(b.bookType));
        bookTable_->setItem(r,5,new QTableWidgetItem(b.publisher.isEmpty() ? "—" : b.publisher));
        bookTable_->setItem(r,6,new QTableWidgetItem(b.series.isEmpty() ? "—" : b.series));
        bookTable_->setItem(r,7,new QTableWidgetItem(joinStrings(b.formats)));
        bookTable_->setItem(r,8,new QTableWidgetItem(b.publishedDate.isValid() ? b.publishedDate.toString(Qt::ISODate) : "—"));
        bookTable_->setItem(r,9,new QTableWidgetItem(b.addedDate.toString("yyyy-MM-dd hh:mm")));
    }
    if (bookTable_->rowCount() > 0 && bookTable_->currentRow() < 0) bookTable_->selectRow(0);
    statusLabel_->setText(QString("%1 books shown in %2").arg(books.size()).arg(activeLibraryName_));
    refreshRightPanel();
}

int MainWindow::currentSelectedBookId() const {
    const int row = bookTable_->currentRow();
    if (row < 0) return -1;
    const auto books = filteredBooks();
    if (row >= books.size()) return -1;
    return books[row].id;
}
Book *MainWindow::findBookById(int id) { auto *lib = activeLibrary(); if (!lib) return nullptr; for (auto &book : lib->books) if (book.id==id) return &book; return nullptr; }
const Book *MainWindow::findBookById(int id) const { const auto *lib = activeLibrary(); if (!lib) return nullptr; for (const auto &book : lib->books) if (book.id==id) return &book; return nullptr; }

QString MainWindow::bookToHtml(const Book &book) const {
    QString html;
    html += "<h2>" + htmlEscape(book.title) + "</h2>";
    if (!book.subtitle.isEmpty()) html += "<p><i>" + htmlEscape(book.subtitle) + "</i></p>";
    html += "<p><b>Authors:</b> " + htmlEscape(book.authors.join(", ")) + "</p>";
    html += "<p><b>Original Author:</b> " + htmlEscape(book.originalAuthor.isEmpty() ? "—" : book.originalAuthor) + "</p>";
    html += "<p><b>Original Language:</b> " + htmlEscape(book.originalLanguage.isEmpty() ? "—" : book.originalLanguage) + "</p>";
    html += "<p><b>Translated By:</b> " + htmlEscape(book.translatedBy.isEmpty() ? "—" : book.translatedBy) + "</p>";
    html += "<p><b>Subject:</b> " + htmlEscape(book.subject) + "</p>";
    html += "<p><b>Other Subjects:</b> " + htmlEscape(book.extraSubjects.join(", ")) + "</p>";
    html += "<p><b>Tags:</b> " + htmlEscape(book.tags.join(", ")) + "</p>";
    html += "<p><b>Series:</b> " + htmlEscape(book.series.isEmpty() ? "—" : book.series) + "</p>";
    html += "<p><b>Publisher:</b> " + htmlEscape(book.publisher.isEmpty() ? "—" : book.publisher) + "</p>";
    html += "<p><b>Book Type:</b> " + htmlEscape(book.bookType) + "</p>";
    html += "<p><b>Published:</b> " + htmlEscape(book.publishedDate.isValid() ? book.publishedDate.toString(Qt::ISODate) : "—") + "</p>";
    html += "<p><b>Added:</b> " + htmlEscape(book.addedDate.toString("yyyy-MM-dd hh:mm")) + "</p>";
    html += "<p><b>Remarks:</b> " + htmlEscape(book.remarks.isEmpty() ? "—" : book.remarks) + "</p>";
    html += "<p><b>Formats:</b> " + htmlEscape(book.formats.join(", ")) + "</p>";
    html += "<p><b>Storage Path:</b> " + htmlEscape(book.storagePath) + "</p>";
    html += "<p><b>Checksum:</b> " + htmlEscape(book.checksum) + "</p>";
    if (!book.coverPath.isEmpty()) html += "<p><b>Cover Path:</b> " + htmlEscape(book.coverPath) + "</p>";
    return html;
}
QString MainWindow::previewToHtml(const Book &book) const { return "<h2>Preview</h2><p><b>" + htmlEscape(book.title) + "</b></p><p>" + htmlEscape(book.description) + "</p><p><b>Formats:</b> " + htmlEscape(book.formats.join(", ")) + "</p>"; }
void MainWindow::refreshRightPanel() { const int id=currentSelectedBookId(); const auto *book=findBookById(id); if(!book){ detailsView_->setHtml("<p>No book selected.</p>"); previewView_->setHtml("<p>No book selected.</p>"); return; } detailsView_->setHtml(bookToHtml(*book)); previewView_->setHtml(previewToHtml(*book)); }

void MainWindow::onSetupDatabase() { DatabaseSetupDialog dialog(this); if (dialog.exec()==QDialog::Accepted) statusLabel_->setText(dialog.resultText()); }

void MainWindow::onAddLibrary() {
    AddLibraryDialog dialog(this);
    if (dialog.exec()==QDialog::Accepted) {
        const QString name = dialog.libraryName();
        if (name.isEmpty()) return;
        if (std::any_of(libraries_.cbegin(), libraries_.cend(), [&](const LibraryData &lib){ return lib.name == name; })) {
            QMessageBox::warning(this, "Add Library", "A library with this name already exists.");
            return;
        }
        LibraryData lib; lib.name = name; lib.rootPath = dialog.rootPath().isEmpty() ? QString("/library/%1").arg(slugify(name)) : dialog.rootPath(); libraries_.push_back(lib); activeLibraryName_ = name; refreshLibraryLists(); refreshBooks();
    }
}

void MainWindow::onAddBook() {
    auto *lib = activeLibrary(); if (!lib) return;
    AddBookDialog dialog(this); dialog.setNextBookId(nextBookId_++);
    if (dialog.exec()==QDialog::Accepted) {
        Book b = dialog.book();
        if (b.title.trimmed().isEmpty()) { QMessageBox::warning(this, "Add Book", "Title is required."); return; }
        const QString subject = b.subject.isEmpty() ? "General" : b.subject;
        const QString author = b.authors.isEmpty() ? "Unknown Author" : b.authors.first();
        const QString ext = b.formats.isEmpty() ? "epub" : b.formats.first().toLower();
        b.storagePath = QString("%1/%2/%3/%4.%5").arg(lib->rootPath.isEmpty() ? QString("/library/%1").arg(slugify(lib->name)) : lib->rootPath, slugify(subject), slugify(author), slugify(b.title), ext);
        if (b.description.isEmpty()) b.description = "Imported through the add-book dialog.";
        lib->books.push_back(b); refreshBooks(); statusLabel_->setText(QString("Added book: %1").arg(b.title));
    }
}

void MainWindow::onEditMetadata() {
    auto *lib = activeLibrary(); const int id = currentSelectedBookId(); if (!lib || id < 0) return; auto *book = findBookById(id); if (!book) return;
    AddBookDialog dialog(this); dialog.setNextBookId(book->id);
    if (dialog.exec()==QDialog::Accepted) { Book edited = dialog.book(); edited.id = book->id; edited.storagePath = book->storagePath; edited.coverPath = book->coverPath; *book = edited; refreshBooks(); }
}

void MainWindow::onBulkEditMetadata() { BulkEditDialog dialog(this); dialog.exec(); }
void MainWindow::onDownloadMetadata() { MetadataDownloadDialog dialog(this); dialog.exec(); }
void MainWindow::onConvertFile() { ConversionDialog dialog(this); dialog.exec(); }
void MainWindow::onImportWebsite() { WebsiteImportDialog dialog(this); dialog.exec(); }

void MainWindow::onStartWebServer() {
    WebServerDialog dialog(this);
    dialog.setLibraries(&libraries_);
    dialog.exec();
}
void MainWindow::onCopyMoveBooks() { CopyMoveDialog dialog(this); dialog.exec(); }
void MainWindow::onModifySubject() { SubjectEditDialog dialog(this); dialog.exec(); }

void MainWindow::onOpenViewer() {
    auto *book = findBookById(currentSelectedBookId());
    BookViewerDialog dialog(this);
    if (book) dialog.setBook(*book);
    dialog.exec();
}

void MainWindow::onDeleteBook() {
    auto *lib = activeLibrary(); const int id = currentSelectedBookId(); if (!lib || id < 0) return; const auto *book = findBookById(id); if (!book) return;
    if (QMessageBox::question(this, "Delete Book", QString("Permanently delete '%1'?").arg(book->title)) != QMessageBox::Yes) return;
    auto it = std::remove_if(lib->books.begin(), lib->books.end(), [&](const Book &b){ return b.id == id; });
    if (it != lib->books.end()) lib->books.erase(it, lib->books.end());
    refreshBooks(); statusLabel_->setText("Book deleted permanently.");
}

void MainWindow::onMergeDuplicates() {
    auto *lib = activeLibrary(); const int id = currentSelectedBookId(); if (!lib || id < 0) return; auto *base = findBookById(id); if (!base) return;
    const QString targetTitle = base->title.trimmed().toLower();
    QVector<Book> duplicates;
    for (const auto &book : lib->books) if (book.id != id && book.title.trimmed().toLower() == targetTitle) duplicates.push_back(book);
    if (duplicates.isEmpty()) { QMessageBox::information(this, "Merge Records", "No duplicate records with the same title were found in this library."); return; }
    for (const auto &dup : duplicates) {
        for (const auto &fmt : dup.formats) if (!base->formats.contains(fmt)) base->formats.push_back(fmt);
        for (const auto &tag : dup.tags) if (!base->tags.contains(tag)) base->tags.push_back(tag);
        for (const auto &subject : dup.extraSubjects) if (!base->extraSubjects.contains(subject)) base->extraSubjects.push_back(subject);
        if (base->remarks.isEmpty()) base->remarks = dup.remarks;
        if (base->coverPath.isEmpty()) base->coverPath = dup.coverPath;
    }
    auto newEnd = std::remove_if(lib->books.begin(), lib->books.end(), [&](const Book &b){ return b.id != id && b.title.trimmed().toLower() == targetTitle; });
    lib->books.erase(newEnd, lib->books.end());
    refreshBooks(); statusLabel_->setText("Duplicate records merged.");
}

void MainWindow::onLibraryChanged() { if (libraryList_->currentItem()) { activeLibraryName_ = libraryList_->currentItem()->text(); refreshBooks(); refreshLibraryLists(); } }
void MainWindow::onBookSelectionChanged() { refreshRightPanel(); }
void MainWindow::showDetailsTab() { rightStack_->setCurrentWidget(detailsView_); detailsButton_->setEnabled(false); previewButton_->setEnabled(true); }
void MainWindow::showPreviewTab() { rightStack_->setCurrentWidget(previewView_); detailsButton_->setEnabled(true); previewButton_->setEnabled(false); }
void MainWindow::onSearchChanged(const QString &text) { searchText_ = text; refreshBooks(); }
