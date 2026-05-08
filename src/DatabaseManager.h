#pragma once
#include <QString>

class QSqlDatabase;

enum class DbBackend {
    PostgreSQL,
    MariaDB,
    SQLite,
    Oracle
};

class DatabaseManager {
public:
    static QString backendKey(DbBackend backend);
    static QString driverForBackend(DbBackend backend);
    static QString schemaSql(DbBackend backend);
    static bool createSchema(QSqlDatabase &db, DbBackend backend, QString *errorMessage = nullptr);
};
