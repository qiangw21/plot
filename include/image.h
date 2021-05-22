#ifndef IMAGE_H
#define IMAGE_H
#include <QPixmap>
#include <QLabel>
#include <QString>
#include <QPainter>
#include <memory>
#include "dcm_loader.h"


class Image
{
public:
    Image():m_canvas(nullptr), m_zoomValue(1.0),
            m_offset(0, 0), m_is_dcm(false){}
    ~Image() {}
    void init(QLabel* canvas);
    int getWidth() {return m_width;}
    int getHeight() {return m_height;}
    const float* getScale() {return m_scale;}
    void setFileRoot(QString &fileroot) {m_file_root = fileroot;}
    bool imread(QString &imgname);
    void display(QPainter &painter);
    void adjustBrightness(int brightness);
    void adjustContrast(int contrast);
    void OnZoomInImage();
    void OnZoomoutImage();
    void OnPresetImage();
    qreal getZoomValue() {return m_zoomValue;}
    const QPoint& getOffset() {return m_offset;}
    void setOffset(QPoint& offset);
    void setOffset(int x, int y);
    void addOffset(QPoint& offset);
    bool getIsDCM() {return m_is_dcm;}
    double getOrgWW(){return m_org_ww;}
    double getOrgWL(){return m_org_wl;}

private:
    QLabel* m_canvas;
    QImage m_image_show;
    QImage m_image_orig;
    QString m_file_root;
    qreal m_zoomValue;
    QPoint m_offset;
    int m_width;
    int m_height;
    float m_scale[2];

    //dcmInfo
    DcmLoader m_dcmLoader;
    double m_org_ww;
    double m_org_wl;
    double m_temp_ww;
    double m_temp_wl;
    bool m_is_dcm;
};

#endif // IMAGE_H
