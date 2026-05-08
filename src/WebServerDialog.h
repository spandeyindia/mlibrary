#pragma once
#include <QDialog>
#include <QVector>
#include "LibraryData.h"

class QLineEdit;
class QTextEdit;
class QPushButton;
class WebServerService;

class WebServerDialog : public QDialog {
    Q_OBJECT
public:
    explicit WebServerDialog(QWidget *parent = nullptr);
    void setLibraries(const QVector<LibraryData> *libraries);

private slots:
    void startServer();
    void stopServer();

private:
    void loadSettings();
    void saveSettings();

    WebServerService *server_ = nullptr;
    const QVector<LibraryData> *libraries_ = nullptr;
    QLineEdit *portEdit_ = nullptr;
    QLineEdit *adminEmailEdit_ = nullptr;
    QTextEdit *log_ = nullptr;
    QPushButton *startBtn_ = nullptr;
    QPushButton *stopBtn_ = nullptr;
};
