#pragma once
#include <QDialog>
class QLineEdit;
class QTextEdit;
class QSpinBox;

class WebsiteImportDialog : public QDialog {
    Q_OBJECT
public:
    explicit WebsiteImportDialog(QWidget *parent = nullptr);
private slots:
    void browseMirror();
    void browseOutputPdf();
    void runMirror();
private:
    QLineEdit *urlEdit_ = nullptr;
    QLineEdit *mirrorEdit_ = nullptr;
    QLineEdit *outputPdfEdit_ = nullptr;
    QLineEdit *wgetEdit_ = nullptr;
    QLineEdit *ebookConvertEdit_ = nullptr;
    QSpinBox *depthSpin_ = nullptr;
    QTextEdit *log_ = nullptr;
};
