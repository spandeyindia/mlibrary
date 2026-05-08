#pragma once
#include <QObject>
#include <QMap>
#include <QJsonObject>
#include <QString>
#include <QPixmap>

class QNetworkAccessManager;

class MetadataService : public QObject {
    Q_OBJECT
public:
    explicit MetadataService(QObject *parent = nullptr);

    void lookupByIsbn(const QString &isbn);

signals:
    void lookupFinished(const QJsonObject &metadata, const QPixmap &cover, const QString &log, const QString &error);

private:
    QNetworkAccessManager *nam_;
};
