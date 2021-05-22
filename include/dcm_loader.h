#ifndef DCM_LOADER_H
#define DCM_LOADER_H
#include "dcmtk/dcmdata/dctk.h"
#include "dcmtk/dcmimgle/dcmimage.h"
#include "dcmtk/dcmdata/dcfilefo.h"

class DcmLoader
{
public:
    DcmLoader():m_img(nullptr), m_pixelData(nullptr),
        m_wl(0), m_ww(0), m_width(0), m_height(0){}
    ~DcmLoader(){
        if (m_img != nullptr)
            delete[] m_img;
    }
    bool loadFile(const std::string& dcmfile);
    void setWindow(double wl, double ww);
    void getWindow(double& wl, double& ww);
    int getWidth(){return static_cast<int>(m_width);}
    int getHeight(){return static_cast<int>(m_height);}
    unsigned char* getImg(){return m_img;}

private:
    unsigned char* m_img;
    Uint16* m_pixelData;
    DcmFileFormat m_dfile;
    double m_wl;
    double m_ww;
    unsigned short m_width;
    unsigned short m_height;
    OFString m_PhotometricInterpretation;
};

#endif
