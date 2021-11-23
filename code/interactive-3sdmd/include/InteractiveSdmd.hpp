#pragma once 

#include <QWidget>
#include <QImage>
#include <QMap>
#include <QLabel>
#include <QSlider>
#include <QDockWidget>
#include <QString>
#include <QStatusBar>
#include <ImageViewerWidget/ImageViewerWidget.hpp>
#include "dmdProcess.hpp"
#include "dmdReconstruct.hpp"
#include "scribbleWidget.hpp"

class scribbleWidget;

class InteractiveSdmd : public QWidget
{
Q_OBJECT

protected:
  using ImageViewerWidget = ImageViewerWidget::ImageViewerWidget;

public:

  InteractiveSdmd();
  inline void setImage(const QImage &newImage) 
  { 
    scribble_->setImage(newImage); 
    setFixedSize(newImage.width(),newImage.height()+160);
  }
  
  inline void readIntoSdmd(const char *c_str) { dmdProcess_.readFromFile(c_str); }
 
protected:  
  
  QImage fieldToImage(FIELD<float> *field) const; 
  FIELD<float> *imageToField(QImage img) const; 

  QLayout* createRunButtons();
  QLayout* createSaliencyButtons();
  QLayout* createSdmdControls();
  //ImageViewerWidget *imageViewer_;

protected slots:
   
  void RunBtn_press(); 
  void saliencyBtn_press();
  void colorBtn_press();
  void widthBtn_press();
  void clearBtn_press();
  void zoomIn_press();
  void zoomOut_press();
  void openImgBtn_press();
  void DrawEllipseBtn_toggled(bool checked);
 
  void numberLayersSlider_onValueChange(int val);
  void IslandsSlider_onValueChange(int val);
  void SkelSaliencySlider_onValueChange(int val);
  void HDSlider_onValueChange(int val);
  void onStateChanged(int state);

private:
  QSlider *numberLayersSlider_;
  QLabel *numberLayersValueLabel_;

  QSlider *IslandsSlider_;
  QLabel *IslandsValueLabel_;

  QSlider *SaliencySlider_, *HDSlider_;
  QSlider *HDLayout;
  QLabel *SaliencyValueLabel_, *HDLabel_;


  dmdProcess dmdProcess_;
  dmdReconstruct dmdRecon_;
  int layerVal;
  float IslandsVal, SaliencyVal, HDVal;
  QStatusBar *bar;
  scribbleWidget *scribble_;
  bool InterpState = false;
};