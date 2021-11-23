#include <sstream>
//#include "ManipulateCPs/ManipulateCPs.hpp"
#include "dmdReconstruct.hpp"

FIELD<float>* prev_layer;
FIELD<float>* prev_layer_dt;
bool firstTime;
int CurrNode;

dmdReconstruct::dmdReconstruct() {
    printf("dmdReconstruct....\n");
    //RunOnce = 1;
}
/*
dmdReconstruct::~dmdReconstruct() {
    deallocateCudaMem_recon();
}*/

vector<vector<Vector3<float>>> dmdReconstruct::GetCPs(int nodeID){
    auto Cps = IndexingCP.at(IndexingCP.size() - nodeID);
    return Cps;
}
void dmdReconstruct::reconFromMovedCPs(int inty, vector<vector<Vector3<float>>> CPlist)
{
    vector<Vector3<float>> movedSample;
    BSplineCurveFitterWindow3 movedSpline;
    movedSample = movedSpline.ReadIndexingSpline(CPlist);//loadSample();
    
    initOutput(0);
    renderMovedLayer(inty, movedSample);

    //update IndexingCP.
    IndexingCP[IndexingCP.size() - CurrNode] = CPlist;
    //update IndexingSample
    IndexingSample[IndexingSample.size() - CurrNode] = movedSample;
}

void dmdReconstruct::renderMovedLayer(int intensity, vector<Vector3<float>> SampleForEachCC){
    
    program.link();
    program.bind();
//    ==============DRAWING TO THE FBO============

    contextFunc->glBindFramebuffer(GL_FRAMEBUFFER, buffer);

    contextFunc->glEnable(GL_DEPTH_TEST);
    contextFunc->glClear(GL_DEPTH_BUFFER_BIT);
    contextFunc->glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    contextFunc->glClear(GL_COLOR_BUFFER_BIT);


    QOpenGLBuffer vertexPositionBuffer(QOpenGLBuffer::VertexBuffer);
    vertexPositionBuffer.create();
    vertexPositionBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    vertexPositionBuffer.bind();

    GLfloat width_2 = (GLfloat)width/2.0;
    GLfloat height_2 = (GLfloat)height/2.0;

    //read each skeleton point
    float x, y, r;
    Vector3<float> EachSample;
    
    for(auto it = SampleForEachCC.begin();it!=SampleForEachCC.end();it++){
        EachSample = *it;
        x = EachSample[0];
        y = height - EachSample[1] - 1;
        r = EachSample[2]; 

        float vertexPositions[] = {
        (x-r)/width_2 - 1,   (y-r)/height_2 - 1,
        (x-r)/width_2 - 1,   (y+r+1)/height_2 - 1,
        (x+r+1)/width_2 - 1, (y+r+1)/height_2 - 1,
        (x+r+1)/width_2 - 1, (y-r)/height_2 - 1,
        };

            
        vertexPositionBuffer.allocate(vertexPositions, 8 * sizeof(float));
        
        program.enableAttributeArray("position");
        program.setAttributeBuffer("position", GL_FLOAT, 0, 2);
        
        program.setUniformValue("r", (GLfloat)r);
        
        program.setUniformValue("x0", (GLfloat)x);
        
        program.setUniformValue("y0", (GLfloat)y);

        contextFunc->glDrawArrays(GL_QUADS, 0, 4);

    }
       
    //========SAVE IMAGE===========
        float *data = (float *) malloc(width * height * sizeof (float));
        
        contextFunc->glEnable(GL_TEXTURE_2D);
        contextFunc->glBindTexture(GL_TEXTURE_2D, tex);

        // Altering range [0..1] -> [0 .. 255] 
        contextFunc->glReadPixels(0, 0, width, height, GL_RED, GL_FLOAT, data);


        for (unsigned int x = 0; x < width; ++x) 
            for (unsigned int y = 0; y < height; ++y)
            {
                unsigned int y_ = height - 1 -y;

                if(*(data + y * width + x))
                    output->set(x, y_, intensity);
            }
        
        free(data);
    output->NewwritePGM("output.pgm");  
    program.release();
    vertexPositionBuffer.release();
    contextFunc->glBindFramebuffer(GL_FRAMEBUFFER, 0); 
}

void dmdReconstruct::openglSetup(){

    openGLContext.setFormat(surfaceFormat);
    openGLContext.create();
    if(!openGLContext.isValid()) qDebug("Unable to create context");

    surface.setFormat(surfaceFormat);
    surface.create();
    if(!surface.isValid()) qDebug("Unable to create the Offscreen surface");

    openGLContext.makeCurrent(&surface);
                 
//    ========GEOMEETRY SETUP========

    program.addShaderFromSourceCode(QOpenGLShader::Vertex,
                                   "#version 330\r\n"
                                   "in vec2 position;\n"
                                   "void main() {\n"
                                   "    gl_Position = vec4(position, 0.0, 1.0);\n"
                                   "}\n"
                                   );
    program.addShaderFromSourceCode(QOpenGLShader::Fragment,
                                   "#version 330\r\n"
                                   "uniform float r;\n"
                                   "uniform float x0;\n"
                                   "uniform float y0;\n"
                                   "void main() {\n"
                                   "float trans_x = gl_FragCoord.x;\n"
                                   "float trans_y = gl_FragCoord.y;\n"
                                   "    float alpha = ((trans_x-x0) * (trans_x-x0) + (trans_y-y0) * (trans_y-y0)) <= r*r ? 1.0 : 0.0;\n"
                                   "    gl_FragColor = vec4(alpha,alpha,alpha,alpha);\n"
                                   "    gl_FragDepth = 1.0-alpha;\n"
                                   "}\n"
                                   );
  
}


void dmdReconstruct::framebufferSetup(){

    contextFunc =  openGLContext.functions();
    //generate Framebuffer;
    contextFunc->glGenFramebuffers(1, &buffer);
    contextFunc->glBindFramebuffer(GL_FRAMEBUFFER, buffer);

    //generate tex and bind to Framebuffer;
    contextFunc->glGenTextures(1, &tex);
    contextFunc->glBindTexture(GL_TEXTURE_2D, tex);
    contextFunc->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
                    GL_RGBA, GL_UNSIGNED_BYTE, NULL); 
    contextFunc->glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    contextFunc->glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
    contextFunc->glBindTexture(GL_TEXTURE_2D, 0);
        
    contextFunc->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                    GL_TEXTURE_2D, tex, 0);
/// Attach depth buffer 
    contextFunc->glGenRenderbuffers(1, &depthbuffer);
    contextFunc->glBindRenderbuffer(GL_RENDERBUFFER, depthbuffer);
    contextFunc->glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    contextFunc->glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthbuffer);
       
    contextFunc->glViewport(0,0, width, height);

    contextFunc->glBindFramebuffer(GL_FRAMEBUFFER, 0);                                
  
}


void dmdReconstruct::renderLayer(int intensity_index){
    program.link();
    program.bind();

//    ==============DRAWING TO THE FBO============

    contextFunc->glBindFramebuffer(GL_FRAMEBUFFER, buffer);

    contextFunc->glEnable(GL_DEPTH_TEST);
    contextFunc->glClear(GL_DEPTH_BUFFER_BIT);
    contextFunc->glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    contextFunc->glClear(GL_COLOR_BUFFER_BIT);


    QOpenGLBuffer vertexPositionBuffer(QOpenGLBuffer::VertexBuffer);
    vertexPositionBuffer.create();
    vertexPositionBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    vertexPositionBuffer.bind();

    GLfloat width_2 = (GLfloat)width/2.0;
    //program.setUniformValue("width_2", width_2);
    GLfloat height_2 = (GLfloat)height/2.0;
    //program.setUniformValue("height_2", height_2);

    //read each skeleton point
    float x, y, r;
    vector<Vector3<float>> SampleForEachInty;
    Vector3<float> EachSample;
    
    SampleForEachInty = IndexingSample_interactive.at(intensity_index);
    for(auto it = SampleForEachInty.begin();it!=SampleForEachInty.end();it++){
        EachSample = *it;
        x = EachSample[0];
        y = height - EachSample[1] - 1;
        r = EachSample[2]; 

        float vertexPositions[] = {
        (x-r)/width_2 - 1,   (y-r)/height_2 - 1,
        (x-r)/width_2 - 1,   (y+r+1)/height_2 - 1,
        (x+r+1)/width_2 - 1, (y+r+1)/height_2 - 1,
        (x+r+1)/width_2 - 1, (y-r)/height_2 - 1,
        };

            
        vertexPositionBuffer.allocate(vertexPositions, 8 * sizeof(float));
        
        program.enableAttributeArray("position");
        program.setAttributeBuffer("position", GL_FLOAT, 0, 2);
        
        program.setUniformValue("r", (GLfloat)r);
        
        program.setUniformValue("x0", (GLfloat)x);
        
        program.setUniformValue("y0", (GLfloat)y);

        contextFunc->glDrawArrays(GL_QUADS, 0, 4);

    }
       cout<<gray_levels.at(intensity_index)<<"\t";
    //========SAVE IMAGE===========
        float *data = (float *) malloc(width * height * sizeof (float));
        
        contextFunc->glEnable(GL_TEXTURE_2D);
        contextFunc->glBindTexture(GL_TEXTURE_2D, tex);

        // Altering range [0..1] -> [0 .. 255] 
        contextFunc->glReadPixels(0, 0, width, height, GL_RED, GL_FLOAT, data);


        for (unsigned int x = 0; x < width; ++x) 
            for (unsigned int y = 0; y < height; ++y)
            {
                unsigned int y_ = height - 1 -y;

                if(*(data + y * width + x))
                    output->set(x, y_, gray_levels.at(intensity_index));
            }
        
        free(data);
      
    program.release();
    vertexPositionBuffer.release();
    contextFunc->glBindFramebuffer(GL_FRAMEBUFFER, 0); 

}


FIELD<float>* dmdReconstruct::renderLayer_interp(int i){
    
    program.link();
    program.bind();

//    ==============DRAWING TO THE FBO============

    contextFunc->glBindFramebuffer(GL_FRAMEBUFFER, buffer);

    contextFunc->glEnable(GL_DEPTH_TEST);
    contextFunc->glClear(GL_DEPTH_BUFFER_BIT);
    contextFunc->glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    contextFunc->glClear(GL_COLOR_BUFFER_BIT);


    QOpenGLBuffer vertexPositionBuffer(QOpenGLBuffer::VertexBuffer);
    vertexPositionBuffer.create();
    vertexPositionBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    vertexPositionBuffer.bind();

    GLfloat width_2 = (GLfloat)width/2.0;
    //program.setUniformValue("width_2", width_2);
    GLfloat height_2 = (GLfloat)height/2.0;
    //program.setUniformValue("height_2", height_2);

    //read each skeleton point
    float x, y, r;
    vector<Vector3<float>> SampleForEachInty;
    Vector3<float> EachSample;
    
    SampleForEachInty = IndexingSample_interactive.at(i);
    for(auto it = SampleForEachInty.begin();it!=SampleForEachInty.end();it++){
        EachSample = *it;
        x = EachSample[0];
        y = height - EachSample[1] - 1;
        r = EachSample[2]; 

        float vertexPositions[] = {
        (x-r)/width_2 - 1,   (y-r)/height_2 - 1,
        (x-r)/width_2 - 1,   (y+r+1)/height_2 - 1,
        (x+r+1)/width_2 - 1, (y+r+1)/height_2 - 1,
        (x+r+1)/width_2 - 1, (y-r)/height_2 - 1,
        };

            
        vertexPositionBuffer.allocate(vertexPositions, 8 * sizeof(float));
        
        program.enableAttributeArray("position");
        program.setAttributeBuffer("position", GL_FLOAT, 0, 2);
        
        program.setUniformValue("r", (GLfloat)r);
        
        program.setUniformValue("x0", (GLfloat)x);
        
        program.setUniformValue("y0", (GLfloat)y);

        contextFunc->glDrawArrays(GL_QUADS, 0, 4);

    }
       
    //========SAVE IMAGE===========

    float *data = (float *) malloc(width * height * sizeof (float));
    
    contextFunc->glEnable(GL_TEXTURE_2D);
    contextFunc->glBindTexture(GL_TEXTURE_2D, tex);

    // Altering range [0..1] -> [0 .. 255] 
    contextFunc->glReadPixels(0, 0, width, height, GL_RED, GL_FLOAT, data);

    FIELD<float>* CrtLayer = new FIELD<float>(width, height);
    
    for (unsigned int x = 0; x < width; ++x) 
        for (unsigned int y = 0; y < height; ++y)
        {
            unsigned int y_ = height - 1 -y;

            CrtLayer->set(x, y_, 0); //ensure that the init value is 0 everywhere.

            if(*(data + y * width + x))
                CrtLayer->set(x, y_, 1);
        
        }

    
    free(data);
    program.release();
    vertexPositionBuffer.release();
    contextFunc->glBindFramebuffer(GL_FRAMEBUFFER, 0); 

    return CrtLayer;

}

void dmdReconstruct::readControlPoints(int width_, int height_, int clear_color, vector<int> gray_levels_){
    //cout<<"gray_levels_ size: "<<gray_levels_.size()<<endl;
    BSplineCurveFitterWindow3 readCPs;
    IndexingSample_interactive = readCPs.SplineGenerate();//loadSample();
    
    width = width_;
    height = height_;
    clearColor = clear_color;
    gray_levels = gray_levels_;
    if (RunOnce) {openglSetup(); RunOnce = false;}
    framebufferSetup(); 
    initialize_skeletonization_recon(width, height);//initCUDA
    
}

void dmdReconstruct::readIndexingControlPoints(int width_, int height_, int clear_color, multimap<int,int> Inty_Node){
   /**/ BSplineCurveFitterWindow3 readCPs;
    IndexingSample = readCPs.ReadIndexingSpline();//loadIndexingSample();
    IndexingCP = readCPs.get_indexingCP();
    width = width_;
    height = height_;
    clearColor = clear_color;
    Inty_node = Inty_Node;
    if (RunOnce) {openglSetup(); RunOnce = false;}
    framebufferSetup(); 
    initialize_skeletonization_recon(width, height);//initCUDA
    
}

void dmdReconstruct::initOutput(int clear_color) {
    output = new FIELD<float>(width, height);
    prev_layer = new FIELD<float>(width, height);
    prev_layer_dt = new FIELD<float>(width, height);
   
    
    for (unsigned int x = 0; x < width; x++) 
        for (unsigned int y = 0; y < height; y++)
            output->set(x, y, clear_color);

}

FIELD<float>* dmdReconstruct::get_dt_of_alpha(FIELD<float>* alpha) {
    auto alpha_dupe = alpha->dupe();
    
    auto dt = computeCUDADT(alpha_dupe);
    return dt;
}
   
  
void dmdReconstruct::get_interp_layer(int i, int SuperResolution, bool last_layer)
{
    bool interp_firstLayer = 1;
    int prev_intensity, prev_bound_value, curr_bound_value;
    unsigned int x, y;
    
    int intensity = gray_levels.at(i);
    FIELD<float>* curr_layer = renderLayer_interp(i);
    //debug
     /*//if(firstTime || last_layer){
    stringstream skel;
    skel<<"out/s"<<intensity<<".pgm";
    curr_layer->writePGM(skel.str().c_str());
    //}
    */
    FIELD<float>* curr_dt = get_dt_of_alpha(curr_layer);
    

 if(interp_firstLayer) {

    if(firstTime){//first layer
        FIELD<float>* first_layer_forDT = new FIELD<float>(width, height);
        firstTime = false;
        prev_intensity = clearColor;
        prev_bound_value = clearColor;
        for (int i = 0; i < width; ++i) 
            for (int j = 0; j < height; ++j){
                if(i==0 || j==0 || i==width-1 || j==height-1)    first_layer_forDT -> set(i, j, 0); 
                else first_layer_forDT -> set(i, j, 1); 
                prev_layer -> set(i, j, 1);
            }

        prev_layer_dt = get_dt_of_alpha(first_layer_forDT);
    }
    
    curr_bound_value = (prev_intensity + intensity)/2;

    for (x = 0; x < width; ++x) 
        for (y = 0; y < height; ++y)
        {
            if(!curr_layer->value(x, y) && prev_layer->value(x, y))
            {// If there are pixels active between boundaries we smoothly interpolate them

                    float prev_dt_val = prev_layer_dt->value(x, y);
                    float curr_dt_val = curr_dt->value(x, y);

                    float interp_alpha = prev_dt_val / ( prev_dt_val + curr_dt_val);
                    float interp_color = curr_bound_value * interp_alpha + prev_bound_value *  (1 - interp_alpha);

                    output->set(x, y, interp_color);

            }
        }

    //CUDA_interp(curr_layer, prev_layer, prev_layer_dt, curr_dt, curr_bound_value, prev_bound_value, false, 0);

     }
    else{
          if(firstTime){ 
              firstTime = false; 
              //CUDA_interp(curr_layer, prev_layer, prev_layer_dt, curr_dt, curr_bound_value, clearColor, true, 0);//draw clear_color
             
              curr_bound_value = (clearColor + intensity)/2;
          }
          else{
            curr_bound_value = (prev_intensity + intensity)/2;
            //CUDA_interp(curr_layer, prev_layer, prev_layer_dt, curr_dt, curr_bound_value, prev_bound_value, false, 0);
            for (x = 0; x < width; ++x) 
                for (y = 0; y < height; ++y)
                {
                    if(!curr_layer->value(x, y) && prev_layer->value(x, y))// If there are pixels active between boundaries we smoothly interpolate them
                    { 
                        float prev_dt_val = prev_layer_dt->value(x, y);
                        float curr_dt_val = curr_dt->value(x, y);

                        float interp_alpha = prev_dt_val / ( prev_dt_val + curr_dt_val);
                        float interp_color = curr_bound_value * interp_alpha + prev_bound_value *  (1 - interp_alpha);

                        output->set(x, y, interp_color);
                    }
                }
        }   
     }

    prev_bound_value = curr_bound_value;

    if(last_layer){
        //output = CUDA_interp(curr_layer, prev_layer, prev_layer_dt, curr_dt, curr_bound_value, prev_bound_value, false, intensity);
        
        for (x = 0; x < width; ++x) 
            for (y = 0; y < height; ++y){
                if(curr_layer->value(x, y))
                {
                    int interp_last_layer_value = prev_bound_value + (curr_dt->value(x, y)/5);
                    int MaxIntensity = (intensity+10) > 255 ? 255 : (intensity+10);
                    output->set(x, y, (interp_last_layer_value > MaxIntensity) ? MaxIntensity : interp_last_layer_value);

                }
            }
    }

    prev_intensity = intensity;
    prev_layer = curr_layer->dupe();
    prev_layer_dt = curr_dt->dupe();

    delete curr_layer;
    delete curr_dt;
}

void dmdReconstruct::ReconstructImage(bool interpolate){
    firstTime = true;
    initOutput(clearColor);
    if(!gray_levels.empty())
    {
        for (int i = 0; i < gray_levels.size(); i++) {
            if(interpolate){
                bool last_layer = false;
                int SuperResolution = 1;
                int max_level = *std::max_element(gray_levels.begin(), gray_levels.begin() + gray_levels.size());

                if(gray_levels.at(i) == max_level) last_layer = true;
                get_interp_layer(i, SuperResolution, last_layer);

            }
            else{
                renderLayer(i);
            } 
        }

        output->NewwritePGM("output.pgm");
        printf("DMD finished!\n");

    }
    else printf("gray_levels is empty!");

}

FIELD<float>* dmdReconstruct::renderLayer_interp(int intensity, int nodeID){
   
    program.link();
    program.bind();

//    ==============DRAWING TO THE FBO============

    contextFunc->glBindFramebuffer(GL_FRAMEBUFFER, buffer);

    contextFunc->glEnable(GL_DEPTH_TEST);
    contextFunc->glClear(GL_DEPTH_BUFFER_BIT);
    contextFunc->glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    contextFunc->glClear(GL_COLOR_BUFFER_BIT);


    QOpenGLBuffer vertexPositionBuffer(QOpenGLBuffer::VertexBuffer);
    vertexPositionBuffer.create();
    vertexPositionBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    vertexPositionBuffer.bind();

    GLfloat width_2 = (GLfloat)width/2.0;
    //program.setUniformValue("width_2", width_2);
    GLfloat height_2 = (GLfloat)height/2.0;
    //program.setUniformValue("height_2", height_2);

    //read each skeleton point
    float x, y, r;
    vector<Vector3<float>> SampleForEachCC;
    Vector3<float> EachSample;
    
    auto it1 = Inty_node.lower_bound(intensity);
    auto it2 = Inty_node.upper_bound(intensity);
    while(it1 != it2){//process all nodes for the current intensity
        int node_id = it1->second;
        ++it1;
        if(node_id != nodeID){
            SampleForEachCC = IndexingSample.at(IndexingSample.size() - node_id);
            for(auto it = SampleForEachCC.begin();it!=SampleForEachCC.end();it++){
                EachSample = *it;
                x = EachSample[0];
                y = height - EachSample[1] - 1;
                r = EachSample[2]; 

                float vertexPositions[] = {
                (x-r)/width_2 - 1,   (y-r)/height_2 - 1,
                (x-r)/width_2 - 1,   (y+r+1)/height_2 - 1,
                (x+r+1)/width_2 - 1, (y+r+1)/height_2 - 1,
                (x+r+1)/width_2 - 1, (y-r)/height_2 - 1,
                };

                    
                vertexPositionBuffer.allocate(vertexPositions, 8 * sizeof(float));
                
                program.enableAttributeArray("position");
                program.setAttributeBuffer("position", GL_FLOAT, 0, 2);
                
                program.setUniformValue("r", (GLfloat)r);
                
                program.setUniformValue("x0", (GLfloat)x);
                
                program.setUniformValue("y0", (GLfloat)y);

                contextFunc->glDrawArrays(GL_QUADS, 0, 4);

            }
        }
    }

    //========SAVE IMAGE===========
       
    float *data = (float *) malloc(width * height * sizeof (float));
    
    contextFunc->glEnable(GL_TEXTURE_2D);
    contextFunc->glBindTexture(GL_TEXTURE_2D, tex);

    // Altering range [0..1] -> [0 .. 255] 
    contextFunc->glReadPixels(0, 0, width, height, GL_RED, GL_FLOAT, data);

    FIELD<float>* CrtLayer = new FIELD<float>(width, height);
    
    for (unsigned int x = 0; x < width; ++x) 
        for (unsigned int y = 0; y < height; ++y)
        {
            unsigned int y_ = height - 1 -y;

            CrtLayer->set(x, y_, 0); //ensure that the init value is 0 everywhere.

            if(*(data + y * width + x))
                CrtLayer->set(x, y_, 1);
        
        }

    
    free(data);
    program.release();
    vertexPositionBuffer.release();
    contextFunc->glBindFramebuffer(GL_FRAMEBUFFER, 0); 

    return CrtLayer;


}


void dmdReconstruct::get_interp_layer(int intensity, int nodeID, int SuperResolution, bool last_layer)
{
    bool interp_firstLayer = 0;
    int prev_intensity, prev_bound_value, curr_bound_value;
    unsigned int x, y;
    
    FIELD<float>* curr_layer = renderLayer_interp(intensity, nodeID);
    
    FIELD<float>* curr_dt = get_dt_of_alpha(curr_layer);
   if(firstTime || last_layer){
    stringstream layer;
    layer<<"out/l"<<intensity<<".pgm";
    curr_layer->writePGM(layer.str().c_str());}
     /**/
    
 if(interp_firstLayer) {
    if(firstTime){//first layer
    FIELD<float>* first_layer_forDT = new FIELD<float>(width, height);
        firstTime = false;
        prev_intensity = clearColor;
        prev_bound_value = clearColor;
        for (int i = 0; i < width; ++i) 
            for (int j = 0; j < height; ++j){
                if(i==0 || j==0 || i==width-1 || j==height-1)    first_layer_forDT -> set(i, j, 0); 
                else first_layer_forDT -> set(i, j, 1); 
                prev_layer -> set(i, j, 1);
            }

        prev_layer_dt = get_dt_of_alpha(first_layer_forDT);
    }
    
    curr_bound_value = (prev_intensity + intensity)/2;
    for (x = 0; x < width; ++x) 
        for (y = 0; y < height; ++y)
        {
            if(!curr_layer->value(x, y) && prev_layer->value(x, y))
            {// If there are pixels active between boundaries we smoothly interpolate them

                float prev_dt_val = prev_layer_dt->value(x, y);
                float curr_dt_val = curr_dt->value(x, y);

                float interp_alpha = prev_dt_val / ( prev_dt_val + curr_dt_val);
                float interp_color = curr_bound_value * interp_alpha + prev_bound_value *  (1 - interp_alpha);

                output->set(x, y, interp_color);
            }
        }

    //CUDA_interp(curr_layer, prev_layer, prev_layer_dt, curr_dt, curr_bound_value, prev_bound_value, false, 0);

     }
     else{
          if(firstTime){ 
              firstTime = false;
              //CUDA_interp(curr_layer, prev_layer, prev_layer_dt, curr_dt, curr_bound_value, clearColor, true, 0);//draw clear_color
              
              curr_bound_value = (clearColor + intensity)/2;
          }
          else{
            curr_bound_value = (prev_intensity + intensity)/2;
            //CUDA_interp(curr_layer, prev_layer, prev_layer_dt, curr_dt, curr_bound_value, prev_bound_value, false, 0);
            for (x = 0; x < width; ++x) 
                for (y = 0; y < height; ++y)
                {
                    if(!curr_layer->value(x, y) && prev_layer->value(x, y))// If there are pixels active between boundaries we smoothly interpolate them
                    { 
                        float prev_dt_val = prev_layer_dt->value(x, y);
                        float curr_dt_val = curr_dt->value(x, y);

                        float interp_alpha = prev_dt_val / ( prev_dt_val + curr_dt_val);
                        float interp_color = curr_bound_value * interp_alpha + prev_bound_value *  (1 - interp_alpha);

                        output->set(x, y, interp_color);
                    }
                }
        }   
     }

    prev_bound_value = curr_bound_value;

    if(last_layer){
       // output = CUDA_interp(curr_layer, prev_layer, prev_layer_dt, curr_dt, curr_bound_value, prev_bound_value, false, intensity);
        for (x = 0; x < width; ++x) 
            for (y = 0; y < height; ++y){
                if(curr_layer->value(x, y))
                {
                    int interp_last_layer_value = prev_bound_value + (curr_dt->value(x, y)/5);
                    int MaxIntensity = (intensity+10) > 255 ? 255 : (intensity+10);
                    output->set(x, y, (interp_last_layer_value > MaxIntensity) ? MaxIntensity : interp_last_layer_value);

                }
            }
    }

    prev_intensity = intensity;
    prev_layer = curr_layer->dupe();
    prev_layer_dt = curr_dt->dupe();

    delete curr_layer;
    delete curr_dt;
}

void dmdReconstruct::renderIndexingLayer(int intensity, int nodeID){
    //cout<<"nodeID: "<<nodeID<<endl;
   
    program.link();
    program.bind();

//    ==============DRAWING TO THE FBO============

    contextFunc->glBindFramebuffer(GL_FRAMEBUFFER, buffer);

    contextFunc->glEnable(GL_DEPTH_TEST);
    contextFunc->glClear(GL_DEPTH_BUFFER_BIT);
    contextFunc->glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    contextFunc->glClear(GL_COLOR_BUFFER_BIT);


    QOpenGLBuffer vertexPositionBuffer(QOpenGLBuffer::VertexBuffer);
    vertexPositionBuffer.create();
    vertexPositionBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    vertexPositionBuffer.bind();

    GLfloat width_2 = (GLfloat)width/2.0;
    //program.setUniformValue("width_2", width_2);
    GLfloat height_2 = (GLfloat)height/2.0;
    //program.setUniformValue("height_2", height_2);

    //read each skeleton point
    float x, y, r;
    vector<Vector3<float>> SampleForEachCC;
    Vector3<float> EachSample;
    
    auto it1 = Inty_node.lower_bound(intensity);
    auto it2 = Inty_node.upper_bound(intensity);
    while(it1 != it2){//process all nodes for the current intensity
        int node_id = it1->second;
        ++it1;
        if(node_id != nodeID){
            SampleForEachCC = IndexingSample.at(IndexingSample.size() - node_id);
            for(auto it = SampleForEachCC.begin();it!=SampleForEachCC.end();it++){
                EachSample = *it;
                x = EachSample[0];
                y = height - EachSample[1] - 1;
                r = EachSample[2]; 

                float vertexPositions[] = {
                (x-r)/width_2 - 1,   (y-r)/height_2 - 1,
                (x-r)/width_2 - 1,   (y+r+1)/height_2 - 1,
                (x+r+1)/width_2 - 1, (y+r+1)/height_2 - 1,
                (x+r+1)/width_2 - 1, (y-r)/height_2 - 1,
                };

                    
                vertexPositionBuffer.allocate(vertexPositions, 8 * sizeof(float));
                
                program.enableAttributeArray("position");
                program.setAttributeBuffer("position", GL_FLOAT, 0, 2);
                
                program.setUniformValue("r", (GLfloat)r);
                
                program.setUniformValue("x0", (GLfloat)x);
                
                program.setUniformValue("y0", (GLfloat)y);

                contextFunc->glDrawArrays(GL_QUADS, 0, 4);

            }
        }
    }

    //========SAVE IMAGE===========
        float *data = (float *) malloc(width * height * sizeof (float));
        
        contextFunc->glEnable(GL_TEXTURE_2D);
        contextFunc->glBindTexture(GL_TEXTURE_2D, tex);

        // Altering range [0..1] -> [0 .. 255] 
        contextFunc->glReadPixels(0, 0, width, height, GL_RED, GL_FLOAT, data);


        for (unsigned int x = 0; x < width; ++x) 
            for (unsigned int y = 0; y < height; ++y)
            {
                unsigned int y_ = height - 1 -y;

                if(*(data + y * width + x))
                    output->set(x, y_, intensity);
            }
        
        free(data);
      
    program.release();
    vertexPositionBuffer.release();
    contextFunc->glBindFramebuffer(GL_FRAMEBUFFER, 0); 

}

void dmdReconstruct::renderLayer(int intensity, int nodeID){
    //cout<<"nodeID: "<<nodeID<<endl;
   
    program.link();
    program.bind();

//    ==============DRAWING TO THE FBO============

    contextFunc->glBindFramebuffer(GL_FRAMEBUFFER, buffer);

    contextFunc->glEnable(GL_DEPTH_TEST);
    contextFunc->glClear(GL_DEPTH_BUFFER_BIT);
    contextFunc->glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    contextFunc->glClear(GL_COLOR_BUFFER_BIT);


    QOpenGLBuffer vertexPositionBuffer(QOpenGLBuffer::VertexBuffer);
    vertexPositionBuffer.create();
    vertexPositionBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    vertexPositionBuffer.bind();

    GLfloat width_2 = (GLfloat)width/2.0;
    //program.setUniformValue("width_2", width_2);
    GLfloat height_2 = (GLfloat)height/2.0;
    //program.setUniformValue("height_2", height_2);

    //read each skeleton point
    float x, y, r;
    vector<Vector3<float>> SampleForEachCC;
    Vector3<float> EachSample;
    
    SampleForEachCC = IndexingSample.at(IndexingSample.size() - nodeID);
    for(auto it = SampleForEachCC.begin();it!=SampleForEachCC.end();it++){
        EachSample = *it;
        x = EachSample[0];
        y = height - EachSample[1] - 1;
        r = EachSample[2]; 

        float vertexPositions[] = {
        (x-r)/width_2 - 1,   (y-r)/height_2 - 1,
        (x-r)/width_2 - 1,   (y+r+1)/height_2 - 1,
        (x+r+1)/width_2 - 1, (y+r+1)/height_2 - 1,
        (x+r+1)/width_2 - 1, (y-r)/height_2 - 1,
        };

            
        vertexPositionBuffer.allocate(vertexPositions, 8 * sizeof(float));
        
        program.enableAttributeArray("position");
        program.setAttributeBuffer("position", GL_FLOAT, 0, 2);
        
        program.setUniformValue("r", (GLfloat)r);
        
        program.setUniformValue("x0", (GLfloat)x);
        
        program.setUniformValue("y0", (GLfloat)y);

        contextFunc->glDrawArrays(GL_QUADS, 0, 4);

    }
       
    //========SAVE IMAGE===========
        float *data = (float *) malloc(width * height * sizeof (float));
        
        contextFunc->glEnable(GL_TEXTURE_2D);
        contextFunc->glBindTexture(GL_TEXTURE_2D, tex);

        // Altering range [0..1] -> [0 .. 255] 
        contextFunc->glReadPixels(0, 0, width, height, GL_RED, GL_FLOAT, data);


        for (unsigned int x = 0; x < width; ++x) 
            for (unsigned int y = 0; y < height; ++y)
            {
                unsigned int y_ = height - 1 -y;

                if(*(data + y * width + x))
                    output->set(x, y_, intensity);
            }
        
        free(data);
      
    program.release();
    vertexPositionBuffer.release();
    contextFunc->glBindFramebuffer(GL_FRAMEBUFFER, 0); 

}


void dmdReconstruct::ReconstructIndexingImage(bool interpolate, int nodeID, int action){
    if(nodeID > 0) CurrNode = nodeID;
    firstTime = true;
    if(action) initOutput(0);
    else initOutput(clearColor);
    if(!IndexingSample.empty()){
        
        int LastInty = 256;
        bool found = false;
        for (auto const& it : Inty_node) { //for each graylevel(from small num to large num).
            if(LastInty != it.first){
                LastInty = it.first;
                //cout<<"LastInty "<<LastInty<<"\t";
                if(action){//highlight
                    if(nodeID != 0){
                        auto it1 = Inty_node.lower_bound(it.first);
                        auto it2 = Inty_node.upper_bound(it.first);
                        while(it1 != it2){//process all nodes for the current intensity
                            int node_id = it1->second;
                            if(node_id == nodeID){
                                renderLayer(it.first, nodeID);
                                found = true;
                                break;
                            }
                            ++it1;
                        }
                        if(found) break;
                    }
                }
                else{//else delete the current node-CC
                    if(interpolate){
                        bool last_layer = false; 
                        int SuperResolution = 1;
                        auto [max_level, max] = *std::max_element(Inty_node.begin(), Inty_node.end());
                        //int max_level = std::max_element(Inty_node.begin(), Inty_node.end());
                        
                        if(it.first == (int)max_level) last_layer = true;
                        get_interp_layer(it.first, nodeID, SuperResolution, last_layer);
                
                    /**/
                    }
                    else renderIndexingLayer(it.first, nodeID);
                } 
            }
        }
        
        output->NewwritePGM("output.pgm");
        printf("DMD finished!\n");
    }

    else printf("gray_levels is empty!");

}
