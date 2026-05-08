#include "LicenseDialog.h"
#include <QDialogButtonBox>
#include <QTextBrowser>
#include <QVBoxLayout>

LicenseDialog::LicenseDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("GPL v3.0 License");
    resize(820, 640);

    auto *layout = new QVBoxLayout(this);
    auto *text = new QTextBrowser(this);
    text->setPlainText(R"(This project is distributed under the GNU General Public License v3.0.

For the full license text, include the standard GPL v3.0 LICENSE file in the repository root.
Recommended source header:
Copyright (C) 2026 Sanjay Pandey
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY.

For the canonical full text, see:
https://www.gnu.org/licenses/gpl-3.0.txt
)");
    layout->addWidget(text);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
}
