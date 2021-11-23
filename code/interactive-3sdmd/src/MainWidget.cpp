#include "MainWidget.hpp"

#include <QMainWindow>

#include <QMessageBox>
#include <QImageReader>
#include <QImageWriter>
#include <QImage>
#include <QGuiApplication>
#include <QDir>
#include <QStatusBar>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QAction>
#include <ImageViewerWidget/ImageViewerWidget.hpp>

#include <QDockWidget>

#include <QDebug>

#include "MainWindow.hpp"

MainWidget::MainWidget(QWidget *parent)
  :QWidget{parent}
{ 
  namespace iv = ImageViewerWidget; 
  QLayout *mainLayout = new QVBoxLayout;

  imageViewer_ = new iv::ImageViewerWidget{this};
  
  Interactive_sdmd = new InteractiveSdmd();

  createDockWidgetSdmd();

  mainLayout->addWidget(imageViewer_);

  //imageViewer_->scrollAreaWidget()->viewport()->installEventFilter(this);

  setLayout(mainLayout);

}

bool MainWidget::loadImage(const QString &filename)
{
  
  QImageReader reader{filename};
  reader.setAutoTransform(true);
  const QImage newImage = reader.read();
  
  if (newImage.isNull()) {
    QMessageBox::information(this, QGuiApplication::applicationDisplayName(), 
      tr("Cannot load %1: %2").arg(QDir::toNativeSeparators(filename), reader.errorString()));
    return false;
  }
  
  imageViewer_->setImage(newImage);
  
  Interactive_sdmd->setImage(newImage);
  const char *c_str = filename.toLocal8Bit().data();
  Interactive_sdmd->readIntoSdmd(c_str);
  
  return true;
}

bool MainWidget::saveImage(const QString &filename)
{
  QImageWriter writer(filename);

  if (!writer.write(imageViewer_->image())) {
    QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
      tr("Cannot write %1: %2").arg(QDir::toNativeSeparators(filename), writer.errorString()));
    return false;
  }

  return true;
}

void MainWidget::zoomOut()
{
  imageViewer_->zoomOut();
}

void MainWidget::zoomIn()
{
  imageViewer_->zoomIn();
}


void MainWidget::createDockWidgetSdmd()
{
  dockWidgetSdmd_ = new QDockWidget{"3S-DMD processing", 
    qobject_cast<QMainWindow*>(parent())};
    
  dockWidgetSdmd_->setVisible(false);
  dockWidgetSdmd_->setFloating(true);  
  dockWidgetSdmd_->setWidget(Interactive_sdmd);
 
}

