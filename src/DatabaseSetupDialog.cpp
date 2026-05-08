#include "DatabaseSetupDialog.h"
#include "DatabaseManager.h"

#include <QComboBox>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSettings>
#include <QTextEdit>
#include <QVBoxLayout>

DatabaseSetupDialog::DatabaseSetupDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Database Setup");
    resize(860, 560);

    auto *layout = new QVBoxLayout(this);
    auto *form = new QFormLayout();

    backendCombo_ = new QComboBox(this);
    backendCombo_->addItem("PostgreSQL", static_cast<int>(DbBackend::PostgreSQL));
    backendCombo_->addItem("MariaDB / MySQL", static_cast<int>(DbBackend::MariaDB));
    backendCombo_->addItem("SQLite", static_cast<int>(DbBackend::SQLite));
    backendCombo_->addItem("Oracle", static_cast<int>(DbBackend::Oracle));
    backendCombo_->addItem("JSON Flatfile", static_cast<int>(DbBackend::JsonFlatfile));

    connEdit_ = new QLineEdit(this);
    connEdit_->setPlaceholderText("host=localhost port=5432 dbname=ebook_library user=postgres password=postgres");

    sqlitePathEdit_ = new QLineEdit(this);
    sqlitePathEdit_->setPlaceholderText("/path/to/mlibrary.db");

    jsonPathEdit_ = new QLineEdit(this);
    jsonPathEdit_->setPlaceholderText("/path/to/mlibrary.json");

    adminPathEdit_ = new QLineEdit(this);
    adminPathEdit_->setPlaceholderText("Optional: TNS alias / service path for Oracle");
    multiUserCheck_ = new QCheckBox("Enable users, roles, and authentication", this);

    form->addRow("Backend:", backendCombo_);
    form->addRow("", multiUserCheck_);
    form->addRow("Connection string:", connEdit_);
    form->addRow("SQLite file path:", sqlitePathEdit_);
    form->addRow("JSON flatfile path:", jsonPathEdit_);
    form->addRow("Oracle service/TNS path:", adminPathEdit_);
    layout->addLayout(form);

    output_ = new QTextEdit(this);
    output_->setReadOnly(true);
    layout->addWidget(output_);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    auto *createBtn = new QPushButton("Create Tables", this);
    buttons->addButton(createBtn, QDialogButtonBox::ActionRole);

    QSettings s("mlibrary", "mlibrary");
    const int b = s.value("db/backend", int(DbBackend::PostgreSQL)).toInt();
    const int idx = backendCombo_->findData(b);
    if (idx >= 0) backendCombo_->setCurrentIndex(idx);
    connEdit_->setText(s.value("db/conn", "host=localhost port=5432 dbname=ebook_library user=postgres password=postgres").toString());
    sqlitePathEdit_->setText(s.value("db/sqlite", "").toString());
    jsonPathEdit_->setText(s.value("db/json", "").toString());
    adminPathEdit_->setText(s.value("db/oracle", "").toString());
    multiUserCheck_->setChecked(s.value("security/multiUserMode", false).toBool());

    connect(createBtn, &QPushButton::clicked, this, &DatabaseSetupDialog::createTables);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
}

QString DatabaseSetupDialog::resultText() const { return resultText_; }

DbBackend DatabaseSetupDialog::selectedBackend() const {
    return static_cast<DbBackend>(backendCombo_->currentData().toInt());
}

QString DatabaseSetupDialog::connectionString() const { return connEdit_->text().trimmed(); }
QString DatabaseSetupDialog::sqlitePath() const { return sqlitePathEdit_->text().trimmed(); }
QString DatabaseSetupDialog::jsonPath() const { return jsonPathEdit_->text().trimmed(); }

void DatabaseSetupDialog::createTables() {
    const DbBackend backend = selectedBackend();
    if (backend == DbBackend::JsonFlatfile) {
        QString error;
        if (DatabaseManager::createJsonFlatfile(jsonPath(), &error)) {
            QSettings s("mlibrary", "mlibrary");
            s.setValue("security/multiUserMode", multiUserCheck_->isChecked());
            resultText_ = "JSON flatfile created or refreshed successfully.";
            output_->append(resultText_);
        } else {
            resultText_ = error;
            output_->append("JSON flatfile creation failed:");
            output_->append(error);
        }
        return;
    }

    const QString driver = DatabaseManager::driverForBackend(backend);
    if (driver.isEmpty()) {
        resultText_ = "No SQL driver available for selected backend.";
        output_->append(resultText_);
        return;
    }

    if (QSqlDatabase::contains("ebook_setup_connection")) {
        QSqlDatabase::removeDatabase("ebook_setup_connection");
    }

    QSqlDatabase db = QSqlDatabase::addDatabase(driver, "ebook_setup_connection");

    if (backend == DbBackend::SQLite) {
        if (sqlitePath().isEmpty()) {
            resultText_ = "SQLite path is required.";
            output_->append(resultText_);
            return;
        }
        db.setDatabaseName(sqlitePath());
    } else {
        const auto parts = connectionString().split(' ', Qt::SkipEmptyParts);
        for (const auto &p : parts) {
            const auto idx = p.indexOf('=');
            if (idx <= 0) continue;
            const auto key = p.left(idx);
            const auto value = p.mid(idx + 1);
            if (key == "host") db.setHostName(value);
            else if (key == "port") db.setPort(value.toInt());
            else if (key == "dbname") db.setDatabaseName(value);
            else if (key == "user") db.setUserName(value);
            else if (key == "password") db.setPassword(value);
            else if (key == "service") db.setDatabaseName(value);
        }
        if (backend == DbBackend::Oracle && !adminPathEdit_->text().trimmed().isEmpty()) {
            db.setDatabaseName(adminPathEdit_->text().trimmed());
        }
    }

    if (!db.open()) {
        resultText_ = db.lastError().text();
        output_->append("Connection failed:");
        output_->append(resultText_);
        return;
    }

    QString error;
    if (DatabaseManager::createSchema(db, backend, multiUserCheck_->isChecked(), &error)) {
        resultText_ = "Database tables created or already existed successfully.";
        output_->append(resultText_);
        QSettings s("mlibrary", "mlibrary");
        s.setValue("security/multiUserMode", multiUserCheck_->isChecked());
    } else {
        resultText_ = error;
        output_->append("Database setup failed:");
        output_->append(error);
    }
}
