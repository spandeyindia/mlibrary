#include "WebServerDialog.h"
#include "WebServerService.h"
#include "LibraryData.h"

#include <QCheckBox>
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
    requireAuthCheck_ = new QCheckBox("Require basic auth", this);
    webUserEdit_ = new QLineEdit(this);
    webPassEdit_ = new QLineEdit(this);
    webPassEdit_->setEchoMode(QLineEdit::Password);
    httpsCheck_ = new QCheckBox("Enable HTTPS (settings only for now)", this);
    tlsCertEdit_ = new QLineEdit(this);
    tlsKeyEdit_ = new QLineEdit(this);

    form->addRow("Port:", portEdit_);
    form->addRow("Admin email:", adminEmailEdit_);
    form->addRow("", requireAuthCheck_);
    form->addRow("Web username:", webUserEdit_);
    form->addRow("Web password:", webPassEdit_);
    form->addRow("", httpsCheck_);
    form->addRow("TLS certificate path:", tlsCertEdit_);
    form->addRow("TLS key path:", tlsKeyEdit_);
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
    adminEmailEdit_->setText(settings.value("web/adminEmail", settings.value("webserver/adminEmail", "")).toString());
    const bool multiUserMode = settings.value("security/multiUserMode", false).toBool();
    requireAuthCheck_->setChecked(settings.value("web/authEnabled", multiUserMode).toBool());
    webUserEdit_->setText(settings.value("web/user", "").toString());
    webPassEdit_->setText(settings.value("web/pass", "").toString());
    httpsCheck_->setChecked(settings.value("web/https", false).toBool());
    tlsCertEdit_->setText(settings.value("web/cert", "").toString());
    tlsKeyEdit_->setText(settings.value("web/key", "").toString());
}

void WebServerDialog::saveSettings() {
    QSettings settings("mlibrary", "mlibrary");
    settings.setValue("webserver/port", portEdit_->text().toInt());
    settings.setValue("web/adminEmail", adminEmailEdit_->text().trimmed());
    settings.setValue("web/authEnabled", requireAuthCheck_->isChecked());
    settings.setValue("web/user", webUserEdit_->text().trimmed());
    settings.setValue("web/pass", webPassEdit_->text());
    settings.setValue("web/https", httpsCheck_->isChecked());
    settings.setValue("web/cert", tlsCertEdit_->text().trimmed());
    settings.setValue("web/key", tlsKeyEdit_->text().trimmed());
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
    if (server_->start(port, libraries_, adminEmailEdit_->text(), requireAuthCheck_->isChecked() ? webUserEdit_->text() : QString(), requireAuthCheck_->isChecked() ? webPassEdit_->text() : QString(), &error)) {
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
