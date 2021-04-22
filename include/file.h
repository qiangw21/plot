#ifndef FILE_H
#define FILE_H
#include <QString>
#include <QStringList>

class File_OP
{
public:
    File_OP();
    void reader(const QString& filename, QStringList& lists);
    void writer(const QString& filename, const QStringList& lists, bool wr_header=true);

private:
    QString m_header;

};




#endif // IMAGE_H
