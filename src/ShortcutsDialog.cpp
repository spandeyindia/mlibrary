#include "ShortcutsDialog.h"
#include <QDialogButtonBox>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

ShortcutsDialog::ShortcutsDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Keyboard Shortcuts");
    resize(680, 420);

    auto *layout = new QVBoxLayout(this);
    auto *table = new QTableWidget(0, 2, this);
    table->setHorizontalHeaderLabels({"Action", "Shortcut"});
    table->horizontalHeader()->setStretchLastSection(true);

    const std::pair<const char*, const char*> rows[] = {
        {"Open Setup DB", "Ctrl+D"},
        {"Add Library", "Ctrl+L"},
        {"Add Book", "Ctrl+B"},
        {"Edit Metadata", "Ctrl+E"},
        {"Bulk Edit", "Ctrl+Shift+E"},
        {"Download Metadata / Cover", "Ctrl+M"},
        {"Convert File", "Ctrl+Shift+C"},
        {"Start Web Server", "Ctrl+W"},
        {"Book Viewer", "Ctrl+V"},
        {"Preferences", "Ctrl+P"},
        {"About", "F1"},
    };

    table->setRowCount(int(std::size(rows)));
    for (int i = 0; i < int(std::size(rows)); ++i) {
        table->setItem(i, 0, new QTableWidgetItem(rows[i].first));
        table->setItem(i, 1, new QTableWidgetItem(rows[i].second));
    }
    layout->addWidget(table);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
}
