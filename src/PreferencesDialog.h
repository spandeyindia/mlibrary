#pragma once
#include <QDialog>
#include "DatabaseManager.h"

class QLineEdit;
class QComboBox;
class QCheckBox;
class QTextEdit;
class QSpinBox;
class QTabWidget;

class PreferencesDialog : public QDialog {
    Q_OBJECT
public:
    explicit PreferencesDialog(QWidget *parent = nullptr);

private slots:
    void loadProfile();
    void saveProfile();
    void removeProfile();

private:
    DbBackend backendFromUi() const;
    void setBackend(DbBackend backend);
    void loadValuesFromSettings();
    void saveValuesToSettings();
    QString currentProfileName() const;

    QTabWidget *tabs_ = nullptr;

    QComboBox *profileCombo_ = nullptr;
    QLineEdit *profileNameEdit_ = nullptr;
    QComboBox *backendCombo_ = nullptr;
    QLineEdit *connEdit_ = nullptr;
    QLineEdit *sqlitePathEdit_ = nullptr;
    QLineEdit *jsonPathEdit_ = nullptr;
    QLineEdit *oraclePathEdit_ = nullptr;

    QLineEdit *smtpHostEdit_ = nullptr;
    QSpinBox *smtpPortSpin_ = nullptr;
    QLineEdit *smtpUserEdit_ = nullptr;
    QLineEdit *smtpPassEdit_ = nullptr;
    QLineEdit *smtpFromEdit_ = nullptr;

    QLineEdit *webUserEdit_ = nullptr;
    QLineEdit *webPassEdit_ = nullptr;
    QCheckBox *webAuthCheck_ = nullptr;
    QCheckBox *webHttpsCheck_ = nullptr;
    QLineEdit *webCertEdit_ = nullptr;
    QLineEdit *webKeyEdit_ = nullptr;
    QSpinBox *webPortSpin_ = nullptr;

    QLineEdit *ebookConvertEdit_ = nullptr;
    QLineEdit *wgetEdit_ = nullptr;

    QTextEdit *log_ = nullptr;
};
