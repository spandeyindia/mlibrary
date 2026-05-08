#include "BookViewerDialog.h"
#include "ConversionService.h"

#include <QBuffer>
#include <QPixmap>
#include <QTabWidget>
#include <QFileInfo>
#include <QDialogButtonBox>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QImage>
#include <QImageReader>
#include <QLabel>
#include <QLineEdit>
#include <QPdfDocument>
#include <QPdfView>
#include <QPushButton>
#include <QScrollArea>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTextBrowser>
#include <QVBoxLayout>

BookViewerDialog::BookViewerDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Book Viewer");
    resize(1100, 800);

    auto *layout = new QVBoxLayout(this);
    auto *form = new QFormLayout();

    pathEdit_ = new QLineEdit(this);
    browseBtn_ = new QPushButton("Browse", this);
    openBtn_ = new QPushButton("Open", this);

    auto *row = new QWidget(this);
    auto *rowLayout = new QHBoxLayout(row);
    rowLayout->setContentsMargins(0, 0, 0, 0);
    rowLayout->addWidget(pathEdit_);
    rowLayout->addWidget(browseBtn_);
    rowLayout->addWidget(openBtn_);

    form->addRow("File:", row);
    layout->addLayout(form);

    connect(browseBtn_, &QPushButton::clicked, this, &BookViewerDialog::browseFile);
    connect(openBtn_, &QPushButton::clicked, this, &BookViewerDialog::openCurrent);

    auto *tabs = new QTabWidget(this);

    textBrowser_ = new QTextBrowser(tabs);
    pdfDocument_ = new QPdfDocument(this);
    pdfView_ = new QPdfView(tabs);
    pdfView_->setDocument(pdfDocument_);
    imageLabel_ = new QLabel(tabs);
    imageLabel_->setAlignment(Qt::AlignCenter);
    imageLabel_->setMinimumSize(700, 500);
    imageLabel_->setScaledContents(false);

    statusBrowser_ = new QTextBrowser(tabs);

    tabs->addTab(textBrowser_, "Text / HTML");
    tabs->addTab(pdfView_, "PDF");
    tabs->addTab(imageLabel_, "Image");
    tabs->addTab(statusBrowser_, "Status");

    layout->addWidget(tabs);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
}

void BookViewerDialog::setBook(const Book &book) {
    currentBook_ = book;
    pathEdit_->setText(book.storagePath);
    openCurrent();
}

void BookViewerDialog::openPath(const QString &path) {
    pathEdit_->setText(path);
    openCurrent();
}

void BookViewerDialog::browseFile() {
    const QString path = QFileDialog::getOpenFileName(this, "Open ebook file");
    if (!path.isEmpty()) pathEdit_->setText(path);
}

QString BookViewerDialog::safeTempPrefix() const {
    return "mlibrary_preview_";
}

BookViewerDialog::ViewKind BookViewerDialog::viewKindForPath(const QString &path) const {
    const QString ext = QFileInfo(path).suffix().toLower();
    if (ext == "pdf") return ViewKind::Pdf;
    if (ext == "txt" || ext == "text" || ext == "md" || ext == "html" || ext == "htm" || ext == "xhtml" || ext == "xml" || ext == "rtf") return ViewKind::Text;
    if (ext == "png" || ext == "jpg" || ext == "jpeg" || ext == "gif" || ext == "bmp" || ext == "webp" || ext == "svg") return ViewKind::Image;
    if (ext == "epub" || ext == "mobi" || ext == "azw" || ext == "azw3" || ext == "dvi" || ext == "cbr" || ext == "cbz" || ext == "cdr") return ViewKind::External;
    return ViewKind::External;
}

void BookViewerDialog::openCurrent() {
    currentPath_ = pathEdit_->text().trimmed();
    loadCurrentFile();
}

QString BookViewerDialog::convertIfPossible(const QString &sourcePath, const QString &targetExt, QString *error) const {
    Q_UNUSED(targetExt);
    QTemporaryDir tmp;
    if (!tmp.isValid()) {
        if (error) *error = "Unable to create temporary directory.";
        return {};
    }
    const QString outPath = tmp.path() + "/" + safeTempPrefix() + "preview.pdf";
    QString log;
    if (!ConversionService::convertFile(ConversionService::defaultEbookConvertPath(), sourcePath, outPath, &log, error)) {
        return {};
    }
    return outPath;
}

void BookViewerDialog::showTextFile(const QString &path) {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        showUnsupported(path, f.errorString());
        return;
    }
    const QByteArray data = f.readAll();
    textBrowser_->setPlainText(QString::fromUtf8(data));
    statusBrowser_->setHtml("<p>Rendered as text.</p>");
}

void BookViewerDialog::showPdfFile(const QString &path) {
    if (pdfDocument_->load(path) != QPdfDocument::Error::None) {
        showUnsupported(path, "Unable to load PDF.");
        return;
    }
    statusBrowser_->setHtml("<p>Rendered as PDF.</p>");
}

void BookViewerDialog::showImageFile(const QString &path) {
    QImageReader reader(path);
    QImage img = reader.read();
    if (img.isNull()) {
        showUnsupported(path, "Unable to load image.");
        return;
    }
    imageLabel_->setPixmap(QPixmap::fromImage(img).scaled(1000, 700, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    statusBrowser_->setHtml("<p>Rendered as image.</p>");
}

void BookViewerDialog::showUnsupported(const QString &path, const QString &reason) {
    statusBrowser_->setHtml(QString("<p><b>Unsupported or failed to render:</b> %1</p><p>%2</p>")
                            .arg(path.toHtmlEscaped(), reason.toHtmlEscaped()));
    textBrowser_->setPlainText(QString("Could not render: %1
%2").arg(path, reason));
}

void BookViewerDialog::loadCurrentFile() {
    const QString path = currentPath_.trimmed();
    if (path.isEmpty()) {
        showUnsupported("", "No file selected.");
        return;
    }

    const QFileInfo fi(path);
    if (!fi.exists()) {
        showUnsupported(path, "File does not exist.");
        return;
    }

    const auto kind = viewKindForPath(path);
    textBrowser_->clear();
    pdfDocument_->clear();
    imageLabel_->clear();
    imageLabel_->setText(QString());

    switch (kind) {
        case ViewKind::Text:
            showTextFile(path);
            break;
        case ViewKind::Pdf:
            showPdfFile(path);
            break;
        case ViewKind::Image:
            showImageFile(path);
            break;
        case ViewKind::External: {
            QString err;
            const QString pdf = convertIfPossible(path, "pdf", &err);
            if (!pdf.isEmpty()) {
                showPdfFile(pdf);
            } else {
                showUnsupported(path, err.isEmpty() ? "No native viewer available for this format." : err);
            }
            break;
        }
        case ViewKind::None:
            showUnsupported(path, "No viewer kind.");
            break;
    }
}

