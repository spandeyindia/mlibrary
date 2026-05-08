#pragma once
#include <QObject>
#include <QString>
#include <QVector>
#include <QMap>
#include <QByteArray>

class QTcpServer;
class LibraryData;

class WebServerService : public QObject {
    Q_OBJECT
public:
    explicit WebServerService(QObject *parent = nullptr);

    bool start(quint16 port, const QVector<LibraryData> *libraries, const QString &adminEmail, const QString &authUser, const QString &authPass, QString *error = nullptr);
    void stop();
    bool isRunning() const;
    quint16 port() const;

signals:
    void logMessage(const QString &message);
    void approvalRequested(const QString &message);

private slots:
    void onNewConnection();

private:
    QByteArray handleRequest(const QString &requestText) const;
    QByteArray httpResponse(int statusCode, const QString &statusText, const QByteArray &body, const QString &contentType = "text/html; charset=utf-8") const;

    QString htmlEscape(const QString &text) const;
    QString renderHome() const;
    QString renderLibrary(const QString &name) const;
    QString renderBook(int bookId) const;
    QString renderUploadForm(const QString &libraryName) const;
    QString renderEditForm(const QString &bookId) const;
    QByteArray renderDownload(int bookId) const;

    QString urlDecode(const QString &value) const;
    QMap<QString, QString> parseForm(const QString &body) const;
    QString requestValue(const QMap<QString, QString> &form, const QString &key) const;

    const class Book *findBookById(int id) const;
    void queueApproval(const QString &kind, const QString &libraryName, const QString &summary);

    QTcpServer *server_ = nullptr;
    quint16 port_ = 0;
    const QVector<LibraryData> *libraries_ = nullptr;
    QString adminEmail_;
    QString authUser_;
    QString authPass_;
};
