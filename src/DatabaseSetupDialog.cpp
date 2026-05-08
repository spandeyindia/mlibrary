#include "DatabaseSetupDialog.h"
#include "DatabaseManager.h"
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QSqlDatabase>
#include <QSqlError>
#include <QTextEdit>
#include <QVBoxLayout>

DatabaseSetupDialog::DatabaseSetupDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Database Setup");
    auto *layout = new QVBoxLayout(this);
    auto *form = new QFormLayout();
    connEdit_ = new QLineEdit(this);
    connEdit_->setText("host=localhost port=5432 dbname=ebook_library user=postgres password=postgres");
    form->addRow("Connection string:", connEdit_);
    layout->addLayout(form);
    output_ = new QTextEdit(this);
    output_->setReadOnly(true);
    layout->addWidget(output_);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    auto *createBtn = new QPushButton("Create Tables", this);
    buttons->addButton(createBtn, QDialogButtonBox::ActionRole);
    connect(createBtn, &QPushButton::clicked, this, &DatabaseSetupDialog::createTables);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
}
QString DatabaseSetupDialog::resultText() const { return resultText_; }
QString DatabaseSetupDialog::connectionString() const { return connEdit_->text().trimmed(); }

void DatabaseSetupDialog::createTables() {
    if (QSqlDatabase::contains("ebook_setup_connection")) {
        QSqlDatabase::removeDatabase("ebook_setup_connection");
    }
    QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL", "ebook_setup_connection");
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
    }
    if (!db.open()) {
        resultText_ = db.lastError().text();
        output_->append("Connection failed:");
        output_->append(resultText_);
        return;
    }
    QString error;
    if (DatabaseManager::createSchema(db, &error)) {
        resultText_ = "Database tables created or already existed successfully.";
        output_->append(resultText_);
    } else {
        resultText_ = error;
        output_->append("Database setup failed:");
        output_->append(error);
    }
}
