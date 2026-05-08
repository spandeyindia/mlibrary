#include "PreferencesDialog.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QSettings>
#include <QSpinBox>
#include <QTabWidget>
#include <QTextEdit>
#include <QVBoxLayout>

PreferencesDialog::PreferencesDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Preferences");
    resize(920, 700);

    auto *layout = new QVBoxLayout(this);
    tabs_ = new QTabWidget(this);
    layout->addWidget(tabs_);

    QWidget *dbTab = new QWidget(this);
    auto *dbLayout = new QVBoxLayout(dbTab);
    auto *dbForm = new QFormLayout();

    profileCombo_ = new QComboBox(this);
    profileNameEdit_ = new QLineEdit(this);
    backendCombo_ = new QComboBox(this);
    backendCombo_->addItem("PostgreSQL", int(DbBackend::PostgreSQL));
    backendCombo_->addItem("MariaDB / MySQL", int(DbBackend::MariaDB));
    backendCombo_->addItem("SQLite", int(DbBackend::SQLite));
    backendCombo_->addItem("Oracle", int(DbBackend::Oracle));
    backendCombo_->addItem("JSON Flatfile", int(DbBackend::JsonFlatfile));

    connEdit_ = new QLineEdit(this);
    sqlitePathEdit_ = new QLineEdit(this);
    jsonPathEdit_ = new QLineEdit(this);
    oraclePathEdit_ = new QLineEdit(this);

    dbForm->addRow("Profile:", profileCombo_);
    dbForm->addRow("Profile name:", profileNameEdit_);
    dbForm->addRow("Backend:", backendCombo_);
    dbForm->addRow("Connection string:", connEdit_);
    dbForm->addRow("SQLite path:", sqlitePathEdit_);
    dbForm->addRow("JSON flatfile path:", jsonPathEdit_);
    dbForm->addRow("Oracle service / TNS:", oraclePathEdit_);
    dbLayout->addLayout(dbForm);

    auto *dbButtons = new QHBoxLayout();
    auto *loadBtn = new QPushButton("Load Profile", this);
    auto *saveBtn = new QPushButton("Save Profile", this);
    auto *removeBtn = new QPushButton("Delete Profile", this);
    dbButtons->addWidget(loadBtn);
    dbButtons->addWidget(saveBtn);
    dbButtons->addWidget(removeBtn);
    dbButtons->addStretch();
    dbLayout->addLayout(dbButtons);

    tabs_->addTab(dbTab, "Database");

    QWidget *emailTab = new QWidget(this);
    auto *emailLayout = new QVBoxLayout(emailTab);
    auto *emailForm = new QFormLayout();
    smtpHostEdit_ = new QLineEdit(this);
    smtpPortSpin_ = new QSpinBox(this);
    smtpPortSpin_->setRange(1, 65535);
    smtpPortSpin_->setValue(587);
    smtpUserEdit_ = new QLineEdit(this);
    smtpPassEdit_ = new QLineEdit(this);
    smtpPassEdit_->setEchoMode(QLineEdit::Password);
    smtpFromEdit_ = new QLineEdit(this);
    emailForm->addRow("SMTP host:", smtpHostEdit_);
    emailForm->addRow("SMTP port:", smtpPortSpin_);
    emailForm->addRow("SMTP user:", smtpUserEdit_);
    emailForm->addRow("SMTP password:", smtpPassEdit_);
    emailForm->addRow("From address:", smtpFromEdit_);
    emailLayout->addLayout(emailForm);
    tabs_->addTab(emailTab, "Email");

    QWidget *webTab = new QWidget(this);
    auto *webLayout = new QVBoxLayout(webTab);
    auto *webForm = new QFormLayout();
    webAuthCheck_ = new QCheckBox("Require basic auth", this);
    webUserEdit_ = new QLineEdit(this);
    webPassEdit_ = new QLineEdit(this);
    webPassEdit_->setEchoMode(QLineEdit::Password);
    webHttpsCheck_ = new QCheckBox("Enable HTTPS (settings only; TLS wiring can be added later)", this);
    webPortSpin_ = new QSpinBox(this);
    webPortSpin_->setRange(1, 65535);
    webPortSpin_->setValue(8088);
    webCertEdit_ = new QLineEdit(this);
    webKeyEdit_ = new QLineEdit(this);
    webForm->addRow("Web server port:", webPortSpin_);
    webForm->addRow("", webAuthCheck_);
    webForm->addRow("Web username:", webUserEdit_);
    webForm->addRow("Web password:", webPassEdit_);
    webForm->addRow("", webHttpsCheck_);
    webForm->addRow("TLS certificate path:", webCertEdit_);
    webForm->addRow("TLS key path:", webKeyEdit_);
    webLayout->addLayout(webForm);
    tabs_->addTab(webTab, "Web Server");

    QWidget *securityTab = new QWidget(this);
    auto *secLayout = new QVBoxLayout(securityTab);
    auto *secInfo = new QTextEdit(this);
    secInfo->setReadOnly(true);
    secInfo->setHtml("<p><b>Security mode is chosen in Setup DB.</b></p><p>Single-user mode disables auth and keeps the app private to one laptop user. Multi-user mode enables users, roles, sessions, and audit tables.</p>");
    secLayout->addWidget(secInfo);
    tabs_->addTab(securityTab, "Security");

    QWidget *pathsTab = new QWidget(this);
    auto *pathsLayout = new QVBoxLayout(pathsTab);
    auto *pathsForm = new QFormLayout();
    ebookConvertEdit_ = new QLineEdit(this);
    wgetEdit_ = new QLineEdit(this);
    pathsForm->addRow("ebook-convert path:", ebookConvertEdit_);
    pathsForm->addRow("wget path:", wgetEdit_);
    pathsLayout->addLayout(pathsForm);
    tabs_->addTab(pathsTab, "Paths");

    log_ = new QTextEdit(this);
    log_->setReadOnly(true);
    layout->addWidget(log_);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);

    connect(loadBtn, &QPushButton::clicked, this, &PreferencesDialog::loadProfile);
    connect(saveBtn, &QPushButton::clicked, this, &PreferencesDialog::saveProfile);
    connect(removeBtn, &QPushButton::clicked, this, &PreferencesDialog::removeProfile);
    connect(profileCombo_, &QComboBox::currentIndexChanged, this, &PreferencesDialog::loadProfile);

    loadValuesFromSettings();
}

DbBackend PreferencesDialog::backendFromUi() const {
    return static_cast<DbBackend>(backendCombo_->currentData().toInt());
}

void PreferencesDialog::setBackend(DbBackend backend) {
    for (int i = 0; i < backendCombo_->count(); ++i) {
        if (static_cast<DbBackend>(backendCombo_->itemData(i).toInt()) == backend) {
            backendCombo_->setCurrentIndex(i);
            return;
        }
    }
}

QString PreferencesDialog::currentProfileName() const {
    const QString name = profileNameEdit_->text().trimmed();
    return name.isEmpty() ? "Default" : name;
}

void PreferencesDialog::loadValuesFromSettings() {
    QSettings s("mlibrary", "mlibrary");
    const QString active = s.value("db/activeProfile", "Default").toString();

    profileCombo_->clear();
    const int size = s.beginReadArray("dbProfiles");
    QString activeText = active;
    for (int i = 0; i < size; ++i) {
        s.setArrayIndex(i);
        const QString name = s.value("name", "Default").toString();
        profileCombo_->addItem(name);
    }
    s.endArray();
    if (profileCombo_->count() == 0) {
        profileCombo_->addItem("Default");
    }

    int idx = profileCombo_->findText(activeText);
    if (idx < 0) idx = 0;
    profileCombo_->setCurrentIndex(idx);
    loadProfile();
}

void PreferencesDialog::saveValuesToSettings() {
    QSettings s("mlibrary", "mlibrary");
    const QString profileName = currentProfileName();
    const QString active = profileName;
    const QString oldName = profileCombo_->currentText();

    int size = s.beginReadArray("dbProfiles");
    QVector<QVariantMap> profiles;
    profiles.reserve(size);
    for (int i = 0; i < size; ++i) {
        s.setArrayIndex(i);
        QVariantMap map;
        map["name"] = s.value("name", "Default").toString();
        map["backend"] = s.value("backend", int(DbBackend::PostgreSQL)).toInt();
        map["conn"] = s.value("conn", "").toString();
        map["sqlite"] = s.value("sqlite", "").toString();
        map["json"] = s.value("json", "").toString();
        map["oracle"] = s.value("oracle", "").toString();
        profiles.push_back(map);
    }
    s.endArray();

    bool replaced = false;
    for (auto &m : profiles) {
        if (m["name"].toString() == oldName) {
            m["name"] = profileName;
            m["backend"] = backendCombo_->currentData().toInt();
            m["conn"] = connEdit_->text().trimmed();
            m["sqlite"] = sqlitePathEdit_->text().trimmed();
            m["json"] = jsonPathEdit_->text().trimmed();
            m["oracle"] = oraclePathEdit_->text().trimmed();
            replaced = true;
            break;
        }
    }
    if (!replaced) {
        QVariantMap m;
        m["name"] = profileName;
        m["backend"] = backendCombo_->currentData().toInt();
        m["conn"] = connEdit_->text().trimmed();
        m["sqlite"] = sqlitePathEdit_->text().trimmed();
        m["json"] = jsonPathEdit_->text().trimmed();
        m["oracle"] = oraclePathEdit_->text().trimmed();
        profiles.push_back(m);
    }

    s.beginWriteArray("dbProfiles");
    for (int i = 0; i < profiles.size(); ++i) {
        s.setArrayIndex(i);
        s.setValue("name", profiles[i]["name"]);
        s.setValue("backend", profiles[i]["backend"]);
        s.setValue("conn", profiles[i]["conn"]);
        s.setValue("sqlite", profiles[i]["sqlite"]);
        s.setValue("oracle", profiles[i]["oracle"]);
    }
    s.endArray();
    s.setValue("db/activeProfile", profileName);
    s.setValue("db/backend", backendCombo_->currentData().toInt());
    s.setValue("db/conn", connEdit_->text().trimmed());
    s.setValue("db/sqlite", sqlitePathEdit_->text().trimmed());
    s.setValue("db/json", jsonPathEdit_->text().trimmed());
    s.setValue("db/oracle", oraclePathEdit_->text().trimmed());

    s.setValue("smtp/host", smtpHostEdit_->text().trimmed());
    s.setValue("smtp/port", smtpPortSpin_->value());
    s.setValue("smtp/user", smtpUserEdit_->text().trimmed());
    s.setValue("smtp/pass", smtpPassEdit_->text());
    s.setValue("smtp/from", smtpFromEdit_->text().trimmed());

    s.setValue("web/port", webPortSpin_->value());
    s.setValue("web/authEnabled", webAuthCheck_->isChecked());
    s.setValue("web/user", webUserEdit_->text().trimmed());
    s.setValue("web/pass", webPassEdit_->text());
    s.setValue("web/https", webHttpsCheck_->isChecked());
    s.setValue("web/cert", webCertEdit_->text().trimmed());
    s.setValue("web/key", webKeyEdit_->text().trimmed());

    s.setValue("paths/ebookConvert", ebookConvertEdit_->text().trimmed());
    s.setValue("paths/wget", wgetEdit_->text().trimmed());

    profileCombo_->clear();
    for (const auto &m : profiles) profileCombo_->addItem(m["name"].toString());
    const int idx = profileCombo_->findText(profileName);
    profileCombo_->setCurrentIndex(idx >= 0 ? idx : 0);
}

void PreferencesDialog::loadProfile() {
    QSettings s("mlibrary", "mlibrary");
    const QString name = profileCombo_->currentText().isEmpty() ? "Default" : profileCombo_->currentText();
    profileNameEdit_->setText(name);

    int size = s.beginReadArray("dbProfiles");
    QVariantMap found;
    for (int i = 0; i < size; ++i) {
        s.setArrayIndex(i);
        if (s.value("name").toString() == name) {
            found["backend"] = s.value("backend", int(DbBackend::PostgreSQL)).toInt();
            found["conn"] = s.value("conn", "").toString();
            found["sqlite"] = s.value("sqlite", "").toString();
            found["oracle"] = s.value("oracle", "").toString();
            break;
        }
    }
    s.endArray();

    if (found.isEmpty()) {
        found["backend"] = s.value("db/backend", int(DbBackend::PostgreSQL)).toInt();
        found["conn"] = s.value("db/conn", "host=localhost port=5432 dbname=ebook_library user=postgres password=postgres").toString();
        found["sqlite"] = s.value("db/sqlite", "").toString();
        found["json"] = s.value("db/json", "").toString();
        found["oracle"] = s.value("db/oracle", "").toString();
    }

    setBackend(static_cast<DbBackend>(found["backend"].toInt()));
    connEdit_->setText(found["conn"].toString());
    sqlitePathEdit_->setText(found["sqlite"].toString());
    jsonPathEdit_->setText(found["json"].toString());
    oraclePathEdit_->setText(found["oracle"].toString());

    smtpHostEdit_->setText(s.value("smtp/host", "").toString());
    smtpPortSpin_->setValue(s.value("smtp/port", 587).toInt());
    smtpUserEdit_->setText(s.value("smtp/user", "").toString());
    smtpPassEdit_->setText(s.value("smtp/pass", "").toString());
    smtpFromEdit_->setText(s.value("smtp/from", "").toString());

    webPortSpin_->setValue(s.value("web/port", 8088).toInt());
    webAuthCheck_->setChecked(s.value("web/authEnabled", false).toBool());
    webUserEdit_->setText(s.value("web/user", "").toString());
    webPassEdit_->setText(s.value("web/pass", "").toString());
    webHttpsCheck_->setChecked(s.value("web/https", false).toBool());
    webCertEdit_->setText(s.value("web/cert", "").toString());
    webKeyEdit_->setText(s.value("web/key", "").toString());

    ebookConvertEdit_->setText(s.value("paths/ebookConvert", "").toString());
    wgetEdit_->setText(s.value("paths/wget", "").toString());
}

void PreferencesDialog::saveProfile() {
    saveValuesToSettings();
    log_->append("Preferences saved.");
}

void PreferencesDialog::removeProfile() {
    QSettings s("mlibrary", "mlibrary");
    const QString name = profileCombo_->currentText();
    int size = s.beginReadArray("dbProfiles");
    QVector<QVariantMap> profiles;
    for (int i = 0; i < size; ++i) {
        s.setArrayIndex(i);
        if (s.value("name").toString() == name) continue;
        QVariantMap m;
        m["name"] = s.value("name");
        m["backend"] = s.value("backend");
        m["conn"] = s.value("conn");
        m["sqlite"] = s.value("sqlite");
        m["oracle"] = s.value("oracle");
        profiles.push_back(m);
    }
    s.endArray();
    s.remove("dbProfiles");
    s.beginWriteArray("dbProfiles");
    for (int i = 0; i < profiles.size(); ++i) {
        s.setArrayIndex(i);
        s.setValue("name", profiles[i]["name"]);
        s.setValue("backend", profiles[i]["backend"]);
        s.setValue("conn", profiles[i]["conn"]);
        s.setValue("sqlite", profiles[i]["sqlite"]);
        s.setValue("oracle", profiles[i]["oracle"]);
    }
    s.endArray();
    profileCombo_->removeItem(profileCombo_->currentIndex());
    log_->append(QString("Profile '%1' removed.").arg(name));
}
