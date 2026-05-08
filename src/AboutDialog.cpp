#include "AboutDialog.h"
#include <QApplication>
#include <QDialogButtonBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QTextBrowser>

AboutDialog::AboutDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("About mlibrary");
    resize(620, 420);

    auto *layout = new QVBoxLayout(this);
    auto *title = new QLabel("<h1>mlibrary</h1>", this);
    layout->addWidget(title);

    auto *info = new QTextBrowser(this);
    info->setHtml(R"HTML(
<p><b>Version:</b> 1.0.0</p>
<p><b>Owner / Author:</b> Sanjay Pandey</p>
<p><b>Organization:</b> Adya Infotech</p>
<p><b>License:</b> GPL v3.0</p>
<p><b>Framework:</b> Qt 6 Widgets</p>
<p><b>Supported backends:</b> PostgreSQL, MariaDB/MySQL, SQLite, Oracle, JSON Flatfile</p>
<p><b>Supported viewing formats:</b> txt, html, pdf, image formats, and best-effort conversion for other ebook formats</p>
<p><b>Website:</b> https://www.adyainfotech.com</p>
<p><b>Build:</b> Desktop app with Calibre-style layout and content server.</p>
)HTML");
    layout->addWidget(info);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
}
