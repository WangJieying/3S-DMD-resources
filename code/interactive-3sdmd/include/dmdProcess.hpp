#pragma once

#include <sstream>
#include <stdio.h>
#include <skeleton_cuda.hpp>

#include <BSplineCurveFitterWindow3.h>
using namespace std;
class dmdProcess {
  public:
    
    dmdProcess();
    ~dmdProcess();
    void readFromFile (const char *c_str){
      filename = c_str;
      OriginalImage = FIELD<float>::read(c_str); 
      nPix = OriginalImage->dimX() * OriginalImage->dimY();
    }

    inline void setProcessedImage(FIELD<float> *pimg) 
    { 
      OriginalImage  = pimg;
      nPix = OriginalImage->dimX() * OriginalImage->dimY();
    }

    //inline FIELD<float> *curImage() { return processedImage; }

    //
    void removeIslands(float islandThreshold);
    FIELD<float>* GenerateSaliencyMap(FIELD<float> *sm);
    void LayerSelection(bool cumulative, int num_layers);
    int computeSkeletons(float saliency_threshold, float hausdorff);
    void Init_indexingSkeletons();
    int indexingSkeletons(FIELD<float> * CC, int intensity, int index);
    //vector<int> getIntensityOfNode(){return IntensityOfNode;}
    multimap<int,int> getInty_Node(){return Inty_node;}
    vector<int> get_gray_levels() {return gray_levels;}
    
    //
    void find_layers(int clear_color, double* importance_upper, double width);
    void calculateImportance(bool cumulative, int num_layers);
    void removeLayers();
    int CalculateCPnum(int i, FIELD<float> *imDupeCurr, int WriteToFile, float hausdorff);

    int clear_color;

  private:
    const char *filename;
    FIELD<float>* processedImage, *OriginalImage;
    int nPix;
    double *importance;
    BSplineCurveFitterWindow3 spline;
    vector<int> gray_levels;
    multimap<int,int> Inty_node;
    FIELD<float>   *sm_;
    //stringstream ofBuffer;
};

