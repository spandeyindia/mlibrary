#pragma once
#include <QString>

class QSqlDatabase;

enum class DbBackend {
    PostgreSQL,
    MariaDB,
    SQLite,
    Oracle,
    JsonFlatfile
};

class DatabaseManager {
public:
    static QString backendKey(DbBackend backend);
    static QString driverForBackend(DbBackend backend);
    static bool createSchema(QSqlDatabase &db, DbBackend backend, bool multiUserMode = false, QString *errorMessage = nullptr);
    static bool createJsonFlatfile(const QString &path, QString *errorMessage = nullptr);
};
