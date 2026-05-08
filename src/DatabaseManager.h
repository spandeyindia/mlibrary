#pragma once
#include <QString>

class QSqlDatabase;

class DatabaseManager {
public:
    static bool createSchema(QSqlDatabase &db, QString *errorMessage = nullptr);
};
