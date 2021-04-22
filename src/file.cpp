#include "file.h"
#include <QDir>
#include <QFile>
#include <QTextStream>

File_OP::File_OP(): m_header("label,start_x,start_y,end_x,end_y") {}

void File_OP::reader(const QString &filename, QStringList& lists){
    QFile file(filename);
    QString line;
    if(file.open(QFile::QIODevice::ReadOnly)){
        lists.clear();
        QTextStream stream(&file);
        while(!stream.atEnd()){
            line=stream.readLine();
            lists.append(line);
        }
        file.close();
    }
}

void File_OP::writer(const QString &filename, const QStringList &lists, bool wr_header){
    QFile file(filename);
    if(file.open(QIODevice::WriteOnly)){
        QTextStream stream(&file);
        if(wr_header)
            stream << m_header << endl;
        for(int i=0; i<lists.size(); ++i){
            stream << lists[i] << endl;
        }
        file.close();
    }
}

