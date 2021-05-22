#include "image.h"

void Image::init(QLabel *canvas){
    m_canvas = canvas;
    m_canvas->setScaledContents(true);
    m_canvas->setMouseTracking(true);
}

bool Image::imread(QString &imgname){
    QString imgfile = m_file_root + "/" + imgname;
    if (imgname.endsWith(".jpg") || imgname.endsWith(".png") || imgname.endsWith(".jpeg")){
        if(! m_image_show.load(imgfile))
            return false;
        //m_image_orig.load(imgfile);
        m_image_orig = m_image_show;
        m_width = m_image_orig.width();
        m_height = m_image_orig.height();
        m_is_dcm = false;
    }else {
        if(! m_dcmLoader.loadFile(imgfile.toStdString()))
            return false;
        m_width = m_dcmLoader.getWidth();
        m_height = m_dcmLoader.getHeight();
        m_dcmLoader.getWindow(m_org_wl, m_org_ww);
        m_temp_wl = m_org_wl;
        m_temp_ww = m_org_ww;
        unsigned char* data = m_dcmLoader.getImg();
        m_image_show = QImage(data, m_width, m_height, m_width*3, QImage::Format_RGB888);
        //m_image_orig = m_image_show;
        m_is_dcm = true;
    }
    return true;
}

void Image::display(QPainter &painter){
    //canvas->setFixedSize(m_width, m_height);
    QPixmap pixmap2show = QPixmap::fromImage(m_image_show);
    m_scale[0] = float(m_canvas->width()) / m_width;
    m_scale[1] = float(m_canvas->height()) / m_height;
    painter.translate(m_offset);
    painter.scale(m_zoomValue, m_zoomValue);
    painter.drawPixmap(0, 0, m_canvas->width(), m_canvas->height(), pixmap2show);

}

void Image::adjustBrightness(int brightness){
    if(m_is_dcm){
        m_temp_ww = static_cast<double>(brightness);
        m_dcmLoader.setWindow(m_temp_wl, m_temp_ww);
    }else{
        int red, green, blue;
        QImage tmp_image = m_image_orig;
        int pixels = m_width * m_height;

        unsigned int *data = (unsigned int *)tmp_image.bits();
        for (int i = 0; i < pixels; ++i)
        {
            red= qRed(data[i])+ brightness;
            red = (red < 0x00) ? 0x00 : (red > 0xff) ? 0xff : red;
            green= qGreen(data[i])+brightness;
            green = (green < 0x00) ? 0x00 : (green > 0xff) ? 0xff : green;
            blue= qBlue(data[i])+brightness;
            blue =  (blue  < 0x00) ? 0x00 : (blue  > 0xff) ? 0xff : blue ;
            data[i] = qRgba(red, green, blue, qAlpha(data[i]));
        }
        m_image_show = tmp_image;
    }
}

void Image::adjustContrast(int contrast){
    if(m_is_dcm){
        m_temp_wl = static_cast<double>(contrast);
        m_dcmLoader.setWindow(m_temp_wl, m_temp_ww);
    }else{
        QImage tmp_image = m_image_orig;
        int pixels = m_width * m_height;
        unsigned int *data = (unsigned int *)tmp_image.bits();

        int red, green, blue, nRed, nGreen, nBlue;

        if (contrast > 0 && contrast < 100)
        {
            float param = 1 / (1 - contrast / 100.0) - 1;

            for (int i = 0; i < pixels; ++i)
            {
                nRed = qRed(data[i]);
                nGreen = qGreen(data[i]);
                nBlue = qBlue(data[i]);

                red = nRed + (nRed - 127) * param;
                red = (red < 0x00) ? 0x00 : (red > 0xff) ? 0xff : red;
                green = nGreen + (nGreen - 127) * param;
                green = (green < 0x00) ? 0x00 : (green > 0xff) ? 0xff : green;
                blue = nBlue + (nBlue - 127) * param;
                blue = (blue < 0x00) ? 0x00 : (blue > 0xff) ? 0xff : blue;

                data[i] = qRgba(red, green, blue, qAlpha(data[i]));
            }
        }else{
            for (int i = 0; i < pixels; ++i)
            {
                nRed = qRed(data[i]);
                nGreen = qGreen(data[i]);
                nBlue = qBlue(data[i]);

                red = nRed + (nRed - 127) * contrast / 100.0;
                red = (red < 0x00) ? 0x00 : (red > 0xff) ? 0xff : red;
                green = nGreen + (nGreen - 127) * contrast / 100.0;
                green = (green < 0x00) ? 0x00 : (green > 0xff) ? 0xff : green;
                blue = nBlue + (nBlue - 127) * contrast / 100.0;
                blue = (blue < 0x00) ? 0x00 : (blue > 0xff) ? 0xff : blue;

                data[i] = qRgba(red, green, blue, qAlpha(data[i]));
            }
        }
        m_image_show = tmp_image;
    }
}

void Image::OnZoomInImage(){
    m_zoomValue += 0.05;
}

void Image::OnZoomoutImage()
{
    m_zoomValue -= 0.05;
    if(m_zoomValue <=0)
    {
        m_zoomValue = 0.05;
    }
}

void Image::OnPresetImage()
{
    m_zoomValue = 1.0;
}

void Image::setOffset(QPoint &offset)
{
    m_offset.setX(offset.x());
    m_offset.setY(offset.y());
}

void Image::setOffset(int x, int y)
{
    m_offset.setX(x);
    m_offset.setY(y);
}

void Image::addOffset(QPoint &offset)
{
    m_offset += offset;
}


