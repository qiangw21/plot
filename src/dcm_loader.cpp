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
    m_img = new unsigned char[m_width*m_height];
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
	double multiplier = 255.0 / (pixel_max_val - pixel_min_val);
    //omp_set_num_threads(2);
    //double start = omp_get_wtime();
    #pragma omp parallel for
    for(int k = 0; k < length; ++k){
        if(m_pixelData[k] > pixel_max_val){
            m_img[k] = 255;
        }else if(m_pixelData[k] < pixel_min_val){
            m_img[k] = 0;
        }else {
            m_img[k] = static_cast<unsigned char>((m_pixelData[k] - pixel_min_val) * multiplier);
        }
        if (m_PhotometricInterpretation == "MONOCHROME1"){
            m_img[k] = static_cast<unsigned char>(255 - m_img[k]);
        }

    }
    //double end = omp_get_wtime();
    //std::cout << "Multi-thread Time is: " << end - start << std::endl;
}

void DcmLoader::getWindow(double &wl, double &ww)
{
    wl = m_wl;
    ww = m_ww;
}
