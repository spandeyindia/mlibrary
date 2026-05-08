#pragma once
#include <QDialog>
class QLineEdit;

class AddLibraryDialog : public QDialog {
    Q_OBJECT
public:
    explicit AddLibraryDialog(QWidget *parent = nullptr);
    QString libraryName() const;
    QString rootPath() const;
private:
    QLineEdit *nameEdit_ = nullptr;
    QLineEdit *rootEdit_ = nullptr;
};
