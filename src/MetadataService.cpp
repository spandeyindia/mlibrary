#include "MetadataService.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QPixmap>
#include <QImage>
#include <QBuffer>

MetadataService::MetadataService(QObject *parent) : QObject(parent), nam_(new QNetworkAccessManager(this)) {}

void MetadataService::lookupByIsbn(const QString &isbn) {
    const QString clean = isbn.trimmed();
    if (clean.isEmpty()) {
        emit lookupFinished({}, QPixmap(), {}, "ISBN is empty.");
        return;
    }

    // Open Library Book API: ISBN metadata lookup
    // Example endpoint: https://openlibrary.org/api/books?bibkeys=ISBN:978...&format=json&jscmd=data
    QUrl url("https://openlibrary.org/api/books");
    QUrlQuery q;
    q.addQueryItem("bibkeys", QString("ISBN:%1").arg(clean));
    q.addQueryItem("format", "json");
    q.addQueryItem("jscmd", "data");
    url.setQuery(q);

    auto *reply = nam_->get(QNetworkRequest(url));
    connect(reply, &QNetworkReply::finished, this, [this, reply, clean]() {
        QString log;
        QString error;

        const auto bytes = reply->readAll();
        reply->deleteLater();

        QJsonObject metadata;
        QPixmap cover;

        if (reply->error() != QNetworkReply::NoError) {
            error = reply->errorString();
            emit lookupFinished(metadata, cover, log, error);
            return;
        }

        const auto doc = QJsonDocument::fromJson(bytes);
        const auto root = doc.object();
        const auto key = QString("ISBN:%1").arg(clean);
        const auto obj = root.value(key).toObject();
        if (obj.isEmpty()) {
            error = "No metadata found in Open Library response.";
            emit lookupFinished(metadata, cover, log, error);
            return;
        }

        metadata["title"] = obj.value("title").toString();
        metadata["subtitle"] = obj.value("subtitle").toString();
        metadata["published_date"] = obj.value("publish_date").toString();
        metadata["publisher"] = obj.value("publishers").toArray().isEmpty() ? "" : obj.value("publishers").toArray().first().toObject().value("name").toString();
        metadata["authors"] = QStringList();
        const auto authors = obj.value("authors").toArray();
        QStringList authorNames;
        for (const auto &a : authors) authorNames << a.toObject().value("name").toString();
        metadata["authors"] = authorNames.join(", ");
        metadata["isbn"] = clean;
        metadata["source"] = "openlibrary";

        // Open Library cover API by ISBN
        // Covers API supports ISBN or Open Library IDs.
        QUrl coverUrl(QString("https://covers.openlibrary.org/b/isbn/%1-L.jpg").arg(clean));
        auto *coverReply = nam_->get(QNetworkRequest(coverUrl));
        connect(coverReply, &QNetworkReply::finished, this, [this, coverReply, metadata, log]() mutable {
            QPixmap cover;
            QString error;
            const auto bytes = coverReply->readAll();
            coverReply->deleteLater();
            if (coverReply->error() == QNetworkReply::NoError && !bytes.isEmpty()) {
                cover.loadFromData(bytes);
            } else {
                error = "Metadata found, but cover download failed or is unavailable.";
            }
            emit lookupFinished(metadata, cover, log, error);
        });
    });
}
