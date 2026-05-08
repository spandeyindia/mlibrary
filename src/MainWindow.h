#pragma once
#include <QMainWindow>
#include "Book.h"

class QListWidget;
class QLineEdit;
class QTableWidget;
class QTextBrowser;
class QPushButton;
class QStackedWidget;
class QLabel;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
private slots:
    void onSetupDatabase();
    void onAddLibrary();
    void onAddBook();
    void onEditMetadata();
    void onBulkEditMetadata();
    void onDownloadMetadata();
    void onDeleteBook();
    void onMergeDuplicates();
    void onConvertFile();
    void onImportWebsite();
    void onStartWebServer();
    void onOpenViewer();
    void onCopyMoveBooks();
    void onModifySubject();
    void onLibraryChanged();
    void onBookSelectionChanged();
    void showDetailsTab();
    void showPreviewTab();
    void onSearchChanged(const QString &text);
private:
    void buildUi();
    void loadSampleData();
    void refreshLibraryLists();
    void refreshBooks();
    void refreshRightPanel();
    LibraryData *activeLibrary();
    const LibraryData *activeLibrary() const;
    QVector<Book> filteredBooks() const;
    int currentSelectedBookId() const;
    Book *findBookById(int id);
    const Book *findBookById(int id) const;
    QString bookToHtml(const Book &book) const;
    QString previewToHtml(const Book &book) const;
    static QString joinStrings(const QStringList &values);
    static QString slugify(const QString &text);

    QVector<LibraryData> libraries_;
    QString activeLibraryName_;
    int nextBookId_ = 100;
    QString searchText_;

    QListWidget *libraryList_ = nullptr;
    QLineEdit *searchEdit_ = nullptr;
    QTableWidget *bookTable_ = nullptr;
    QTextBrowser *detailsView_ = nullptr;
    QTextBrowser *previewView_ = nullptr;
    QPushButton *detailsButton_ = nullptr;
    QPushButton *previewButton_ = nullptr;
    QStackedWidget *rightStack_ = nullptr;
    QLabel *statusLabel_ = nullptr;
};
