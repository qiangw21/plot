#ifndef IMAGE_H
#define IMAGE_H
#include <QPixmap>
#include <QLabel>
#include <QString>
#include <QPainter>
#include <memory>


class Image
{
public:
    Image():m_canvas(nullptr){
       // m_wl_ww = new double[2];
        //m_photometric_interpretation = new char[256];
    }
    ~Image() {
       /* if(m_wl_ww!=nullptr){
            delete[] m_wl_ww;
            m_wl_ww = nullptr;
        }
        if(m_photometric_interpretation!=nullptr){
            delete[] m_photometric_interpretation;
            m_photometric_interpretation = nullptr;
        }*/
    }
    void init(QLabel* canvas);
    int getWidth() {return m_width;}
    int getHeight() {return m_height;}
    const float* getScale() {return m_scale;}
    void setFileRoot(QString &fileroot) {m_file_root = fileroot;}
    void imread(QString &imgname);
    void display(QPainter &painter);
    void adjustBrightness(int brightness);
    void adjustContrast(int contrast);
    //void readDCM(const QString& filename);
    //bool dcmImagePreprocessor(const unsigned short *dcm_raw_data_ptr);
    //std::vector<int> cal_max_min_pixel_data(const unsigned short *dcm_raw_data_ptr);


private:
    QLabel* m_canvas;
    QImage m_image_show;
    QImage m_image_orig;
    QString m_file_root;
    int m_width;
    int m_height;
    float m_scale[2];

    //dcmInfo
    //std::unique_ptr<unsigned short> m_dcm_raw_data_ptr;
    //double *m_wl_ww;
    //char *m_photometric_interpretation;
};

#endif // IMAGE_H
