#pragma once
#include <QDialog>
#include "Book.h"

class QLineEdit;
class QTabWidget;
class QTextBrowser;
class QLabel;
class QPushButton;
class QPdfDocument;
class QPdfView;

class BookViewerDialog : public QDialog {
    Q_OBJECT
public:
    explicit BookViewerDialog(QWidget *parent = nullptr);
    void setBook(const Book &book);
    void openPath(const QString &path);

private slots:
    void browseFile();
    void openCurrent();
    void loadCurrentFile();

private:
    enum class ViewKind { None, Text, Pdf, Image, External };

    ViewKind viewKindForPath(const QString &path) const;
    void showTextFile(const QString &path);
    void showPdfFile(const QString &path);
    void showImageFile(const QString &path);
    void showUnsupported(const QString &path, const QString &reason);
    QString buildConvertedPreview(const QString &path, QString *error) const;
    QString convertIfPossible(const QString &sourcePath, const QString &targetExt, QString *error) const;
    QString safeTempPrefix() const;

    Book currentBook_;
    QString currentPath_;

    QLineEdit *pathEdit_ = nullptr;
    QTextBrowser *textBrowser_ = nullptr;
    QPdfDocument *pdfDocument_ = nullptr;
    QPdfView *pdfView_ = nullptr;
    QLabel *imageLabel_ = nullptr;
    QTextBrowser *statusBrowser_ = nullptr;
    QPushButton *openBtn_ = nullptr;
    QPushButton *browseBtn_ = nullptr;
};
