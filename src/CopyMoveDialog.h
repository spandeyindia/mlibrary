#pragma once
#include <QDialog>
class QComboBox;
class QTextEdit;

class CopyMoveDialog : public QDialog {
    Q_OBJECT
public:
    explicit CopyMoveDialog(QWidget *parent = nullptr);

private slots:
    void runAction();

private:
    QComboBox *actionCombo_ = nullptr;
    QComboBox *libraryCombo_ = nullptr;
    QTextEdit *log_ = nullptr;
};
