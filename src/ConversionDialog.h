#pragma once
#include <QDialog>
class QLineEdit;
class QTextEdit;
class QComboBox;

class ConversionDialog : public QDialog {
    Q_OBJECT
public:
    explicit ConversionDialog(QWidget *parent = nullptr);
private slots:
    void browseSource();
    void browseOutput();
    void runConversion();
private:
    QLineEdit *sourceEdit_ = nullptr;
    QLineEdit *outputEdit_ = nullptr;
    QLineEdit *ebookConvertEdit_ = nullptr;
    QComboBox *targetFormat_ = nullptr;
    QTextEdit *log_ = nullptr;
};
