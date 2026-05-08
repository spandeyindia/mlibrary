#include "WebServerService.h"
#include "LibraryData.h"

#include <QTcpServer>
#include <QTcpSocket>
#include <QUrl>
#include <QUrlQuery>
#include <QFile>
#include <QFileInfo>
#include <QDateTime>
#include <QMap>
#include <QRegularExpression>
#include <QTextStream>

WebServerService::WebServerService(QObject *parent) : QObject(parent) {}

bool WebServerService::start(quint16 port, const QVector<LibraryData> *libraries, const QString &adminEmail, QString *error) {
    stop();
    libraries_ = libraries;
    adminEmail_ = adminEmail.trimmed();

    server_ = new QTcpServer(this);
    connect(server_, &QTcpServer::newConnection, this, &WebServerService::onNewConnection);

    if (!server_->listen(QHostAddress::LocalHost, port)) {
        if (error) *error = server_->errorString();
        server_->deleteLater();
        server_ = nullptr;
        return false;
    }

    port_ = server_->serverPort();
    emit logMessage(QString("Content server started on http://127.0.0.1:%1").arg(port_));
    return true;
}

void WebServerService::stop() {
    if (server_) {
        server_->close();
        server_->deleteLater();
        server_ = nullptr;
    }
    port_ = 0;
}

bool WebServerService::isRunning() const { return server_ && server_->isListening(); }
quint16 WebServerService::port() const { return port_; }

void WebServerService::onNewConnection() {
    while (server_ && server_->hasPendingConnections()) {
        QTcpSocket *socket = server_->nextPendingConnection();
        connect(socket, &QTcpSocket::readyRead, this, [this, socket]() {
            const QString request = QString::fromUtf8(socket->readAll());
            const QByteArray response = handleRequest(request);
            socket->write(response);
            socket->disconnectFromHost();
        });
        connect(socket, &QTcpSocket::disconnected, socket, &QObject::deleteLater);
    }
}

static QString httpMethod(const QString &requestText) {
    return requestText.section(' ', 0, 0).trimmed();
}

static QString httpTarget(const QString &requestText) {
    return requestText.section(' ', 1, 1).trimmed();
}

static QString httpBody(const QString &requestText) {
    const int pos = requestText.indexOf("

");
    if (pos < 0) return {};
    return requestText.mid(pos + 4);
}

QByteArray WebServerService::handleRequest(const QString &requestText) const {
    const QString method = httpMethod(requestText).toUpper();
    const QUrl url(httpTarget(requestText));
    const QString path = url.path();
    const QString body = httpBody(requestText);

    if (method == "GET" && (path == "/" || path.isEmpty())) {
        return httpResponse(200, "OK", renderHome().toUtf8());
    }

    if (method == "GET" && path.startsWith("/library/")) {
        const QString name = QUrl::fromPercentEncoding(path.mid(QString("/library/").size()).toUtf8());
        return httpResponse(200, "OK", renderLibrary(name).toUtf8());
    }

    if (method == "GET" && path.startsWith("/book/")) {
        bool ok = false;
        const int id = path.mid(QString("/book/").size()).toInt(&ok);
        if (ok) return httpResponse(200, "OK", renderBook(id).toUtf8());
    }

    if (method == "GET" && path.startsWith("/download/")) {
        bool ok = false;
        const int id = path.mid(QString("/download/").size()).toInt(&ok);
        if (ok) return renderDownload(id);
    }

    if (method == "GET" && path.startsWith("/upload-request")) {
        const auto lib = QUrlQuery(url).queryItemValue("library");
        return httpResponse(200, "OK", renderUploadForm(lib).toUtf8());
    }

    if (method == "POST" && path.startsWith("/upload-request")) {
        const auto form = parseForm(body);
        const QString library = requestValue(form, "library");
        const QString title = requestValue(form, "title");
        const QString authors = requestValue(form, "authors");
        const QString subject = requestValue(form, "subject");
        const QString bookType = requestValue(form, "book_type");
        const QString source = requestValue(form, "source");
        const QString summary = QString("Upload request: %1 | %2 | %3 | %4 | source=%5")
                                    .arg(title, authors, subject, bookType, source);
        queueApproval("upload", library, summary);
        return httpResponse(200, "OK", "<html><body><h1>Upload request queued for admin approval.</h1><p><a href="/">Home</a></p></body></html>" .toUtf8());
    }

    if (method == "GET" && path.startsWith("/edit-request/")) {
        bool ok = false;
        const int id = path.mid(QString("/edit-request/").size()).toInt(&ok);
        if (ok) return httpResponse(200, "OK", renderEditForm(QString::number(id)).toUtf8());
    }

    if (method == "POST" && path.startsWith("/edit-request/")) {
        const auto form = parseForm(body);
        const QString bookId = path.mid(QString("/edit-request/").size());
        const QString title = requestValue(form, "title");
        const QString authors = requestValue(form, "authors");
        const QString subject = requestValue(form, "subject");
        const QString bookType = requestValue(form, "book_type");
        const QString remarks = requestValue(form, "remarks");
        const QString summary = QString("Edit request for book %1: %2 | %3 | %4 | %5 | %6")
                                    .arg(bookId, title, authors, subject, bookType, remarks);
        queueApproval("edit", QString(), summary);
        return httpResponse(200, "OK", "<html><body><h1>Edit request queued for admin approval.</h1><p><a href="/">Home</a></p></body></html>" .toUtf8());
    }

    return httpResponse(404, "Not Found", "<html><body><h1>Not Found</h1></body></html>" .toUtf8());
}

QString WebServerService::htmlEscape(const QString &text) const {
    return text.toHtmlEscaped();
}

QString WebServerService::renderHome() const {
    QString html = "<html><head><title>mlibrary</title></head><body>";
    html += "<h1>mlibrary Content Server</h1>";
    html += "<p>Browse Library only. Download is allowed. Upload and metadata edits go to admin approval.</p>";
    html += "<ul>";
    if (libraries_) {
        for (const auto &lib : *libraries_) {
            html += "<li><a href="/library/" + QUrl::toPercentEncoding(lib.name) + "">" + htmlEscape(lib.name) + "</a>";
            html += " | <a href="/upload-request?library=" + QUrl::toPercentEncoding(lib.name) + "">Upload books</a>";
            html += "</li>";
        }
    }
    html += "</ul></body></html>";
    return html;
}

QString WebServerService::renderLibrary(const QString &name) const {
    QString html = "<html><head><title>" + htmlEscape(name) + "</title></head><body>";
    html += "<h1>" + htmlEscape(name) + "</h1>";
    html += "<p><a href="/">Back</a> | <a href="/upload-request?library=" + QUrl::toPercentEncoding(name) + "">Upload books</a></p>";
    html += "<ul>";
    if (libraries_) {
        for (const auto &lib : *libraries_) {
            if (lib.name == name) {
                for (const auto &book : lib.books) {
                    html += "<li><a href="/book/" + QString::number(book.id) + "">" + htmlEscape(book.title) + "</a>";
                    html += " — " + htmlEscape(book.authors.join(", "));
                    html += " — " + htmlEscape(book.formats.join(", "));
                    html += " — <a href="/download/" + QString::number(book.id) + "">Download</a>";
                    html += " — <a href="/edit-request/" + QString::number(book.id) + "">Request metadata edit</a>";
                    html += "</li>";
                }
            }
        }
    }
    html += "</ul></body></html>";
    return html;
}

const Book *WebServerService::findBookById(int id) const {
    if (!libraries_) return nullptr;
    for (const auto &lib : *libraries_) {
        for (const auto &book : lib.books) {
            if (book.id == id) return &book;
        }
    }
    return nullptr;
}

QString WebServerService::renderBook(int bookId) const {
    const Book *book = findBookById(bookId);
    if (!book) return "<html><body><h1>Book not found</h1></body></html>";

    QString html = "<html><head><title>" + htmlEscape(book->title) + "</title></head><body>";
    html += "<p><a href="/">Home</a></p>";
    html += "<h1>" + htmlEscape(book->title) + "</h1>";
    html += "<p><b>Authors:</b> " + htmlEscape(book->authors.join(", ")) + "</p>";
    html += "<p><b>Subject:</b> " + htmlEscape(book->subject) + "</p>";
    html += "<p><b>Book Type:</b> " + htmlEscape(book->bookType) + "</p>";
    html += "<p><b>Formats:</b> " + htmlEscape(book->formats.join(", ")) + "</p>";
    html += "<p><b>Published:</b> " + htmlEscape(book->publishedDate.isValid() ? book->publishedDate.toString(Qt::ISODate) : "—") + "</p>";
    html += "<p><b>Remarks:</b> " + htmlEscape(book->remarks.isEmpty() ? "—" : book->remarks) + "</p>";
    html += "<p><b>Storage path:</b> " + htmlEscape(book->storagePath) + "</p>";
    html += "<p><a href="/download/" + QString::number(book->id) + "">Download</a></p>";
    html += "<p><a href="/edit-request/" + QString::number(book->id) + "">Request metadata edit</a></p>";
    html += "</body></html>";
    return html;
}

QString WebServerService::renderUploadForm(const QString &libraryName) const {
    QString html = "<html><head><title>Upload request</title></head><body>";
    html += "<h1>Upload books for approval</h1>";
    html += "<p>The upload will be reviewed by admin before the book is added to the library.</p>";
    html += "<form method="post" action="/upload-request">";
    html += "<p>Library: <input name="library" value="" + htmlEscape(libraryName) + ""/></p>";
    html += "<p>Title: <input name="title"/></p>";
    html += "<p>Authors: <input name="authors"/></p>";
    html += "<p>Subject: <input name="subject"/></p>";
    html += "<p>Book Type: <input name="book_type"/></p>";
    html += "<p>Source (file path or description): <input name="source"/></p>";
    html += "<p><button type="submit">Submit upload request</button></p>";
    html += "</form>";
    html += "<p><a href="/">Home</a></p>";
    html += "</body></html>";
    return html;
}

QString WebServerService::renderEditForm(const QString &bookId) const {
    const Book *book = findBookById(bookId.toInt());
    QString html = "<html><head><title>Edit request</title></head><body>";
    html += "<h1>Request metadata edit</h1>";
    html += "<p>The edit will be reviewed by admin before it is applied.</p>";
    html += "<form method="post" action="/edit-request/" + htmlEscape(bookId) + "">";
    html += "<p>Title: <input name="title" value="" + htmlEscape(book ? book->title : QString()) + ""/></p>";
    html += "<p>Authors: <input name="authors" value="" + htmlEscape(book ? book->authors.join(", ") : QString()) + ""/></p>";
    html += "<p>Subject: <input name="subject" value="" + htmlEscape(book ? book->subject : QString()) + ""/></p>";
    html += "<p>Book Type: <input name="book_type" value="" + htmlEscape(book ? book->bookType : QString()) + ""/></p>";
    html += "<p>Remarks: <input name="remarks" value="" + htmlEscape(book ? book->remarks : QString()) + ""/></p>";
    html += "<p><button type="submit">Submit edit request</button></p>";
    html += "</form>";
    html += "<p><a href="/">Home</a></p>";
    html += "</body></html>";
    return html;
}

QByteArray WebServerService::renderDownload(int bookId) const {
    const Book *book = findBookById(bookId);
    if (!book) {
        return httpResponse(404, "Not Found", "<html><body><h1>Book not found</h1></body></html>" .toUtf8());
    }

    QFile file(book->storagePath);
    if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
        const QString body = "<html><body><h1>Download unavailable</h1><p>File does not exist on the server.</p></body></html>";
        return httpResponse(404, "Not Found", body.toUtf8());
    }

    const QByteArray data = file.readAll();
    const QString fileName = QFileInfo(book->storagePath).fileName();
    QByteArray response;
    response += "HTTP/1.1 200 OK
";
    response += "Content-Type: application/octet-stream
";
    response += "Content-Disposition: attachment; filename="" + fileName.toUtf8() + ""
";
    response += "Content-Length: " + QByteArray::number(data.size()) + "
";
    response += "Connection: close

";
    response += data;
    return response;
}

QString WebServerService::urlDecode(const QString &value) const {
    return QUrl::fromPercentEncoding(value.replace('+', ' ').toUtf8());
}

QMap<QString, QString> WebServerService::parseForm(const QString &body) const {
    QMap<QString, QString> out;
    for (const auto &pair : body.split('&', Qt::SkipEmptyParts)) {
        const int idx = pair.indexOf('=');
        if (idx <= 0) continue;
        const QString key = urlDecode(pair.left(idx));
        const QString val = urlDecode(pair.mid(idx + 1));
        out.insert(key, val);
    }
    return out;
}

QString WebServerService::requestValue(const QMap<QString, QString> &form, const QString &key) const {
    return form.value(key).trimmed();
}

void WebServerService::queueApproval(const QString &kind, const QString &libraryName, const QString &summary) {
    const QString stamp = QDateTime::currentDateTime().toString(Qt::ISODate);
    const QString msg = QString("[%1] %2 approval queued%3: %4")
                            .arg(stamp, kind,
                                 libraryName.isEmpty() ? QString() : QString(" for library '%1'").arg(libraryName),
                                 summary);
    emit approvalRequested(msg);
    emit logMessage(msg);
    if (!adminEmail_.isEmpty()) {
        emit logMessage(QString("Admin email notification requested for %1").arg(adminEmail_));
    } else {
        emit logMessage("Admin email notification is not configured.");
    }
}

QByteArray WebServerService::httpResponse(int statusCode, const QString &statusText, const QByteArray &body, const QString &contentType) const {
    QByteArray response;
    response += "HTTP/1.1 " + QByteArray::number(statusCode) + " " + statusText.toUtf8() + "
";
    response += "Content-Type: " + contentType.toUtf8() + "
";
    response += "Content-Length: " + QByteArray::number(body.size()) + "
";
    response += "Connection: close

";
    response += body;
    return response;
}
