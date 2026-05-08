#pragma once
#include <QString>
#include <QStringList>
#include <QDate>
#include <QDateTime>
#include <QVector>

struct Book {
    int id = 0;
    QString title;
    QString subtitle;
    QStringList authors;
    QString originalAuthor;
    QString originalLanguage;
    QString translatedBy;
    QString subject;
    QStringList extraSubjects;
    QStringList tags;
    QString series;
    QString seriesNumber;
    QString publisher;
    QString publisherCountry;
    QString bookType;
    QDate publishedDate;
    QDateTime addedDate;
    QString remarks;
    bool favorite = false;
    QString description;
    QStringList formats;
    QString storagePath;
    QString checksum;
    QString physicalLocation;
    QString physicalSection;
    QString physicalShelf;
    QString physicalStatus; // On shelf, Checked out, Reserved, In transit
    QString checkedOutTo;
    QString checkedOutContact;
    QDate checkedOutOn;
    QDate dueDate;
    QString accessionNumber;
    QString barcode;
    QString loanNotes;
    QString coverPath;
};

struct LibraryData {
    QString name;
    QString rootPath;
    QVector<Book> books;
};
