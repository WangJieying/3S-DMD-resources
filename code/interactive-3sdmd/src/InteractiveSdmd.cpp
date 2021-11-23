
//#include "MainWidget.hpp"
#include "InteractiveSdmd.hpp"

#include <QHBoxLayout>
#include <QVBoxLayout>

#include <QIcon>
#include <QDockWidget>
#include <QAction>

#include <QSpacerItem>
#include <QSizePolicy>
#include <QCheckBox>
#include <QDebug>
#include <QPushButton>
#include <QColorDialog>
#include <QInputDialog>
#include <QFileDialog>

float max(float x,float y) 		{ return (x<y)? y : x; }
float min(float x,float y)		{ return (x<y)? x : y; } 

InteractiveSdmd::InteractiveSdmd()
{
  QLayout *mainLayout = new QVBoxLayout;

  QLayout *SaliencybtnLayout = createSaliencyButtons();

  mainLayout->addItem(SaliencybtnLayout);

  QLayout *ToolLayout = new QHBoxLayout;

  QLayout *thresholdLayout = createSdmdControls();
  QLayout *btnLayout = createRunButtons();

  ToolLayout->addItem(thresholdLayout);
  ToolLayout->addItem(btnLayout);

  mainLayout->addItem(ToolLayout);
  
  //imageViewer_ = new ImageViewerWidget{this};
  scribble_ = new scribbleWidget();
  mainLayout->addWidget(scribble_);

  bar = new QStatusBar;
  bar->showMessage(tr("Set the thresholds and press the Run button."));
  mainLayout->addWidget(bar);

  setLayout(mainLayout);
  
}
QLayout *InteractiveSdmd::createRunButtons()
{
  QLayout *btnLayout = new QVBoxLayout;

  QCheckBox *checkBox = new QCheckBox(this);
  checkBox->setText("Interp");
  connect(checkBox, SIGNAL(stateChanged(int)), this, SLOT(onStateChanged(int)));
  btnLayout->addWidget(checkBox);

  //QPushButton *skelRecBtn = new QPushButton{ QIcon{":/images/Skel_icon.png"}, "", this};
  //skelRecBtn->setIconSize(QSize{50, 50});
  QPushButton *skelRecBtn = new QPushButton();
  skelRecBtn->setText("Run");
  skelRecBtn->setFixedSize(QSize{50, 30});
  connect(skelRecBtn, &QPushButton::clicked, this, &InteractiveSdmd::RunBtn_press);
  
  btnLayout->addWidget(skelRecBtn);

  return btnLayout;
}
QLayout *InteractiveSdmd::createSaliencyButtons()
{
    QLayout *btnLayout = new QHBoxLayout;

    QPushButton *colorBtn = new QPushButton{ QIcon{":/images/color_icon.png"}, "", this};
    colorBtn->setIconSize(QSize{32, 32});
    connect(colorBtn, &QPushButton::clicked, this, &InteractiveSdmd::colorBtn_press);
    
    QPushButton *widthBtn = new QPushButton{ QIcon{":/images/width_icon.png"}, "", this};
    widthBtn->setIconSize(QSize{32, 32});
    connect(widthBtn, &QPushButton::clicked, this, &InteractiveSdmd::widthBtn_press);

    QPushButton *clearBtn = new QPushButton{ QIcon{":/images/clear_screen_icon.png"}, "", this};
    clearBtn->setIconSize(QSize{32, 32});
    connect(clearBtn, &QPushButton::clicked, this, &InteractiveSdmd::clearBtn_press);
    
    QPushButton *OpenImgBtn = new QPushButton{ QIcon{":/images/open_file_icon.png"}, "", this};
    OpenImgBtn->setIconSize(QSize{32, 32});
    connect(OpenImgBtn, &QPushButton::clicked, this, &InteractiveSdmd::openImgBtn_press);
    /*
    QPushButton *saliencyBtn = new QPushButton{ QIcon{":/images/saliency_icon.png"}, "", this};
    saliencyBtn->setIconSize(QSize{32, 32});
    connect(saliencyBtn, &QPushButton::clicked, this, &InteractiveSdmd::saliencyBtn_press);
    */
    QPushButton *DrawEllipseBtn = new QPushButton{ QIcon{":/images/ellipse_icon.png"}, "", this};
    DrawEllipseBtn->setCheckable(true);
    DrawEllipseBtn->setIconSize(QSize{32, 32});
    connect(DrawEllipseBtn, &QPushButton::toggled, this, &InteractiveSdmd::DrawEllipseBtn_toggled);
    
    QPushButton *zoomIn = new QPushButton{ QIcon{":/images/image_zoom_in_icon.png"}, "", this};
    zoomIn->setIconSize(QSize{32, 32});
    connect(zoomIn, &QPushButton::clicked, this, &InteractiveSdmd::zoomIn_press);
    
    QPushButton *zoomOut = new QPushButton{ QIcon{":/images/image_zoom_out_icon.png"}, "", this};
    zoomOut->setIconSize(QSize{32, 32});
    connect(zoomOut, &QPushButton::clicked, this, &InteractiveSdmd::zoomOut_press);
    

    btnLayout->addWidget(OpenImgBtn);
    btnLayout->addWidget(colorBtn);
    btnLayout->addWidget(widthBtn);
    btnLayout->addWidget(DrawEllipseBtn);
    btnLayout->addWidget(clearBtn);
    btnLayout->addWidget(zoomIn);
    btnLayout->addWidget(zoomOut);
    //btnLayout->addWidget(saliencyBtn);

    return btnLayout;
}
QLayout *InteractiveSdmd::createSdmdControls()
{  
  QLayout *controlsLayout = new QVBoxLayout;
  QHBoxLayout *nleavesLayout = new QHBoxLayout;  
  QHBoxLayout *IslandsLayout = new QHBoxLayout;
  QHBoxLayout *saliencyLayout = new QHBoxLayout;
  QHBoxLayout *HDLayout = new QHBoxLayout;

  // -------------------- number of layers (extinction filter) ------------------------
  nleavesLayout->addWidget(new QLabel{" L:   ", this});
  numberLayersSlider_ = new QSlider{Qt::Horizontal, this};
  numberLayersSlider_->setRange(1, 50);
  layerVal = 12;
  numberLayersSlider_->setValue(layerVal);
  numberLayersValueLabel_ = new QLabel(QString::number(layerVal), this);
  numberLayersValueLabel_->setFixedWidth(35);
  numberLayersValueLabel_->setAlignment(Qt::AlignHCenter);
  connect(numberLayersSlider_, &QSlider::sliderMoved, this,
    &InteractiveSdmd::numberLayersSlider_onValueChange);
  
  nleavesLayout->addWidget(numberLayersSlider_);  
  nleavesLayout->addWidget(numberLayersValueLabel_);

  controlsLayout->addItem(nleavesLayout);

  // ------------------------- Islands ------------------------------------------------

 QString str = tr(" ");
 str.append(QChar(0x3b5)); 
 str.append(tr(":   ")); 
 QLabel* label = new QLabel;
 label->setText(str);

  IslandsLayout->addWidget(label);
  
  IslandsSlider_ = new QSlider{Qt::Horizontal, this};
  IslandsSlider_->setRange(1,100);//actually /1000, i.e.(0.001, 0.1);
  IslandsVal = 0.01;
  IslandsSlider_->setValue(IslandsVal*1000);
  IslandsValueLabel_ = new QLabel{QString::number(IslandsVal), this};
  IslandsValueLabel_->setFixedWidth(50);
  IslandsValueLabel_->setAlignment(Qt::AlignHCenter);
  connect(IslandsSlider_, &QSlider::sliderMoved, this, 
    &InteractiveSdmd::IslandsSlider_onValueChange);

  IslandsLayout->addWidget(IslandsSlider_);
  IslandsLayout->addWidget(IslandsValueLabel_);

  controlsLayout->addItem(IslandsLayout);

  // --------------- Saliency -----------------------------------------
 QString str1 = tr(" ");
 str1.append(QChar(0x3b4)); 
 str1.append(tr(":   ")); 
 QLabel* label1 = new QLabel;
 label1->setText(str1);

  saliencyLayout->addWidget(label1);

  SaliencySlider_ = new QSlider{Qt::Horizontal, this};
  SaliencySlider_->setRange(1, 200);
  SaliencyVal = 0.3;
  SaliencySlider_->setValue(SaliencyVal*100);
  SaliencyValueLabel_ = new QLabel{QString::number(SaliencyVal), this};
  SaliencyValueLabel_->setFixedWidth(50);
  SaliencyValueLabel_->setAlignment(Qt::AlignHCenter);
  connect(SaliencySlider_, &QSlider::sliderMoved, this,
    &InteractiveSdmd::SkelSaliencySlider_onValueChange);

  saliencyLayout->addWidget(SaliencySlider_);
  saliencyLayout->addWidget(SaliencyValueLabel_);

  controlsLayout->addItem(saliencyLayout);

// --------------- HD filter -----------------------------------------
  //HDLayout->addWidget(new QLabel{"Hausdorff: ", this});
  QString str2 = tr(" ");
 str2.append(QChar(0x3b3)); 
 str2.append(tr(":   ")); 
 QLabel* label2 = new QLabel;
 label2->setText(str2);

  HDLayout->addWidget(label2);

  HDSlider_ = new QSlider{Qt::Horizontal, this};
  HDSlider_->setRange(1, 20);//actually /2000, i.e.(0.0005, 0.01);
  HDVal = 0.002;
  HDSlider_->setValue(HDVal*2000);
  HDLabel_ = new QLabel{QString::number(HDVal), this};
  HDLabel_->setFixedWidth(50);
  HDLabel_->setAlignment(Qt::AlignHCenter);
  connect(HDSlider_, &QSlider::sliderMoved, this,
    &InteractiveSdmd::HDSlider_onValueChange);

  HDLayout->addWidget(HDSlider_);
  HDLayout->addWidget(HDLabel_);

  controlsLayout->addItem(HDLayout);
  return controlsLayout;
}
void InteractiveSdmd::onStateChanged(int state)
{
  if(state == 2) InterpState = true;//checked
  else InterpState = false;//0-unchecked

}
void InteractiveSdmd::RunBtn_press()
{
  /*int CPnum;
  
  bar->showMessage(tr("Removing Islands..."));
  //auto begin = std::chrono::steady_clock::now();
  dmdProcess_.removeIslands(IslandsVal, nullptr);
  auto end = std::chrono::steady_clock::now();
    auto diff_seconds = std::chrono::duration_cast<std::chrono::seconds>(end - begin).count();
    auto diff_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
    std::cout << "Removing Islands took = " << diff_seconds << " s (" << diff_ms << " ms)" << std::endl;
  
  bar->showMessage(tr("Selecting layers..."));
  dmdProcess_.LayerSelection(true, layerVal);
  bar->showMessage(tr("Computing Skeletons..."));
  CPnum = dmdProcess_.computeSkeletons(SaliencyVal, HDVal, nullptr);

  bar->showMessage(tr("Reading Control points..."));
  dmdRecon_.readControlPoints(scribble_->image().width(), scribble_->image().height(), dmdProcess_.clear_color, dmdProcess_.get_gray_levels());
  bar->showMessage(tr("Reconstruction..."));
  cout<<"InterpState: "<<InterpState<<endl;
  dmdRecon_.ReconstructImage(InterpState);
  
  bar->showMessage("Reconstruction finished! Total CPs: " + QString::number(CPnum));

  QImage img = fieldToImage(dmdRecon_.getOutput());    
  setImage(img);
   
  const QString filename = "../images/R.pgm"; 
  const char *c_str = filename.toLocal8Bit().data();
  FIELD<float> *ImageR = FIELD<float>::read(c_str); 
  
   const QString filename1 = "../images/G.pgm"; 
  const char *c_str1 = filename1.toLocal8Bit().data();
  FIELD<float> *ImageG = FIELD<float>::read(c_str1); 

  const QString filename2 = "../images/B.pgm"; 
  const char *c_str2 = filename2.toLocal8Bit().data();
  FIELD<float> *ImageB = FIELD<float>::read(c_str2); 

  QVector<QRgb>  colorTable;
  int r, g, b;           
  for (float* y = ImageR->data(), *u = ImageG->data(), *v = ImageB->data(), *yend = ImageR->data() + ImageR->dimX() * ImageR->dimY(); y < yend; ++y, ++u, ++v) {
        
        r = min(max(0, round((*y)                      + 1.402  * ((*v) - 128))), 255);
        g = min(max(0, round((*y) - 0.3441 * ((*u) - 128) - 0.7141 * ((*v) - 128))), 255);
        b = min(max(0, round((*y) + 1.772  * ((*u) - 128)                     )), 255);
        colorTable.push_back( qRgb(r,g,b) );
   
    }
  
  QImage ColorImage = QImage(ImageR->dimX(),ImageR->dimY(),QImage::Format_RGB32);
  ColorImage.setColorTable(colorTable);

  //QImage img = fieldToImage(ImageR);    
  setImage(ColorImage);*/

  int CPnum;
  FIELD<float> *scribble_sm = imageToField(scribble_->image());
  //scribble_sm->NewwritePGM("sm.pgm");
  FIELD<float> *Generated_sm = dmdProcess_.GenerateSaliencyMap(scribble_sm);
  QImage img = fieldToImage(Generated_sm);    
  setImage(img);//display

  bar->showMessage(tr("Removing Islands..."));
  dmdProcess_.removeIslands(IslandsVal);//scribble_sm has been modified.

  bar->showMessage(tr("Selecting layers..."));
  dmdProcess_.LayerSelection(true, layerVal);

  bar->showMessage(tr("Computing Skeletons..."));
  CPnum = dmdProcess_.computeSkeletons(SaliencyVal, HDVal);
  //CPnum = dmdProcess_.computeSkeletons(SaliencyVal, HDVal, nullptr);
  //
  bar->showMessage(tr("Reading Control points..."));
  dmdRecon_.readControlPoints(scribble_->image().width(), scribble_->image().height(), dmdProcess_.clear_color, dmdProcess_.get_gray_levels());
  bar->showMessage(tr("Reconstruction..."));
  dmdRecon_.ReconstructImage(InterpState);
  //dmdRecon_.ReconstructImage(false);
  
  bar->showMessage("Reconstruction finished! Total CPs: " + QString::number(CPnum));

  img = fieldToImage(dmdRecon_.getOutput());    
  setImage(img);
}

void InteractiveSdmd::saliencyBtn_press(){
  
  /*int CPnum;
  FIELD<float> *scribble_sm = imageToField(scribble_->image());
  //scribble_sm->NewwritePGM("sm.pgm");
  bar->showMessage(tr("Removing Islands..."));
   dmdProcess_.removeIslands(IslandsVal, scribble_sm);//scribble_sm has been modified.

  //scribble_->saliencyProcess();
  bar->showMessage(tr("Selecting layers..."));
  dmdProcess_.LayerSelection(true, layerVal);
  bar->showMessage(tr("Computing Skeletons..."));
  CPnum = dmdProcess_.computeSkeletons(SaliencyVal, HDVal, scribble_sm);
  //CPnum = dmdProcess_.computeSkeletons(SaliencyVal, HDVal, nullptr);
  //
  bar->showMessage(tr("Reading Control points..."));
  dmdRecon_.readControlPoints(scribble_->image().width(), scribble_->image().height(), dmdProcess_.clear_color, dmdProcess_.get_gray_levels());
  bar->showMessage(tr("Reconstruction..."));
  dmdRecon_.ReconstructImage(InterpState);
  //dmdRecon_.ReconstructImage(false);
  
  bar->showMessage("Reconstruction finished! Total CPs: " + QString::number(CPnum));

  QImage img = fieldToImage(dmdRecon_.getOutput());    
  setImage(img);*/
}

void InteractiveSdmd::DrawEllipseBtn_toggled(bool checked){
  if(checked) scribble_->setEllipseMode(true);
  else scribble_->setEllipseMode(false);
}

void InteractiveSdmd::clearBtn_press(){
  scribble_->clearImage();
}
void InteractiveSdmd::openImgBtn_press(){
  QString fileName = QFileDialog::getOpenFileName(this,
                                   tr("Open File"), QDir::currentPath());
  if (!fileName.isEmpty())
      scribble_->openImage(fileName);
}

void InteractiveSdmd::zoomIn_press(){

}
void InteractiveSdmd::zoomOut_press(){
  
}
void InteractiveSdmd::colorBtn_press(){
  QColor newColor = QColorDialog::getColor(scribble_->penColor());
  if (newColor.isValid())
      scribble_->setPenColor(newColor);
}
void InteractiveSdmd::widthBtn_press(){
  bool ok;
  int newWidth = QInputDialog::getInt(this, tr("Scribble"),
                                      tr("Select pen width:"),
                                      scribble_->penWidth(),
                                      1, 100, 50, &ok);
  if (ok)
      scribble_->setPenWidth(newWidth);
}
void InteractiveSdmd::numberLayersSlider_onValueChange(int val)
{  
  layerVal = val;
  numberLayersValueLabel_->setText(QString::number(val));
}

void InteractiveSdmd::IslandsSlider_onValueChange(int val)
{
  IslandsVal = val / 1000.0;
  IslandsValueLabel_->setText(QString::number(IslandsVal));
}

void InteractiveSdmd::SkelSaliencySlider_onValueChange(int val)
{
  SaliencyVal = val / 100.0;
  SaliencyValueLabel_->setText(QString::number(SaliencyVal));
}

void InteractiveSdmd::HDSlider_onValueChange(int val)
{
  HDVal = val / 2000.0;
  HDLabel_->setText(QString::number(HDVal));
}

QImage InteractiveSdmd::fieldToImage(FIELD<float> *fimg) const
{
  QImage img{fimg->dimX(), fimg->dimY(), QImage::Format_Grayscale8};
  float *fimg_data = fimg->data();
  uchar *img_data = img.bits();

  int N = fimg->dimX() * fimg->dimY();
  for (int i = 0; i < N; ++i)
    img_data[i] = static_cast<uchar>(fimg_data[i]);
 
  return img;
}

FIELD<float> *InteractiveSdmd::imageToField(QImage img) const
{
  
  FIELD<float> *fimg = new FIELD<float>{ 
    static_cast<int>(img.width()), static_cast<int>(img.height()) };

  float *fimg_data = fimg->data();
  uchar *img_data = img.bits();

  int N = img.width() * img.height();
  for (int i = 0; i < N; ++i)
    fimg_data[i] = static_cast<float>(img_data[i]);
  
  //fimg->NewwritePGM("new.pgm");
  return fimg;
}
