#include "plot.h"
#include "ui_plot.h"
#include <QFileDialog>
#include <QDebug>
#include <QDir>
#include <QPainter>
#include <QMessageBox>
#include <qmimedata.h>
#include <sstream>
#include <string>
#include <fstream>


Plot::Plot(QWidget *parent) :
    QMainWindow(parent), ui(new Ui::plotUi)
{
    ui->setupUi(this);
    ui->display_image->installEventFilter(this);
	ui->display_image->setAcceptDrops(true);
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
    connect(ui->skip_line,SIGNAL(returnPressed()), this, SLOT(skipImg()));
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
    QString filepath=QFileDialog::getExistingDirectory(this, QStringLiteral("图像路径"), openDir);
    if(filepath.isEmpty()){
        return;
    }else{
        cacheList.clear();
        cacheList << filepath;
        m_cache.writer(cachePath, cacheList, false);
        ui->caption->setText(QStringLiteral("初始化..."));
        m_imgnamelists.clear();
		ui->fileLists->clear();
		m_imgid = 0;
		updateImgNameLists(filepath);
    }

}

bool Plot::eventFilter(QObject *watched, QEvent *event)
{
	if (watched == ui->display_image) {
		if (event->type() == QEvent::DragEnter) {
			QDragEnterEvent *dee = dynamic_cast<QDragEnterEvent *>(event);
			dee->acceptProposedAction();
			return true;
		}
		else if (event->type() == QEvent::Drop) {
			QDropEvent *de = dynamic_cast<QDropEvent *>(event);
			QList<QUrl> urls = de->mimeData()->urls();
			if (urls.isEmpty() || urls.count() > 1)
				return true;
			QFileInfo file(urls.first().toLocalFile());
			m_imgnamelists.clear();
			ui->fileLists->clear();
			m_imgid = 0;
			if (file.isDir()) {
                updateImgNameLists(file.filePath());
				update();
				return false;
			}
            m_img.setFileRoot(file.path());
			m_rects.setFileRoot(file.path());
			QString filename = file.fileName().toLower();
			if (filename.endsWith(".jpg") || filename.endsWith(".png") ||
				filename.endsWith(".jpeg") || filename.endsWith(".dcm") ||
				isDICM(file.filePath())) {
				m_imgnamelists.append(file.fileName());
				ui->fileLists->insertItem(0, file.filePath());
				ui->progressBar->setRange(0, m_imgnamelists.count() - 1);
				ui->skip_line->setText("1");
				updateInf();
				update();
				return false;
			}else{
				int index = filename.split(".").size();
				QMessageBox::information(this, QStringLiteral("提示"),
					tr("Can't load image type: *.") + filename.split(".")[index - 1]);
				return false;
			}			
		}
	}
	if(watched==ui->display_image&&event->type()==QEvent::Paint&&!m_imgnamelists.isEmpty()){
        m_painter.begin(ui->display_image);
        draw();
        update();
        m_painter.end();
        return false;
    }
    
    return QWidget::eventFilter(watched,event);
}

void Plot::wheelEvent(QWheelEvent *event)
{
    if(m_imgnamelists.isEmpty())
        return;
    int value = event->delta();
    if(value > 0)
        m_img.OnZoomInImage();
    else
        m_img.OnZoomoutImage();
}

void Plot::mousePressEvent(QMouseEvent *event)
{
    m_oldpoint=event->pos()-(ui->centralWidget->pos()+ui->widget->pos()+ui->display_image->pos());
    if(!m_imgnamelists.isEmpty()&&event->button()==Qt::RightButton){
        if(ui->rectsTable->currentRow() != -1){
            ui->rectsTable->setCurrentCell(-1, -1);
        }/*else{
            m_labels.addId();
        }*/

    }

}

void Plot::mouseMoveEvent(QMouseEvent *event)
{
    if(!m_imgnamelists.isEmpty())
    {
        QPoint movepoint=event->pos()-(ui->centralWidget->pos()+ui->widget->pos()+ui->display_image->pos());
        qreal zoomValue = m_img.getZoomValue();
        m_movepoint = (movepoint - m_img.getOffset()) / zoomValue;
        if(event->buttons()&Qt::LeftButton){
            int x = static_cast<int>(m_movepoint.x() / m_img.getScale()[0]);
            int y = static_cast<int>(m_movepoint.y() / m_img.getScale()[1]);
            x = x < m_img.getWidth() ? x : m_img.getWidth();
            x = x > 0 ? x : 0;
            y = y < m_img.getHeight() ? y : m_img.getHeight();
            y = y > 0 ? y : 0;
            m_moverectpoint.setX(static_cast<int>(x));
            m_moverectpoint.setY(static_cast<int>(y));
        }else {
            m_moverectpoint.setX(0);
            m_moverectpoint.setY(0);
        }

        if(event->buttons()&Qt::RightButton){
            QPoint pos = movepoint - m_oldpoint;
            m_img.addOffset(pos);
            m_oldpoint = movepoint;
        }
        emit mobilePoint(m_movepoint);
    }
}

void Plot::mouseReleaseEvent(QMouseEvent *event)
{
    if(!m_imgnamelists.isEmpty()&&event->button()==Qt::LeftButton)
    {
        QPoint release_point=event->pos()-(ui->centralWidget->pos()+ui->widget->pos()+ui->display_image->pos());
        qreal zoomValue = m_img.getZoomValue();
        release_point -= m_img.getOffset();
        release_point /= zoomValue;
        if(release_point.x()>=0&&release_point.x()<=m_img.getWidth()*m_img.getScale()[0]&&
           release_point.y()>=0&&release_point.y()<=m_img.getHeight()*m_img.getScale()[1])//判断点是否在图像内部
        {
            m_pairpoint.append(release_point);
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

        emit releasePoint(release_point);
    }
}

void Plot::draw()
{
    m_img.display(m_painter);
    m_painter.setPen(QPen(Qt::green,1,Qt::SolidLine));
    m_painter.drawLine(m_movepoint.x(),0,m_movepoint.x(),ui->display_image->height());
    m_painter.drawLine(0,m_movepoint.y(),ui->display_image->width(),m_movepoint.y());
    if(!m_pairpoint.isEmpty()){
        m_painter.setPen(QPen(Qt::red,2,Qt::SolidLine));
        m_painter.drawPoints(m_pairpoint);
    }
    m_rects.drawRects(m_painter, m_img.getScale());
    if(m_img.getIsDCM()){
        int ww = ui->brightness->value();
        int wl = ui->contrast->value();
        int y = ui->display_image->height() - 20;
        QString text = QString(" WL: ") +  QString::number(wl) + QString("  WW: ") + QString::number(ww);
        m_painter.setPen(QPen(Qt::yellow,18,Qt::SolidLine));//设置画笔形式
        m_painter.drawText(5,y,text);
    }
    int id = ui->rectsTable->currentRow();
    if(id!=-1){
        RectInf& t_rect = m_rects.selectRect(id, m_painter, m_img.getScale());
        moveRectLine(t_rect.minPoint, t_rect.maxPoint);
        m_rects.setRowInf(id, t_rect);
    }

}

void Plot::clickedFileLists(){
    if(ui->fileLists->currentRow() != m_imgid){
        if(ui->autosave->isChecked())
            save();
        m_imgid = ui->fileLists->currentRow();
        updateInf();
    }
}

void Plot::save()
{
    if(m_imgnamelists.isEmpty()){
        QMessageBox::information(this, QStringLiteral("提示"),
								 QStringLiteral("请打开文件夹!"));
        return;
    }
    ui->caption->setText(QStringLiteral("保存中..."));
    m_rects.save(m_imgnamelists[m_imgid]);
    ui->caption->setText(QStringLiteral("保存成功!"));
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
    if(! m_img.imread(m_imgnamelists[m_imgid])){
        QMessageBox::information(this, QStringLiteral("提示"),
                                 tr("Load Image: ") + m_imgnamelists[m_imgid] + tr(" Error!") );
       preImg();
    }
    recover();
    this->setCursor(Qt::ArrowCursor);
    m_pairpoint.clear();
    m_img.OnPresetImage();
    m_img.setOffset(0, 0);
	m_img.setIsUpdateInf(true);
    if(m_img.getIsDCM()){
        int ww = static_cast<int>(m_img.getOrgWW());
        int wl = static_cast<int>(m_img.getOrgWL());
        ui->brightness->setRange(ww-3000, ww+3000);
        ui->contrast->setRange(wl-3000, wl+3000);
        ui->brightness->setValue(ww);
        ui->contrast->setValue(wl);
        ui->brightness_label->setText(QStringLiteral("窗宽："));
        ui->contrast_label->setText(QStringLiteral("窗位："));
        ui->reset_brightness->setText(QStringLiteral("窗宽重置"));
        ui->reset_contrast->setText(QStringLiteral("窗位重置"));
    }else{
        ui->brightness->setRange(-100, 100);
        ui->contrast->setRange(-100, 100);
        ui->brightness->setValue(0);
        ui->contrast->setValue(0);
        ui->brightness_label->setText(QStringLiteral("亮度："));
        ui->contrast_label->setText(QStringLiteral("对比度："));
        ui->reset_brightness->setText(QStringLiteral("亮度重置"));
        ui->reset_contrast->setText(QStringLiteral("对比度重置"));
    }
	m_img.setIsUpdateInf(false);
}

void Plot::preImg()
{
    if(m_imgnamelists.isEmpty()){
        QMessageBox::information (this, QStringLiteral("提示"),
								  QStringLiteral("请打开文件夹!"));
        return;
    }
    if(m_imgid==0){
        QMessageBox::information (this, QStringLiteral("提示"),
								  QStringLiteral("这是第一张!"));
         return;

    }else {
        if(ui->autosave->isChecked())
            save();
        --m_imgid;
        updateInf();
    }
}

void Plot::nextImg()
{
    if(m_imgnamelists.isEmpty()){
        QMessageBox::information (this, QStringLiteral("提示"),
									QStringLiteral("请打开文件夹!"));
        return;
    }
    if(m_imgid==m_imgnamelists.count()-1)
    {
        QMessageBox::information (this, QStringLiteral("提示"),
								 QStringLiteral("已是最后一张!"));
        return;
    }else {
        if(ui->autosave->isChecked())
            save();
        ++m_imgid;
        updateInf();
    }
}

void Plot::skipImg()
{
    if(m_imgnamelists.isEmpty()){
        QMessageBox::information (this, QStringLiteral("提示"),
								  QStringLiteral("请打开文件夹!"));
        return;
    }
    if(ui->autosave->isChecked())
        save();
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
    if(ui->autosave->isChecked())
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
        //m_display_scale.setText(QString::number(brightness));
    }
}

void Plot::adjustContrast(int contrast){
    if(!m_imgnamelists.isEmpty()){
        m_img.adjustContrast(contrast);
    }
}

void Plot::resetBrightness(){
    if(!m_imgnamelists.isEmpty()){
        if(m_img.getIsDCM()){
            int ww = static_cast<int>(m_img.getOrgWW());
            ui->brightness->setValue(ww);
            return;
        }
    }
    ui->brightness->setValue(0);
}

void Plot::resetContrast(){
    if(!m_imgnamelists.isEmpty()){
        if(m_img.getIsDCM()){
            int wl = static_cast<int>(m_img.getOrgWL());
            ui->contrast->setValue(wl);
            return;
        }
    }
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
    if(!m_imgnamelists.isEmpty()&&event->key()==Qt::Key_0&&event->modifiers()==Qt::ControlModifier){
        m_img.OnPresetImage();
        m_img.setOffset(0, 0);
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

void Plot::updateImgNameLists(const QString& filepath)
{
	m_rects.setFileRoot(filepath);
	m_img.setFileRoot(filepath);
	QDir dir(filepath);
	QStringList tmp_imgnamelists = dir.entryList();

	for (int i = 0, k = 0; i < tmp_imgnamelists.size(); ++i) {
		if (tmp_imgnamelists[i].endsWith(".jpg") || tmp_imgnamelists[i].endsWith(".png") || 
			tmp_imgnamelists[i].endsWith(".jpeg") || tmp_imgnamelists[i].endsWith(".dcm") || 
			isDICM(QString(filepath + "/" + tmp_imgnamelists[i]))){
			m_imgnamelists.append(tmp_imgnamelists[i]);
			ui->fileLists->insertItem(k, filepath + "/" + m_imgnamelists[k++]);
		}
	}
	if (m_imgnamelists.isEmpty()) {
		QMessageBox::information(this, QStringLiteral("提示"),
			QStringLiteral("文件夹为空!"));
		return;
	}
	ui->progressBar->setRange(0, m_imgnamelists.count() - 1);
	ui->skip_line->setText("1");
	updateInf();
}

bool Plot::isDICM(const QString& file)
{
	QByteArray cdata = file.toLocal8Bit();
	std::string filepath(cdata);
	std::ifstream temp_file(filepath);
	std::string line;
	if (temp_file.is_open()) {
		std::getline(temp_file, line);
		if (line.size() < 132)
			return false;
		if (line.substr(128, 4) == std::string("DICM"))
			return true;
	}
	return false;
}



