#ifndef RECTANGLE_H
#define RECTANGLE_H
#include <QPoint>
#include <QVector>
#include <QString>
#include <QPainter>
#include <QListWidget>
#include <QTableWidget>
#include "file.h"

struct RectInf
{
    QPoint minPoint;
    QPoint maxPoint;
    QString label;
    //int height;
    //int width;
};

class Labels;

QString rectinf2string(const RectInf& rect);

class Rectangle
{
public:
    Rectangle():m_rects_table(nullptr), m_labels(nullptr){}
    void init(QTableWidget *rectTable, Labels* labels);
    void setHeader();
    void setFileRoot(const QString& fileroot);
    void setRowInf(int rowId, RectInf& rect);
    void append(RectInf& rect);
    void insert(RectInf& rect);
    void clear();
    void deleteRect();
    void save(const QString& imgname);
    void recover(const QString& imgname);
    void drawRects(QPainter& painter, const float* scale);
    RectInf& selectRect(int id, QPainter& painter, const float* scale);

private:
    QTableWidget* m_rects_table;
    QVector<RectInf> m_rects;
    QString m_file_root;
    File_OP m_csv_op;
    Labels* m_labels;
};

class Labels
{
public:
    Labels():m_labels_table(nullptr){}
    void init(QTableWidget *labelTable);
    void addId();
    QString getCurrentLabel();
    int getLabelId(const QString& lable);

private:
    QTableWidget* m_labels_table;
    QStringList m_labels;
    File_OP m_csv_op;
};

#endif // RECTANGLE_H
