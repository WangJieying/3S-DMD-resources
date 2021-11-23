
#include <QtWidgets>

#include "scribbleWidget.hpp"

//! [0]
scribbleWidget::scribbleWidget(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_StaticContents);
    modified = false;
    scribbling = false;
    myPenWidth = 50;
    myPenColor = Qt::blue;
    //myPenColor = QColor(0,0,255,20);
}
//! [0]

//! [1]
bool scribbleWidget::openImage(const QString &fileName)
//! [1] //! [2]
{
    QImage loadedImage;
    if (!loadedImage.load(fileName))
        return false;
    //QSize newSize = loadedImage.size().expandedTo(size());
    //QSize newSize = loadedImage.size();
    //printf("height: %d, width: %d. \n",newSize.height(),newSize.width());
    //resizeImage(&loadedImage, newSize);
    Image = loadedImage;
   
    //modified = false;
    update();
    return true;
}
void scribbleWidget::setImage(const QImage &newImage) 
{ 
  Image = newImage; 
  setFixedSize(Image.width(),Image.height());
  update();
}
//! [2]

//! [3]
bool scribbleWidget::saveImage(const QString &fileName, const char *fileFormat)
//! [3] //! [4]
{
    /*
    int w=image.width();
    int h=image.height();
    QImage diffImage(w, h, QImage::Format_Grayscale8);
    //QImage diffImage = image;
    
    for(int i=0;i<h;i++){
        QRgb *rgb1=(QRgb*)image.constScanLine(i);
        QRgb *rgb2=(QRgb*)originalImage.constScanLine(i);
        for(int j=0;j<w;j++){
            //diffImage.setPixel(i,j,qRgb(0, 0, 0));
            if(rgb1[j]!=rgb2[j])
                diffImage.setPixel(i,j,qRgb(255, 255, 255));
        }
    }
*/
    QImage visibleImage = Image;
    //resizeImage(&visibleImage, size());

    if (visibleImage.save(fileName, fileFormat)) {
        modified = false;
        return true;
    } else {
        return false;
    }
}
//! [4]

//! [5]
void scribbleWidget::setPenColor(const QColor &newColor)
//! [5] //! [6]
{
    myPenColor = newColor;
}
//! [6]

//! [7]
void scribbleWidget::setPenWidth(int newWidth)
//! [7] //! [8]
{
    myPenWidth = newWidth;
}
//! [8]

//! [9]
void scribbleWidget::clearImage()
//! [9] //! [10]
{
    Image.fill(qRgb(255, 255, 255));
    //modified = true;
    update();
}
//! [10]

//! [11]
void scribbleWidget::mousePressEvent(QMouseEvent *event)
//! [11] //! [12]
{
    if (event->button() == Qt::LeftButton) {
        lastPoint = event->pos();
        scribbling = true;
    }
}

void scribbleWidget::mouseMoveEvent(QMouseEvent *event)
{
  if(!darwEllipseMode)
    if ((event->buttons() & Qt::LeftButton) && scribbling)
        drawLineTo(event->pos());
}

void scribbleWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && scribbling) {
      if(darwEllipseMode){
        QPoint BottomRightPoint = event->pos();
        //update();
        QPainter painter(&Image);
        QRect rect(lastPoint, BottomRightPoint);
        painter.setPen(QPen(myPenColor));
        painter.setBrush(QBrush(myPenColor));
        painter.drawEllipse(rect);

        int rad = 2;
        update(QRect(lastPoint, BottomRightPoint).normalized()
                                        .adjusted(-rad, -rad, +rad, +rad));

      }
      else
        drawLineTo(event->pos());
        
      scribbling = false;
    }
}

//! [12] //! [13]
void scribbleWidget::paintEvent(QPaintEvent *event)
//! [13] //! [14]
{
    QPainter painter(this);
    QRect dirtyRect = event->rect();
    painter.drawImage(dirtyRect, Image, dirtyRect);

}
//! [14]

//! [15]

void scribbleWidget::resizeEvent(QResizeEvent *event)
//! [15] //! [16]
{
    /*if (width() > image.width() || height() > image.height()) {
        int newWidth = qMax(width() + 128, image.width());
        int newHeight = qMax(height() + 128, image.height());
        resizeImage(&image, QSize(newWidth, newHeight));
        update();
    }*/
    //printf("height: %d, width: %d. \n",Image.height(),Image.width());
    //printf("height: %d, width: %d. \n",height(), width());
    QWidget::resizeEvent(event);
}
//! [16]

//! [17]
void scribbleWidget::drawLineTo(const QPoint &endPoint)
//! [17] //! [18]
{
    QPainter painter(&Image);
    painter.setPen(QPen(myPenColor, myPenWidth, Qt::SolidLine, Qt::RoundCap,
                        Qt::RoundJoin));
    
    painter.drawLine(lastPoint, endPoint);
    modified = true;

    int rad = (myPenWidth / 2) + 2;
    update(QRect(lastPoint, endPoint).normalized()
                                     .adjusted(-rad, -rad, +rad, +rad));
    lastPoint = endPoint;
}
//! [18]

//! [19]
void scribbleWidget::resizeImage(QImage *image, const QSize &newSize)
//! [19] //! [20]
{
    if (image->size() == newSize)
        return;

    QImage newImage(newSize, QImage::Format_RGB32);
    newImage.fill(qRgb(255, 255, 255));
    QPainter painter(&newImage);
    painter.drawImage(QPoint(0, 0), *image);
    *image = newImage;
}
//! [20]
