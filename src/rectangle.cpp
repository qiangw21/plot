#include "rectangle.h"
#include <QFileInfo>
#include <QDir>
#include <QHeaderView>
#include <QVector>
#include <QDebug>

const QVector<Qt::GlobalColor> colors = {Qt::red, Qt::green, Qt::blue, Qt::cyan, Qt::magenta,
                                     Qt::yellow, Qt::darkRed, Qt::darkGreen, Qt::darkBlue, Qt::darkYellow};

QString rectinf2string(const RectInf &rect){
    QString str = rect.label + "," + QString::number(rect.minPoint.x())
            + "," + QString::number(rect.minPoint.y()) + ","
            + QString::number(rect.maxPoint.x()) + "," +
            QString::number(rect.maxPoint.y());// + "," +
            //QString::number(rect.width) + "," + QString::number(rect.height);
    return str;
}

void Rectangle::init(QTableWidget *rectTable, Labels* labels){
    m_labels = labels;
    m_rects_table = rectTable;
    rectTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    rectTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    rectTable->setSelectionMode(QAbstractItemView::SingleSelection);
    setHeader();
}

void Rectangle::setHeader(){
    QStringList header;
    header << QStringLiteral("标签") << QStringLiteral("起点X坐标") << QStringLiteral("起点Y坐标") <<
              QStringLiteral("终点X坐标") << QStringLiteral("终点Y坐标");// << QStringLiteral("宽度") << QStringLiteral("高度");
    m_rects_table->setHorizontalHeaderLabels(header);
}

void Rectangle::append(RectInf& rect){
    m_rects.push_back(rect);
}

void Rectangle::insert(RectInf &rect){
    m_rects.push_back(rect);
    m_rects_table->insertRow(m_rects_table->rowCount());
    int rowId = m_rects_table->rowCount() - 1;
    setRowInf(rowId, rect);
}

void Rectangle::setRowInf(int rowId, RectInf &rect){

    QTableWidgetItem *item0 = new QTableWidgetItem(rect.label);
    QTableWidgetItem *item1 = new QTableWidgetItem(QString::number(rect.minPoint.x()));
    QTableWidgetItem *item2 = new QTableWidgetItem(QString::number(rect.minPoint.y()));
    QTableWidgetItem *item3 = new QTableWidgetItem(QString::number(rect.maxPoint.x()));
    QTableWidgetItem *item4 = new QTableWidgetItem(QString::number(rect.maxPoint.y()));
    //QTableWidgetItem *item5 = new QTableWidgetItem(QString::number(rect.width));
    //QTableWidgetItem *item6 = new QTableWidgetItem(QString::number(rect.height));
    m_rects_table->setItem(rowId, 0, item0);
    m_rects_table->setItem(rowId, 1, item1);
    m_rects_table->setItem(rowId, 2, item2);
    m_rects_table->setItem(rowId, 3, item3);
    m_rects_table->setItem(rowId, 4, item4);
    //m_rects_table->setItem(rowId, 5, item5);
    //m_rects_table->setItem(rowId, 6, item6);
}

void Rectangle::clear(){
    m_rects.clear();
    m_rects_table->clearContents();
    m_rects_table->setRowCount(0);
}

void Rectangle::deleteRect(){
    int id = m_rects_table->currentRow();
    if (id != -1){
        m_rects.remove(id);
        m_rects_table->removeRow(id);
    }
}

void Rectangle::setFileRoot(const QString &fileroot){
    m_file_root = fileroot;
}

void Rectangle::save(const QString &imgname){
    QStringList saveinf;
    for (int i=0; i<m_rects.size(); ++i){
        QString str = rectinf2string(m_rects[i]);
        saveinf.append(str);
    }
    QStringList tmp_name = imgname.split(".");
    int index = tmp_name.size() - 1;
    tmp_name[index] = "csv";
    QString savepath = m_file_root + "/" + tmp_name.join(".");
    m_csv_op.writer(savepath, saveinf);
}

void Rectangle::recover(const QString &imgname){
    QStringList tmp_name = imgname.split(".");
    int index = tmp_name.size() - 1;
    tmp_name[index] = "csv";
    QString filepath = m_file_root + "/" + tmp_name.join(".");
    clear();
    QFileInfo file(filepath);
    if (file.exists()){
        QStringList recoverinf;
        RectInf rect;
        m_csv_op.reader(filepath, recoverinf);
        for (int i=1; i<recoverinf.size(); ++i){
            QStringList s_rectinf = recoverinf[i].split(",");
            rect.label = s_rectinf[0];
            rect.minPoint.setX(s_rectinf[1].toInt());
            rect.minPoint.setY(s_rectinf[2].toInt());
            rect.maxPoint.setX(s_rectinf[3].toInt());
            rect.maxPoint.setY(s_rectinf[4].toInt());
            //rect.width = s_rectinf[5].toInt();
            //rect.height = s_rectinf[6].toInt();
            insert(rect);
        }
    }
}

void Rectangle::drawRects(QPainter &painter, const float* scale){
    if(!m_rects.empty()){
        for(int i=0; i<m_rects.size(); ++i){
            int x = m_rects[i].minPoint.x()*scale[0];
            int y = m_rects[i].minPoint.y()*scale[1];
            int w = m_rects[i].maxPoint.x()*scale[0] - x;
            int h = m_rects[i].maxPoint.y()*scale[1] - y;
            int colorId = m_labels->getLabelId(m_rects[i].label) % 10;
            painter.setPen(QPen(colors[colorId],2,Qt::SolidLine));//设置画笔形式
            painter.drawRect(x,y,w,h);
            painter.setPen(QPen(Qt::black,1,Qt::SolidLine));//设置画笔形式
            painter.drawText(x,y,m_rects[i].label);
        }
    }
}

RectInf& Rectangle::selectRect(int id, QPainter &painter, const float* scale){

    int x = m_rects[id].minPoint.x()*scale[0];
    int y = m_rects[id].minPoint.y()*scale[1];
    int w = m_rects[id].maxPoint.x()*scale[0] - x;
    int h = m_rects[id].maxPoint.y()*scale[1] - y;
    painter.fillRect(QRectF(x,y,w,h),QBrush(QColor(0,0,255,50)));
    return m_rects[id];

}

void Labels::init(QTableWidget *labelTable){
    m_labels_table = labelTable;
    QString currentpath = QDir::currentPath();
    //qDebug() << currentpath;
    currentpath += "/labelmap.csv";
    m_csv_op.reader(currentpath, m_labels);
    labelTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    labelTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    labelTable->setSelectionMode(QAbstractItemView::SingleSelection);
    //labelTable->setColumnWidth(0, 163);
    QStringList header;
    header << QStringLiteral("标签");
    labelTable->setHorizontalHeaderLabels(header);
    for(int i=0; i<m_labels.size(); ++i){
        labelTable->insertRow(labelTable->rowCount());
        labelTable->setItem(i, 0, new QTableWidgetItem(m_labels[i]));
    }
    labelTable->setCurrentCell(0, 0);
}

void Labels::addId(){
    int label_id = m_labels_table->currentRow();
    if(label_id>=0&&label_id<m_labels.count()-1){
        ++label_id;
    }else{
       label_id = 0;
    }
    m_labels_table->setCurrentCell(label_id, 0);
}

QString Labels::getCurrentLabel(){
    int label_id = m_labels_table->currentRow();
    return m_labels[label_id];
}

int Labels::getLabelId(const QString &label){
    return m_labels.indexOf(label);
}
