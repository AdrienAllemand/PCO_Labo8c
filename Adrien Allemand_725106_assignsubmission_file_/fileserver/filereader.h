#ifndef QFILEREADER_H
#define QFILEREADER_H
#include <QFileInfo>
#include <QTextStream>

class FileReader
{
public:
    FileReader(const QString path, bool debug = false);

    bool fileExists();
    int byteCount();
    QString readAll();

private:
    void simulateHeavyEncoding();
    QFileInfo info;
    bool debug;
};

#endif // QFILEREADER_H
