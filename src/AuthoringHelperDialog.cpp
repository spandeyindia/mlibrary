#include "AuthoringHelperDialog.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLineEdit>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSettings>
#include <QTextBrowser>
#include <QVBoxLayout>

AuthoringHelperDialog::AuthoringHelperDialog(QWidget *parent) : QDialog(parent), nam_(new QNetworkAccessManager(this)) {
    setWindowTitle("AI Authoring Helper");
    resize(980, 760);

    auto *layout = new QVBoxLayout(this);
    auto *form = new QFormLayout();

    modeCombo_ = new QComboBox(this);
    modeCombo_->addItems({
        "Synopsis",
        "Back-cover copy",
        "Author bio",
        "Chapter outline",
        "Series pitch",
        "Improvement suggestions"
    });

    titleEdit_ = new QLineEdit(this);
    authorsEdit_ = new QLineEdit(this);
    subjectEdit_ = new QLineEdit(this);
    audienceEdit_ = new QLineEdit(this);
    toneEdit_ = new QLineEdit(this);
    keywordsEdit_ = new QLineEdit(this);

    form->addRow("Mode:", modeCombo_);
    form->addRow("Title:", titleEdit_);
    form->addRow("Authors:", authorsEdit_);
    form->addRow("Subject:", subjectEdit_);
    form->addRow("Audience:", audienceEdit_);
    form->addRow("Tone / style:", toneEdit_);
    form->addRow("Keywords:", keywordsEdit_);
    layout->addLayout(form);

    useRemoteCheck_ = new QCheckBox("Use remote AI endpoint when configured", this);
    endpointEdit_ = new QLineEdit(this);
    apiKeyEdit_ = new QLineEdit(this);
    apiKeyEdit_->setEchoMode(QLineEdit::Password);
    modelEdit_ = new QLineEdit(this);
    systemPromptEdit_ = new QPlainTextEdit(this);
    systemPromptEdit_->setPlaceholderText("System prompt / authoring instructions");

    auto *remoteForm = new QFormLayout();
    remoteForm->addRow("", useRemoteCheck_);
    remoteForm->addRow("Endpoint URL:", endpointEdit_);
    remoteForm->addRow("API key:", apiKeyEdit_);
    remoteForm->addRow("Model:", modelEdit_);
    remoteForm->addRow("System prompt:", systemPromptEdit_);

    layout->addLayout(remoteForm);

    output_ = new QPlainTextEdit(this);
    output_->setPlaceholderText("Generated output will appear here...");
    layout->addWidget(output_);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    auto *genBtn = new QPushButton("Generate", this);
    auto *copyBtn = new QPushButton("Copy Output", this);
    buttons->addButton(genBtn, QDialogButtonBox::ActionRole);
    buttons->addButton(copyBtn, QDialogButtonBox::ActionRole);
    connect(genBtn, &QPushButton::clicked, this, &AuthoringHelperDialog::generate);
    connect(copyBtn, &QPushButton::clicked, this, &AuthoringHelperDialog::copyOutput);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);

    QSettings s("mlibrary", "mlibrary");
    endpointEdit_->setText(s.value("ai/endpoint", "").toString());
    apiKeyEdit_->setText(s.value("ai/apiKey", "").toString());
    modelEdit_->setText(s.value("ai/model", "gpt-4.1-mini").toString());
    systemPromptEdit_->setPlainText(s.value("ai/systemPrompt", "You are a helpful ebook authoring assistant.").toString());
    useRemoteCheck_->setChecked(s.value("ai/useRemote", false).toBool());
}

void AuthoringHelperDialog::setBook(const Book &book) {
    book_ = book;
    titleEdit_->setText(book.title);
    authorsEdit_->setText(book.authors.join(", "));
    subjectEdit_->setText(book.subject);
    keywordsEdit_->setText(book.tags.join(", "));
}

QString AuthoringHelperDialog::buildPrompt() const {
    return QString(
        "You are an AI assistant helping an ebook authoring workflow.
"
        "Task: %1
"
        "Title: %2
"
        "Authors: %3
"
        "Subject: %4
"
        "Audience: %5
"
        "Tone: %6
"
        "Keywords: %7
"
        "Write a polished, concise, publication-ready answer."
    ).arg(modeCombo_->currentText(),
          titleEdit_->text(),
          authorsEdit_->text(),
          subjectEdit_->text(),
          audienceEdit_->text(),
          toneEdit_->text(),
          keywordsEdit_->text());
}

QString AuthoringHelperDialog::localFallback(const QString &mode) const {
    if (mode == "Synopsis") {
        return QString("Synopsis:
%1 is a compelling work in the %2 space, shaped for %3 readers. The book explores %4 with a tone that is %5.")
            .arg(titleEdit_->text(), subjectEdit_->text(), audienceEdit_->text(), keywordsEdit_->text(), toneEdit_->text());
    }
    if (mode == "Back-cover copy") {
        return QString("Back-cover copy:
Discover %1, a %2 title by %3. This edition is aimed at %4 readers and highlights %5.")
            .arg(titleEdit_->text(), subjectEdit_->text(), authorsEdit_->text(), audienceEdit_->text(), keywordsEdit_->text());
    }
    if (mode == "Author bio") {
        return QString("Author bio:
%1 is associated with this project and writes with a %2 voice for readers interested in %3.")
            .arg(authorsEdit_->text(), toneEdit_->text(), subjectEdit_->text());
    }
    if (mode == "Chapter outline") {
        return QString("Chapter outline:
1. Introduction
2. Core themes
3. Key examples
4. Practical takeaways
5. Closing summary");
    }
    if (mode == "Series pitch") {
        return QString("Series pitch:
A scalable series around %1, written for %2 readers with a %3 tone.")
            .arg(subjectEdit_->text(), audienceEdit_->text(), toneEdit_->text());
    }
    return QString("Improvement suggestions:
- Tighten the premise
- Clarify the audience
- Strengthen the chapter flow
- Expand the strongest scenes or arguments");
}

void AuthoringHelperDialog::sendToAiEndpoint(const QString &prompt) {
    QSettings s("mlibrary", "mlibrary");
    const QString endpoint = endpointEdit_->text().trimmed();
    if (endpoint.isEmpty()) {
        output_->setPlainText(localFallback(modeCombo_->currentText()));
        return;
    }

    QJsonObject req;
    req["model"] = modelEdit_->text().trimmed().isEmpty() ? "gpt-4.1-mini" : modelEdit_->text().trimmed();
    req["input"] = prompt;
    req["system"] = systemPromptEdit_->toPlainText();

    QNetworkRequest request(QUrl(endpoint));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    if (!apiKeyEdit_->text().trimmed().isEmpty()) {
        request.setRawHeader("Authorization", QByteArray("Bearer ") + apiKeyEdit_->text().trimmed().toUtf8());
    }

    auto *reply = nam_->post(request, QJsonDocument(req).toJson());
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        const QByteArray body = reply->readAll();
        const auto err = reply->error();
        reply->deleteLater();

        if (err != QNetworkReply::NoError) {
            output_->setPlainText(localFallback(modeCombo_->currentText()) + "

Remote endpoint failed; local fallback used.
");
            return;
        }

        const auto doc = QJsonDocument::fromJson(body);
        if (doc.isNull()) {
            output_->setPlainText(QString::fromUtf8(body));
            return;
        }

        QString text;
        const auto obj = doc.object();
        if (obj.contains("output_text")) {
            text = obj.value("output_text").toString();
        } else if (obj.contains("choices")) {
            const auto choices = obj.value("choices").toArray();
            if (!choices.isEmpty()) {
                const auto first = choices.first().toObject();
                if (first.contains("message")) {
                    text = first.value("message").toObject().value("content").toString();
                } else if (first.contains("text")) {
                    text = first.value("text").toString();
                }
            }
        }

        if (text.isEmpty()) text = QString::fromUtf8(body);
        output_->setPlainText(text);
    });
}

void AuthoringHelperDialog::generate() {
    const QString prompt = buildPrompt();
    if (useRemoteCheck_->isChecked() && !endpointEdit_->text().trimmed().isEmpty()) {
        sendToAiEndpoint(prompt);
    } else {
        output_->setPlainText(localFallback(modeCombo_->currentText()));
    }

    QSettings s("mlibrary", "mlibrary");
    s.setValue("ai/endpoint", endpointEdit_->text().trimmed());
    s.setValue("ai/apiKey", apiKeyEdit_->text().trimmed());
    s.setValue("ai/model", modelEdit_->text().trimmed());
    s.setValue("ai/systemPrompt", systemPromptEdit_->toPlainText());
    s.setValue("ai/useRemote", useRemoteCheck_->isChecked());
}

void AuthoringHelperDialog::copyOutput() {
    output_->selectAll();
    output_->copy();
}
