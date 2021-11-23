#pragma once 

#include <QMainWindow>
#include "MainWidget.hpp"

#include <QList>

class QDockWidget;
class QAction;
class QProgressBar;

class MainWindow : public QMainWindow
{
  Q_OBJECT 
public:
  MainWindow();

private slots:
  void saveAs();
  void open();

  void dmdProcessAct_onTrigged();
  void imageZoomInAct_onTrigged();
  void imageZoomOutAct_onTrigged();

private:
  void createMenus();  

  void createToolBar();
  

private:
  MainWidget *mainWidget_;
  
  QAction *imageZoomInAct_;
  QAction *imageZoomOutAct_;

  QAction *dmdProcessAct_;
};
