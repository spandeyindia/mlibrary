#pragma once
#include <QDialog>
#include "DatabaseManager.h"

class QLineEdit;
class QTextEdit;
class QComboBox;
class QCheckBox;
class QSettings;

class DatabaseSetupDialog : public QDialog {
    Q_OBJECT
public:
    explicit DatabaseSetupDialog(QWidget *parent = nullptr);
    QString resultText() const;

private slots:
    void createTables();

private:
    DbBackend selectedBackend() const;
    QString connectionString() const;
    QString sqlitePath() const;
    QString jsonPath() const;

    QComboBox *backendCombo_ = nullptr;
    QCheckBox *multiUserCheck_ = nullptr;
    QLineEdit *connEdit_ = nullptr;
    QLineEdit *sqlitePathEdit_ = nullptr;
    QLineEdit *jsonPathEdit_ = nullptr;
    QLineEdit *adminPathEdit_ = nullptr;
    QTextEdit *output_ = nullptr;
    QString resultText_;
};
