#pragma once 

#include <QWidget>
#include <QString>
#include <ImageViewerWidget/ImageViewerWidget.hpp>

#include "InteractiveSdmd.hpp"
#include <QDockWidget>

class MainWidget : public QWidget
{  
  Q_OBJECT
public:
  
  MainWidget(QWidget *parent=nullptr);

  bool loadImage(const QString& filename);
  bool saveImage(const QString& filename);

  const QImage& image() const { return imageViewer_->image(); }
  QImage& image() { return imageViewer_->image(); }
  

  QDockWidget *SdmdDockWidget() {return dockWidgetSdmd_;}

  void zoomOut();
  void zoomIn();


private:
  void createDockWidgetSdmd();
  
private:

  QDockWidget *dockWidgetSdmd_;

  ImageViewerWidget::ImageViewerWidget *imageViewer_;
  
  InteractiveSdmd *Interactive_sdmd;
};