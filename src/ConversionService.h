#pragma once
#include <QString>

class ConversionService {
public:
    static QString defaultEbookConvertPath();
    static QString defaultWgetPath();
    static bool convertFile(const QString &ebookConvertPath, const QString &sourcePath, const QString &outputPath, QString *log, QString *error);
    static bool mirrorWebsiteToPdf(const QString &wgetPath, const QString &ebookConvertPath, const QString &url, const QString &mirrorDir, const QString &outputPdfPath, int depth, QString *log, QString *error);
private:
    static QString findFirstHtmlFile(const QString &rootDir);
};
