#include "dcm_loader.h"
#include "omp.h"

bool DcmLoader::loadFile(const std::string &dcmfile)
{
    OFCondition oc = m_dfile.loadFile(dcmfile.c_str());
    if(!oc.good())
        return false;

    DcmDataset *dataset = m_dfile.getDataset();

    dataset->findAndGetUint16(DCM_Rows, m_height);
    dataset->findAndGetUint16(DCM_Columns, m_width);
    dataset->findAndGetFloat64(DCM_WindowCenter, m_wl);
    dataset->findAndGetFloat64(DCM_WindowWidth, m_ww);
    dataset->findAndGetOFString(DCM_PhotometricInterpretation, m_PhotometricInterpretation);

    DcmElement* element = nullptr;
    oc = dataset->findAndGetElement(DCM_PixelData, element);
    if (oc.bad() || element == nullptr)
        return false;
    oc = element->getUint16Array(m_pixelData);
    if(oc.bad())
        return false;

    if (m_img != nullptr)
        delete [] m_img;
    m_img = new unsigned char[m_width*m_height*3];
    setWindow(m_wl, m_ww);

    return true;
}

void DcmLoader::setWindow(double wl, double ww)
{
    Uint16 pixel_max_val = Uint16(wl + ww / 2);
    Uint16 pixel_min_val = Uint16(wl - ww / 2);
    if(m_img == nullptr)
        return;

    int length = m_height*m_width;
    omp_set_num_threads(1024);
    #pragma omp parallel for
    for(int k = 0; k < length; ++k){
        int pixel_value = m_pixelData[k];
        if(m_pixelData[k] > pixel_max_val)
            pixel_value = pixel_max_val;
        else if(m_pixelData[k] < pixel_min_val)
            pixel_value = pixel_min_val;

        if (m_PhotometricInterpretation == "MONOCHROME1") {
            m_img[3*k] = static_cast<unsigned char>(
                (1.0 - static_cast<double>(pixel_value - pixel_min_val) / (pixel_max_val - pixel_min_val)) * 255 + 0.5);
            m_img[3*k + 1] = m_img[3*k];
            m_img[3*k + 2] = m_img[3*k];
        }
        else {
            m_img[3*k] = static_cast<unsigned char>(
                (static_cast<double>(pixel_value - pixel_min_val) / (pixel_max_val - pixel_min_val)) * 255 + 0.5);
            m_img[3 * k + 1] = m_img[3 * k];
            m_img[3 * k + 2] = m_img[3 * k];
        }
    }
}

void DcmLoader::getWindow(double &wl, double &ww)
{
    wl = m_wl;
    ww = m_ww;
}
