/*
#pragma once 

#include <QWidget>
#include <QPointF>
#include <QSize>
#include <QList>

class QLabel;
class QScrollArea;
class QImage;
class QScrollBar;
class QMouseEvent;

namespace ImageViewerWidget
{
  class ImageViewerWidget : public QWidget
  {
    Q_OBJECT
  public:
    ImageViewerWidget(QWidget *parent=nullptr);

    inline QImage& image() { return image_; };
    inline const QImage& image() const { return image_; }
    inline const QImage& overlayImage() const { return overlayImage_; }
    inline const QImage& resultImage() const { return resultImage_; }

    bool saveImage(const QString &fileName);
    bool loadImage(const QString &fileName);

    void setImage(const QImage &newImage);

    inline QSize normalImageSize() const { return normalImageSize_; }
    inline int normalImageWidth() const { return normalImageSize_.width(); }
    inline int normalImageHeight() const { return normalImageSize_.height(); }
    inline QScrollArea *scrollAreaWidget() { return scrollArea_; }

    void fitToWidget(); 
    void unfitToWidget();    

    void normalSize();    
    void zoomIn();
    void zoomOut();   
    void scaleImage(double factor); 

    inline double scaleFactor() const { return scaleFactor_; }

    void setOverlayImage(const QImage &secondImage);
    void removeOverlay();

    void performImageComposition();

  protected:
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mouseDoubleClickEvent(QMouseEvent *e) override;

  public:
  signals:
    void imageMousePress(const QPointF &p);
    void imageMouseRelease(const QPointF &p);
    void imageMouseDoubleClick(const QPointF &p);

  private:
    void adjustScrollBar(QScrollBar *scrollBar, double factor);
    
    QPointF scrollAreaPointMap(const QPointF &p) const;
    QPointF normalisePoint(const QPointF &p) const;
    QPointF mapScrollPointToImagePoint(const QPointF &p_normalised) const;


  private:
    QSize normalImageSize_;
    QImage image_;
    QLabel *imageLabel_;
    QScrollArea *scrollArea_;
    double scaleFactor_;
    QImage overlayImage_;

    QImage resultImage_;
  };
}
*/


#ifndef SCRIBBLEWIDGET_H
#define SCRIBBLEWIDGET_H

#include <QColor>
#include <QImage>
#include <QPoint>
#include <QWidget>

//! [0]
class scribbleWidget : public QWidget
{
    Q_OBJECT

public:
    scribbleWidget(QWidget *parent = 0);
    //QSize sizeHint() const override;
    bool openImage(const QString &fileName);
    bool saveImage(const QString &fileName, const char *fileFormat);
    void setImage(const QImage &newImage);
    
    //void saliencyProcess();
    
    inline QImage& image() { return Image; };
    void setPenColor(const QColor &newColor);
    void setPenWidth(int newWidth);

    bool isModified() const { return modified; }
    QColor penColor() const { return myPenColor; }
    int penWidth() const { return myPenWidth; }
    inline void setEllipseMode(bool state) {darwEllipseMode = state;}

public slots:
    void clearImage();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void drawLineTo(const QPoint &endPoint);
    void resizeImage(QImage *image, const QSize &newSize);

    bool modified;
    bool scribbling;
    int myPenWidth;
    QColor myPenColor;
    QImage Image;
    QPoint lastPoint;
    bool darwEllipseMode = false;
};
//! [0]

#endif
