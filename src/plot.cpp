#include "plot.h"
#include "ui_plot.h"
#include <QFileDialog>
#include <QDebug>
#include <QDir>
#include <QPainter>
#include <QMessageBox>
#include <sstream>
#include <string>
#include <QDebug>


Plot::Plot(QWidget *parent) :
    QMainWindow(parent), ui(new Ui::plotUi)
{
    ui->setupUi(this);
    ui->display_image->installEventFilter(this);
    ui->widget->setMouseTracking(true);
    ui->centralWidget->setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);    
    connect(ui->open_floder,SIGNAL(clicked(bool)),this,SLOT(openFloder()));
    connect(ui->save,SIGNAL(clicked(bool)),this,SLOT(save()));
    connect(ui->pre_img,SIGNAL(clicked(bool)),this,SLOT(preImg()));
    connect(ui->next_img,SIGNAL(clicked(bool)),this,SLOT(nextImg()));
    connect(ui->delete_button,SIGNAL(clicked(bool)),this,SLOT(deleteRect()));
    connect(ui->skip,SIGNAL(clicked(bool)),this,SLOT(skipImg()));
    connect(ui->exit,SIGNAL(clicked(bool)),this,SLOT(windowClose()));
    connect(ui->clear,SIGNAL(clicked(bool)),this,SLOT(clear()));
    connect(ui->brightness, SIGNAL(valueChanged(int)), this, SLOT(adjustBrightness(int)));
    connect(ui->contrast, SIGNAL(valueChanged(int)),this,SLOT(adjustContrast(int)));
    connect(ui->reset_brightness, SIGNAL(clicked(bool)),this,SLOT(resetBrightness()));
    connect(ui->reset_contrast,SIGNAL(clicked(bool)),this,SLOT(resetContrast()));
    connect(ui->fileLists, SIGNAL(clicked(QModelIndex)), this, SLOT(clickedFileLists()));
    connect(ui->labels, SIGNAL(clicked(QModelIndex)), this, SLOT(changeSelectRectlabel()));
    m_imgid = 0;
    m_labels.init(ui->labels);
    m_img.init(ui->display_image);
    m_rects.init(ui->rectsTable, &m_labels);

}

Plot::~Plot()
{
    delete ui;
}

void Plot::openFloder() //打开图片文件夹
{
    QString cachePath = QDir::currentPath() +  "/cache.txt";
    QStringList cacheList;
    QString openDir = ".";
    QFileInfo file(cachePath);
    if(file.exists()){
        m_cache.reader(cachePath, cacheList);
        QDir dir(cacheList[0]);
        if(dir.exists())
            openDir = cacheList[0];
    }
    QString filepath=QFileDialog::getExistingDirectory(this,tr("图像路径"), openDir);
    if(filepath.isEmpty()){
        return;
    }else{
        cacheList.clear();
        cacheList << filepath;
        m_cache.writer(cachePath, cacheList, false);
        m_rects.setFileRoot(filepath);
        m_img.setFileRoot(filepath);
        ui->caption->setText("初始化...");
        QDir dir(filepath);
        QStringList namefilters;
        namefilters<<"*.jpg"<<"*.png"<<"*.jpeg";
        m_imgnamelists = dir.entryList(namefilters,QDir::Files|QDir::Readable,QDir::Name);
        if(m_imgnamelists.isEmpty()){
            QMessageBox::information(this, tr("提示"),
                                     tr("文件夹为空!"));
            return;
        }
        for(int i=0; i<m_imgnamelists.size(); ++i){
            ui->fileLists->insertItem(i, filepath + "/" + m_imgnamelists[i]);
        }
        //qDebug()<<imgNameList[0];
        ui->progressBar->setRange(0,m_imgnamelists.count()-1);
        ui->skip_line->setText("1");
        updateInf();
    }

}

bool Plot::eventFilter(QObject *watched, QEvent *event)
{
    if(watched==ui->display_image&&event->type()==QEvent::Paint&&!m_imgnamelists.isEmpty()){
        m_painter.begin(ui->display_image);
        draw();
        update();
        m_painter.end();
        return false;
    }
    return QWidget::eventFilter(watched,event);
}



void Plot::mousePressEvent(QMouseEvent *event)
{
    if(!m_imgnamelists.isEmpty()&&event->button()==Qt::RightButton){
        if(ui->rectsTable->currentRow() != -1){
            ui->rectsTable->setCurrentCell(-1, -1);
        }else{
            m_labels.addId();
        }

    }

}

void Plot::mouseMoveEvent(QMouseEvent *event)
{
    if(!m_imgnamelists.isEmpty())
    {
        m_movepoint=event->pos()-(ui->centralWidget->pos()+ui->widget->pos()+ui->display_image->pos());
        if(event->buttons()&Qt::LeftButton){
            int x = static_cast<int>(m_movepoint.x() / m_img.getScale()[0]);
            int y = static_cast<int>(m_movepoint.y() / m_img.getScale()[1]);
            x = x < m_img.getWidth() ? x : m_img.getWidth();
            x = x > 0 ? x : 0;
            y = y < m_img.getHeight() ? y : m_img.getHeight();
            y = y > 0 ? y : 0;
            m_moverectpoint.setX(x);
            m_moverectpoint.setY(y);
        }else {
            m_moverectpoint.setX(0);
            m_moverectpoint.setY(0);
        }
        emit mobilePoint(m_movepoint);
    }
}

void Plot::mouseReleaseEvent(QMouseEvent *event)
{
    if(!m_imgnamelists.isEmpty()&&event->button()==Qt::LeftButton)
    {
        m_point=event->pos()-(ui->centralWidget->pos()+ui->widget->pos()+ui->display_image->pos());
        if(m_point.x()>=0&&m_point.x()<=ui->display_image->width()&&
           m_point.y()>=0&&m_point.y()<=ui->display_image->height())//判断点是否在图像内部
        {
            m_pairpoint.append(m_point);
            if(m_pairpoint.count()==2){
                if( m_pairpoint[1].x()<m_pairpoint[0].x()){
                    int x = m_pairpoint[0].x();
                    m_pairpoint[0].setX(m_pairpoint[1].x());
                    m_pairpoint[1].setX(x);
                }
                if( m_pairpoint[1].y()<m_pairpoint[0].y()){
                    int y = m_pairpoint[0].y();
                    m_pairpoint[0].setY(m_pairpoint[1].y());
                    m_pairpoint[1].setY(y);
                }
                setRectinf();
            }
        }

        emit releasePoint(m_point);
    }
}

void Plot::draw()
{
    m_img.display(m_painter);
    m_painter.setPen(QPen(Qt::black,1,Qt::SolidLine));
    m_painter.drawLine(m_movepoint.x(),0,m_movepoint.x(),ui->display_image->height());
    m_painter.drawLine(0,m_movepoint.y(),ui->display_image->width(),m_movepoint.y());
    if(!m_pairpoint.isEmpty()){
        m_painter.setPen(QPen(Qt::red,2,Qt::SolidLine));
        m_painter.drawPoints(m_pairpoint);
    }
    m_rects.drawRects(m_painter, m_img.getScale());
    int id = ui->rectsTable->currentRow();
    if(id!=-1){
        RectInf& t_rect = m_rects.selectRect(id, m_painter, m_img.getScale());
        moveRectLine(t_rect.minPoint, t_rect.maxPoint);
        m_rects.setRowInf(id, t_rect);
    }

}

void Plot::clickedFileLists(){
    if(ui->fileLists->currentRow() != m_imgid){
        m_imgid = ui->fileLists->currentRow();
        updateInf();
    }
}

void Plot::save()
{
    if(m_imgnamelists.isEmpty()){
        QMessageBox::information(this, tr("提示"),
                                 tr("请打开文件夹!"));
        return;
    }
    ui->caption->setText("保存中...");
    m_rects.save(m_imgnamelists[m_imgid]);
    ui->caption->setText("保存成功！");
}

void Plot::recover()
{
  m_rects.recover(m_imgnamelists[m_imgid]);
}

void Plot::updateInf()
{
    ui->fileLists->setCurrentRow(m_imgid);
    //QString ratioDeal=QString("%1").arg(m_imgid+1)+"/"+QString("%1").arg(m_imgnamelists.count());
    //ui->lineEdit->setText(ratioDeal);
    ui->progressBar->setValue(m_imgid);
    ui->caption->setText("No."+QString("%1").arg(m_imgid+1)+": "+m_imgnamelists[m_imgid]); 
    m_img.imread(m_imgnamelists[m_imgid]);
    ui->brightness->setValue(0);
    ui->contrast->setValue(0);
    recover();
    this->setCursor(Qt::ArrowCursor);
    m_pairpoint.clear();
}

void Plot::preImg()
{
    if(m_imgnamelists.isEmpty()){
        QMessageBox::information (this, tr("提示"),
                                  tr("请打开文件夹!"));
        return;
    }
    if(m_imgid==0){
        QMessageBox::information (this,tr("提示"),
                                  tr("这是第一张!"));
         return;

    }else {
        save();
        --m_imgid;
        updateInf();
    }
}

void Plot::nextImg()
{
    if(m_imgnamelists.isEmpty()){
        QMessageBox::information (this, tr("提示"),
                                  tr("请打开文件夹!"));
        return;
    }
    if(m_imgid==m_imgnamelists.count()-1)
    {
        QMessageBox::information (this, tr("提示"),
                                  tr("已是最后一张!"));
        return;
    }else {
        save();
        ++m_imgid;
        updateInf();
    }
}

void Plot::skipImg()
{
    if(m_imgnamelists.isEmpty()){
        QMessageBox::information (this, tr("提示"),
                                  tr("请打开文件夹!"));
        return;
    }
    m_imgid = ui->skip_line->text().toInt() - 1;
    if(m_imgid<0){
        m_imgid = 0;
    }else if(m_imgid>=m_imgnamelists.count()){
        m_imgid=m_imgnamelists.count()-1;
    }
    updateInf();
}

void Plot::windowClose()
{
    save();
    this->close();
}
void Plot::deleteRect()
{
    m_rects.deleteRect();
    m_pairpoint.clear();
}

void Plot::clear()
{
    m_rects.clear();
}

void Plot::adjustBrightness(int brightness){
    if(!m_imgnamelists.isEmpty()){
        m_img.adjustBrightness(brightness);
    }
}

void Plot::adjustContrast(int contrast){
    if(!m_imgnamelists.isEmpty()){
        m_img.adjustContrast(contrast);
    }
}

void Plot::resetBrightness(){
    ui->brightness->setValue(0);
}

void Plot::resetContrast(){
    ui->contrast->setValue(0);
}

void Plot::keyPressEvent(QKeyEvent *event)
{
    if(!m_imgnamelists.isEmpty()&&event->key()==Qt::Key_S&&event->modifiers()==Qt::ControlModifier){
        save();
        return;
    }
    if(!m_imgnamelists.isEmpty()&&(event->key()==Qt::Key_S || event->key() == Qt::Key_Right)){
            nextImg();
            return;
    }
    if(!m_imgnamelists.isEmpty()&&event->key()==Qt::Key_Down){
        downSelectRect();
        return;
    }
    if(!m_imgnamelists.isEmpty()&&(event->key()==Qt::Key_W || event->key() == Qt::Key_Left)){
        preImg();
        return;
    }
    if(!m_imgnamelists.isEmpty()&&event->key()==Qt::Key_Up){
        upSelectRect();
        return;
    }
    if(!m_imgnamelists.isEmpty()&&event->key()==Qt::Key_Delete){
        deleteRect();
        return;
    }

    QWidget::keyPressEvent(event);
}

void Plot::moveRectLine(QPoint &point1, QPoint &point2)
{
    int interval = 20;
    QPoint movepoint;
    movepoint.setX(m_movepoint.x() / m_img.getScale()[0]);
    movepoint.setY(m_movepoint.y() / m_img.getScale()[1]);
    if(abs(movepoint.x()-point1.x())<interval &&
       abs(movepoint.y()-point1.y())<interval){
        this->setCursor(Qt::SizeFDiagCursor);
        if(m_moverectpoint.x()!=0||m_moverectpoint.y()!=0){
            point1.setX(m_moverectpoint.x());
            point1.setY(m_moverectpoint.y());
            m_pairpoint.clear();
        }
    }else if(abs(movepoint.x()-point2.x())<interval &&
             abs(movepoint.y()-point2.y())<interval){
        this->setCursor(Qt::SizeFDiagCursor);
        if(m_moverectpoint.x()!=0||m_moverectpoint.y()!=0){
            point2.setX(m_moverectpoint.x());
            point2.setY(m_moverectpoint.y());
            m_pairpoint.clear();
        }
    }else if(abs(movepoint.x()-point2.x())<interval &&
             abs(movepoint.y()-point1.y())<interval){
        this->setCursor(Qt::SizeBDiagCursor);
        if(m_moverectpoint.x()!=0||m_moverectpoint.y()!=0){
            point2.setX(m_moverectpoint.x());
            point1.setY(m_moverectpoint.y());
            m_pairpoint.clear();
        }
    }else if(abs(movepoint.x()-point1.x())<interval &&
             abs(movepoint.y()-point2.y())<interval){
        this->setCursor(Qt::SizeBDiagCursor);
        if(m_moverectpoint.x()!=0||m_moverectpoint.y()!=0){
            point1.setX(m_moverectpoint.x());
            point2.setY(m_moverectpoint.y());
            m_pairpoint.clear();
        }
    }else if(abs(movepoint.x() - point1.x())<interval &&
             movepoint.y() >= point1.y() && movepoint.y() <= point2.y()){
        this->setCursor(Qt::SizeHorCursor);
        if(m_moverectpoint.x()!=0||m_moverectpoint.y()!=0){
            point1.setX(m_moverectpoint.x());
            m_pairpoint.clear();
        }
    }else if(abs(movepoint.y() - point1.y())<interval &&
             movepoint.x() >= point1.x() && movepoint.x() <= point2.x()){
        this->setCursor(Qt::SizeVerCursor);
        if(m_moverectpoint.x()!=0||m_moverectpoint.y()!=0){
            point1.setY(m_moverectpoint.y());
            m_pairpoint.clear();
        }
    }else if(abs(movepoint.x() - point2.x())<interval &&
             movepoint.y() >= point1.y() && movepoint.y() <= point2.y()){
        this->setCursor(Qt::SizeHorCursor);
        if(m_moverectpoint.x()!=0||m_moverectpoint.y()!=0){
            point2.setX(m_moverectpoint.x());
            m_pairpoint.clear();
        }
    }else if(abs(movepoint.y() - point2.y())<interval &&
             movepoint.x() >= point1.x() && movepoint.x() <= point2.x()){
        this->setCursor(Qt::SizeVerCursor);
        if(m_moverectpoint.x()!=0||m_moverectpoint.y()!=0){
            point2.setY(m_moverectpoint.y());
            m_pairpoint.clear();
        }
    }else{
        this->setCursor(Qt::ArrowCursor);
    }
}

void Plot::setRectinf(){
    m_rectinf.label = m_labels.getCurrentLabel();
    QPoint temp_point;
    temp_point.setX(m_pairpoint[0].x() / m_img.getScale()[0]);
    temp_point.setY(m_pairpoint[0].y() / m_img.getScale()[1]);
    m_rectinf.minPoint = temp_point;
    temp_point.setX(m_pairpoint[1].x() / m_img.getScale()[0]);
    temp_point.setY(m_pairpoint[1].y() / m_img.getScale()[1]);
    m_rectinf.maxPoint = temp_point;
    //m_rectinf.width  = m_img.getWidth();
    //m_rectinf.height = m_img.getHeight();
    m_rects.insert(m_rectinf);
    m_pairpoint.clear();
}

void Plot::changeSelectRectlabel(){
    int id = ui->rectsTable->currentRow();
    if(id!=-1){
        RectInf& t_rect = m_rects.selectRect(id, m_painter, m_img.getScale());
        t_rect.label = m_labels.getCurrentLabel();
        m_rects.setRowInf(id, t_rect);
    }
}

void Plot::downSelectRect(){
    int id = ui->rectsTable->currentRow();
    if(id == ui->rectsTable->rowCount() - 1){
        nextImg();
        return;
    }
    ui->rectsTable->setCurrentCell(id + 1, 0);
}

void Plot::upSelectRect(){
    int id = ui->rectsTable->currentRow();
    if(id == 0 || id == -1){
        preImg();
        return;
    }
    ui->rectsTable->setCurrentCell(id - 1, 0);
}
