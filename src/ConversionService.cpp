#include "ConversionService.h"
#include <QDir>
#include <QDirIterator>
#include <QProcess>
#include <QUrl>

QString ConversionService::defaultEbookConvertPath() {
#ifdef Q_OS_MACOS
    return "/Applications/calibre.app/Contents/MacOS/ebook-convert";
#else
    return "ebook-convert";
#endif
}

QString ConversionService::defaultWgetPath() { return "wget"; }

static bool runProcess(const QString &program, const QStringList &args, QString *log, QString *error) {
    QProcess p;
    p.setProgram(program);
    p.setArguments(args);
    p.setProcessChannelMode(QProcess::MergedChannels);
    p.start();
    if (!p.waitForStarted(30000)) {
        if (error) *error = "Failed to start process: " + program;
        return false;
    }
    p.waitForFinished(-1);
    const auto out = QString::fromLocal8Bit(p.readAll());
    if (log) *log = out;
    if (p.exitStatus() != QProcess::NormalExit || p.exitCode() != 0) {
        if (error) *error = out.isEmpty() ? QString("%1 exited with code %2").arg(program).arg(p.exitCode()) : out;
        return false;
    }
    return true;
}

bool ConversionService::convertFile(const QString &ebookConvertPath, const QString &sourcePath, const QString &outputPath, QString *log, QString *error) {
    QString program = ebookConvertPath.trimmed().isEmpty() ? defaultEbookConvertPath() : ebookConvertPath.trimmed();
    return runProcess(program, {sourcePath, outputPath}, log, error);
}

QString ConversionService::findFirstHtmlFile(const QString &rootDir) {
    QDirIterator it(rootDir, QStringList() << "*.html" << "*.htm", QDir::Files, QDirIterator::Subdirectories);
    if (it.hasNext()) return it.next();
    return {};
}

bool ConversionService::mirrorWebsiteToPdf(const QString &wgetPath, const QString &ebookConvertPath, const QString &url, const QString &mirrorDir, const QString &outputPdfPath, int depth, QString *log, QString *error) {
    if (url.trimmed().isEmpty() || mirrorDir.trimmed().isEmpty() || outputPdfPath.trimmed().isEmpty()) {
        if (error) *error = "URL, mirror directory, and output PDF are required.";
        return false;
    }

    QDir().mkpath(mirrorDir);
    QString wgetProgram = wgetPath.trimmed().isEmpty() ? defaultWgetPath() : wgetPath.trimmed();

    QStringList wgetArgs{
        "--mirror", "--convert-links", "--adjust-extension", "--page-requisites", "--no-parent",
        "--directory-prefix", mirrorDir
    };
    if (depth > 0) {
        wgetArgs << "--level" << QString::number(depth);
    }
    wgetArgs << url;

    QString wgetLog;
    if (!runProcess(wgetProgram, wgetArgs, &wgetLog, error)) {
        if (log) *log = wgetLog;
        return false;
    }

    const QString htmlPath = findFirstHtmlFile(mirrorDir);
    if (htmlPath.isEmpty()) {
        if (error) *error = "No HTML file found after mirroring.";
        if (log) *log = wgetLog;
        return false;
    }

    QString convLog;
    if (!convertFile(ebookConvertPath, htmlPath, outputPdfPath, &convLog, error)) {
        if (log) *log = wgetLog + "\n" + convLog;
        return false;
    }

    if (log) *log = wgetLog + "\n" + convLog;
    return true;
}
