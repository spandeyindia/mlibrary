#include "WebServerDialog.h"
#include "WebServerService.h"
#include "LibraryData.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QSettings>
#include <QTextEdit>
#include <QVBoxLayout>

WebServerDialog::WebServerDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Web Content Server");
    resize(720, 460);

    auto *layout = new QVBoxLayout(this);
    auto *form = new QFormLayout();

    portEdit_ = new QLineEdit(this);
    portEdit_->setText("8088");
    adminEmailEdit_ = new QLineEdit(this);
    adminEmailEdit_->setPlaceholderText("admin@example.com");

    form->addRow("Port:", portEdit_);
    form->addRow("Admin email:", adminEmailEdit_);
    layout->addLayout(form);

    log_ = new QTextEdit(this);
    log_->setReadOnly(true);
    layout->addWidget(log_);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    startBtn_ = new QPushButton("Start", this);
    stopBtn_ = new QPushButton("Stop", this);
    buttons->addButton(startBtn_, QDialogButtonBox::ActionRole);
    buttons->addButton(stopBtn_, QDialogButtonBox::ActionRole);

    server_ = new WebServerService(this);
    loadSettings();
    connect(startBtn_, &QPushButton::clicked, this, &WebServerDialog::startServer);
    connect(stopBtn_, &QPushButton::clicked, this, &WebServerDialog::stopServer);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(server_, &WebServerService::logMessage, this, [this](const QString &msg) { log_->append(msg); });
    connect(server_, &WebServerService::approvalRequested, this, [this](const QString &msg) {
        log_->append(msg);
        if (!adminEmailEdit_->text().trimmed().isEmpty()) {
            log_->append(QString("Email notification queued for %1").arg(adminEmailEdit_->text().trimmed()));
        }
    });

    layout->addWidget(buttons);
}

void WebServerDialog::setLibraries(const QVector<LibraryData> *libraries) {
    libraries_ = libraries;
}

void WebServerDialog::loadSettings() {
    QSettings settings("mlibrary", "mlibrary");
    portEdit_->setText(QString::number(settings.value("webserver/port", 8088).toInt()));
    adminEmailEdit_->setText(settings.value("webserver/adminEmail", "").toString());
}

void WebServerDialog::saveSettings() {
    QSettings settings("mlibrary", "mlibrary");
    settings.setValue("webserver/port", portEdit_->text().toInt());
    settings.setValue("webserver/adminEmail", adminEmailEdit_->text().trimmed());
}

void WebServerDialog::startServer() {
    if (!libraries_) {
        log_->append("No library data connected.");
        return;
    }
    bool ok = false;
    const auto port = portEdit_->text().toUShort(&ok);
    if (!ok || port == 0) {
        log_->append("Please enter a valid port between 1 and 65535.");
        return;
    }

    QString error;
    if (server_->start(port, libraries_, adminEmailEdit_->text(), &error)) {
        log_->append(QString("Started at http://127.0.0.1:%1").arg(server_->port()));
        saveSettings();
    } else {
        log_->append("Failed to start server:");
        log_->append(error);
    }
}

void WebServerDialog::stopServer() {
    server_->stop();
    log_->append("Server stopped.");
}
