#include "image.h"
/*#include "itkConfigure.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkGDCMImageIO.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkImageSeriesReader.h"

int read_dicom_tags(const std::string& filename, char *photometric_interpretation, double *wl_ww) {

 char wl[256] = "";
 char ww[256] = "";

 typedef signed short ItkPixelType;
 const unsigned int Dimension = 2;
 typedef itk::Image<ItkPixelType, Dimension> ImageType;
 typedef itk::ImageSeriesReader<ImageType> ReaderType;
 ReaderType::Pointer reader = ReaderType::New();
 typedef itk::GDCMImageIO ImageIOType;
 ImageIOType::Pointer dicomIO = ImageIOType::New();

 reader->SetFileName(filename);
 reader->SetImageIO(dicomIO);

 try {
    reader->Update();
 }catch (itk::ExceptionObject &ex) {
     std::cout << ex << std::endl;
     return EXIT_FAILURE;
 }

 typedef itk::MetaDataDictionary DictionaryType;
 const DictionaryType &dictionary = dicomIO->GetMetaDataDictionary();

 typedef itk::MetaDataObject<std::string> MetaDataStringType;
 auto itr = dictionary.Begin();
 auto end = dictionary.End();

 while (itr != end) {

    itk::MetaDataObjectBase::Pointer entry = itr->second;
    MetaDataStringType::Pointer entryvalue = dynamic_cast<MetaDataStringType*>(entry.GetPointer());
    if (entryvalue) {
        std::string tagkey = itr->first;
        std::string labelID;
        bool found = itk::GDCMImageIO::GetLabelFromTag(tagkey, labelID);

        std::string tagvalue = entryvalue->GetMetaDataObjectValue();

        // tag photometric_interpretation and window level, window width
        bool found_entry = false;
        if (labelID == "Photometric Interpretation") {
            memcpy(photometric_interpretation, tagvalue.c_str(), tagvalue.length() - 1);
            found_entry = true;
        }
        if (labelID == "Window Center") {
            memcpy(wl, tagvalue.c_str(), tagvalue.length() * sizeof(char));
            found_entry = true;
        }
        if (labelID == "Window Width") {
            memcpy(ww, tagvalue.c_str(), tagvalue.length() * sizeof(char));
            found_entry = true;
        }

    }

    ++itr;
 }

 if (std::string(wl) != "" && std::string(ww) != "") {
    wl_ww[0] = double(std::stoi(wl));
    wl_ww[1] = double(std::stoi(ww));
 }

 return EXIT_SUCCESS;
}*/

void Image::init(QLabel *canvas){
    m_canvas = canvas;
    m_canvas->setScaledContents(true);
    m_canvas->setMouseTracking(true);
}

void Image::imread(QString &imgname){
    QString imgfile = m_file_root + "/" + imgname;
    //if (imgname.endsWith(".dcm") || imgname.endsWith(".DCM")){
      //  readDCM(imgfile);
    //}else{
        m_image_show.load(imgfile);
        //m_image_orig.load(imgfile);
        m_image_orig = m_image_show;
        m_width = m_image_orig.width();
        m_height = m_image_orig.height();
    //}
}

void Image::display(QPainter &painter){
    //canvas->setFixedSize(m_width, m_height);
    QPixmap pixmap2show = QPixmap::fromImage(m_image_show);
    m_scale[0] = float(m_canvas->width()) / m_width;
    m_scale[1] = float(m_canvas->height()) / m_height;
    painter.drawPixmap(0, 0, m_canvas->width(), m_canvas->height(), pixmap2show);
}

void Image::adjustBrightness(int brightness){
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

void Image::adjustContrast(int contrast){
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

/*void Image::readDCM(const QString &filename){
    typedef unsigned short InputPixelType;
    const unsigned int   InputDimension = 2;
    typedef itk::Image< InputPixelType, InputDimension> InputImageType;

    typedef itk::ImageFileReader< InputImageType > ReaderType;
    ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileName(filename.toStdString());

    typedef itk::GDCMImageIO ImageIOType;
    ImageIOType::Pointer gdcmImageIO = ImageIOType::New();
    reader->SetImageIO(gdcmImageIO);

    try{
        reader->Update();
    }catch (itk::ExceptionObject& e){
        std::cerr << "exception in file reader " << std::endl;
        std::cerr << e << std::endl;
        return;
    }

    InputImageType::Pointer itkImage = reader->GetOutput();
    InputImageType::SizeType size = itkImage->GetLargestPossibleRegion().GetSize();
    typedef typename itk::ImageRegionIterator<InputImageType> IteratorType;
    IteratorType it(itkImage, itkImage->GetLargestPossibleRegion());

    m_width = static_cast<int>(size[0]);
    m_height = static_cast<int>(size[1]);
    int depth  = static_cast<int>(size[2]);
    InputPixelType* raw_ptr = new InputPixelType[m_width * m_height * depth];
    m_dcm_raw_data_ptr.reset(raw_ptr);
    int idx = 0;
    it.GoToBegin();
    while (!it.IsAtEnd()) {
        raw_ptr[idx] = it.Get();
        ++it; ++idx;
    }

    assert(idx == m_width * m_height * depth);
    read_dicom_tags(filename.toStdString(), m_photometric_interpretation, m_wl_ww);
    dcmImagePreprocessor(raw_ptr);
}


bool Image::dcmImagePreprocessor(const unsigned short *dcm_raw_data_ptr){
  if(dcm_raw_data_ptr != nullptr){
    return false;
  }

  std::vector<int> max_min_pixel = cal_max_min_pixel_data(dcm_raw_data_ptr);
  int pixel_max_val = max_min_pixel[0];
  int pixel_min_val = max_min_pixel[1];

  int pixel_value = 0;
  std::unique_ptr<uchar> dcm_img;
  uchar* tem_img = new uchar[m_width * m_height];
  dcm_img.reset(tem_img);
  std::string _photometric_interpretation(m_photometric_interpretation);
  for(int k =0; k < m_width*m_height; ++k){
    pixel_value = dcm_raw_data_ptr[k] > pixel_max_val ? pixel_max_val:dcm_raw_data_ptr[k];
    pixel_value = dcm_raw_data_ptr[k] < pixel_min_val ? pixel_min_val:pixel_value;
    if(_photometric_interpretation == "MONOCHROME1"){
        pixel_value = static_cast<float>(1.0 - static_cast<float>(pixel_value - pixel_min_val) / (pixel_max_val - pixel_min_val));
    }else{
        pixel_value = static_cast<float>(pixel_value - pixel_min_val) / (pixel_max_val - pixel_min_val);
    }
    tem_img[k] = static_cast<uchar>(pixel_value*255 + 0.5);
  }

  m_image_orig = QImage(tem_img, m_width, m_height, QImage::Format_RGB32);
  m_image_show = m_image_orig;
  return true;
}

std::vector<int> Image::cal_max_min_pixel_data(const unsigned short *dcm_raw_data_ptr)
 {

  const double *wl_ww = m_wl_ww;

   // calculate max_val, min_val for normalization
  const int num_pixel_elements = m_width*m_height;
  const ushort *im_ptr = dcm_raw_data_ptr;
  int max_pixel = im_ptr[0];
  int min_pixel = im_ptr[0];
  for(int i=0; i<num_pixel_elements; ++i){
      if(max_pixel<im_ptr[i])
        max_pixel = im_ptr[i];
      if(min_pixel > im_ptr[i])
        min_pixel = im_ptr[i];
  }
  int pixel_max_val = max_pixel;
  int pixel_min_val = min_pixel;
  if (int(wl_ww[0]) != 0 || int(wl_ww[1]) != 0){
      int wl = static_cast<int>(wl_ww[0]);
      int ww = static_cast<int>(wl_ww[1]);
      if (int(wl + ww/2) < max_pixel && int(wl - ww/2) > min_pixel){
           pixel_max_val = int(wl + ww/2);
           pixel_min_val = int(wl- ww/2);
       }
  }

  std::vector<int> max_min_pixel_data(2);
  max_min_pixel_data[0] = pixel_max_val;
  max_min_pixel_data[1] = pixel_min_val;

  return max_min_pixel_data;
 }*/


