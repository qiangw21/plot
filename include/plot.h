#ifndef PLOT_H
#define PLOT_H

#include <QMap>
#include <QSet>
#include <QMainWindow>
#include <QString>
#include <QObject>
#include <QMouseEvent>
#include <Qvector>
#include <QPainter>
#include "label_infs.h"
#include "image.h"

enum LineLoc {
    line_left,
    line_right,
    line_top,
    line_down,
    line_top_left,
    line_top_right,
    line_down_left,
    line_down_right,
    none_loc
};

namespace Ui {
class plotUi;
}

class Plot : public QMainWindow
{
    Q_OBJECT


public:
    explicit Plot(QWidget *parent = nullptr);
    ~Plot();
public slots:
    void openFloder();
    void save();
    void preImg();
    void nextImg();
    void deleteRect();
    void skipImg();
    void windowClose();
    void adjustBrightness(int brightness);
    void adjustContrast(int contrast);
    void resetBrightness();
    void resetContrast();
    void clear();
    void clickedFileLists();
    void changeSelectRectlabel();
    //void selectRectInfo();
    void downSelectRect();
    void upSelectRect();

protected:
   bool eventFilter(QObject *watched,QEvent *event);
   void wheelEvent(QWheelEvent *event);
   void mousePressEvent(QMouseEvent *event);
   void mouseMoveEvent(QMouseEvent *event);
   void mouseReleaseEvent(QMouseEvent *event);
   virtual void keyPressEvent(QKeyEvent *event);

signals:
   void pressPoint(QPoint s);
   void mobilePoint(QPoint m);
   void releasePoint(QPoint e);

private:
    Ui::plotUi *ui;
    QPainter m_painter; //画笔
    Labels m_labels;   //标签信息
    RectInf m_rectinf; //当前标注信息
    LabelInfs m_rects; //标注信息
    Image m_img;
    QStringList m_imgnamelists; //文件夹中图片名
    QPoint m_movepoint,m_oldpoint,m_moverectpoint;
    QPolygon m_pairpoint;//存储最小点和最大点
    int m_imgid;//当前图像计数
    LineLoc m_lineloc;
    bool m_leftPress;
    File_OP m_cache;

    void draw();//画矩形
    void recover(); //恢复标注状态
    void updateInf();//更新信息
    void updateImgNameLists(const QString& dir);
    bool isDICM(const QString& file);
    void moveRectLine(QPoint &point1, QPoint &point2);//修改矩形框
    void setMoveLineCuros(QPoint &point1, QPoint &point2);
    void setRectinf();
};

#endif // PLOT_H
