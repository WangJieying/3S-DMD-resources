#include "include/DataManager.hpp"
#include "include/texture3D.hpp"
#include "include/framebuffer.hpp"
#include "include/io.hpp"
#include "include/messages.h"
#include "include/skeleton_cuda.hpp"
#include <math.h>
#include <chrono>
#define ARRAY_SIZE(arr) sizeof(arr)/sizeof(arr[0])
#define SET_TEXTURE(arr, i, val) do { (arr)[(4 * (i))] = (arr)[(4 * (i) + 1)] = (arr)[(4 * (i) + 2)] = (val); (arr)[(4 * (i) + 3)] = 255.0; } while(0);
FIELD<float>* prev_layer;
FIELD<float>* prev_layer_dt;

DataManager::DataManager() {
    dim = new int[2];
    fbo = (Framebuffer *) 0;
    
}

DataManager::~DataManager() {
    delete [] dim;
    delete dataLoc;
    delete fbo;
    delete sh_alpha;
    deallocateCudaMem();
}

void DataManager::initCUDA() {
    initialize_skeletonization(dim[0], dim[1]);
 
}
void DataManager::initOUT() {
    output = new FIELD<float>(dim[0], dim[1]);
    prev_layer = new FIELD<float>(dim[0], dim[1]);
    prev_layer_dt = new FIELD<float>(dim[0], dim[1]);
    //cout<<"clear_color00 "<<clear_color<<endl;
    //if(clear_color > 124 && clear_color< 130) clear_color = 128;
    for (unsigned int x = 0; x < dim[0]; x++) 
        for (unsigned int y = 0; y < dim[1]; y++)
            output->set(x, y, clear_color);
}


FIELD<float>* DataManager::get_texture_data() {

    unsigned char* data = (unsigned char*)(calloc(dim[0] * dim[1], sizeof(unsigned char)));
    //cout<<"here1"<<endl;
    glReadPixels(0, 0, dim[0], dim[1], GL_RED, GL_UNSIGNED_BYTE, data);
    
    FIELD<float>* f = new FIELD<float>(dim[0], dim[1]);
    
    for (int i = 0; i < dim[0] * dim[1]; ++i) {
        
        int x = i % dim[0];
        int y = i / dim[0];
        
        f->set(x, dim[1] - y - 1, data[i]);
        
      
    }
    free(data);
    data = NULL;
    return f;
}

// Computes the DT of the alpha map. It's settable if the DT needs to be computed
// of the foreground (i.e. pixels which are 0) or the background (i.e. pixels that are 1)
FIELD<float>* DataManager::get_dt_of_alpha(FIELD<float>* alpha, bool foreground) {
    auto alpha_dupe = alpha->dupe();
 
    auto ret = computeCUDADT(alpha_dupe, foreground);
   
    return ret;
}


void DataManager::get_interp_layer(int intensity, int prev_intensity, int superR, bool last_layer, int time1, int prev_invert, int invert) {
    bool interp_firstLayer = true;
    bool compute_dt_of_foreground = false;
    unsigned int x, y;
    int curr_bound_value;
    FIELD<float>* curr_alpha = getAlphaMapOfLayer(intensity, superR, time1, invert);

   
    FIELD<float>* curr_dt = get_dt_of_alpha(curr_alpha, !compute_dt_of_foreground);
    
    //if (prev_intensity == 0) curr_dt->NewwritePGM("first2.pgm");
    //FIELD<float>* prev_alpha = nullptr;
    FIELD<float>* prev_alpha = new FIELD<float>(dim[0], dim[1]);
    FIELD<float>* prev_alpha_forDT = new FIELD<float>(dim[0], dim[1]);
    
    FIELD<float>* prev_dt = nullptr;

    FIELD<float>* inward_dt = nullptr;
    if(last_layer){
        inward_dt = get_dt_of_alpha(curr_alpha, compute_dt_of_foreground);//inward
        //inward_dt->NewwritePGM("inward.pgm");
    }

    if(interp_firstLayer) {
        if (prev_intensity == 0) {

            prev_intensity = clear_color;
            prev_bound_value = clear_color;
            for (int i = 0; i < dim[0]; ++i) 
                for (int j = 0; j < dim[1]; ++j){
                    if(i==0 || j==0 || i==dim[0]-1 || j==dim[1]-1)    prev_alpha_forDT -> set(i, j, 0); 
                    else prev_alpha_forDT -> set(i, j, 1); 
                    prev_alpha -> set(i, j, 1);
                }

            prev_dt = get_dt_of_alpha(prev_alpha_forDT, compute_dt_of_foreground);
           
        }
        else
        {
            prev_alpha = prev_layer;
            //prev_alpha = getAlphaMapOfLayer(prev_intensity, superR, 4, prev_invert);
            prev_dt = prev_layer_dt;
            //prev_dt = get_dt_of_alpha(prev_alpha, compute_dt_of_foreground);
        }

    
        curr_bound_value = (prev_intensity + intensity)/2;
        prev_layer = curr_alpha->dupe();
        prev_layer_dt = curr_dt->dupe();
        
          //if (prev_intensity == clear_color) prev_dt->NewwritePGM("first.pgm");
        for (x = 0; x < dim[0]; ++x) 
            for (y = 0; y < dim[1]; ++y)
            {
                if(!curr_alpha->value(x, y) && prev_alpha->value(x, y))
                {// If there are pixels active between boundaries we smoothly interpolate them
                     //if(time1 == 1 && prev_intensity == clear_color);//Do not interpolation here.
                     //if(time1 >1 && prev_intensity>124 && prev_intensity<131)//cope with black background (i.e. Y:0, Cb:128, Cr:128)
                         //   output->set(x, y, 128); //Black background. (i.e. Y:0, Cb:128, Cr:128) //for single color background.
                        //else{
                        float prev_dt_val = prev_dt->value(x, y);
                        float curr_dt_val = curr_dt->value(x, y);

                        float interp_alpha = prev_dt_val / ( prev_dt_val + curr_dt_val);
                        float interp_color = curr_bound_value * interp_alpha + prev_bound_value *  (1 - interp_alpha);
                        
                        output->set(x, y, interp_color);
                    //}
                   
                }
            }
    } 
    else{   
        if(prev_intensity != 0){
            
            curr_bound_value = (prev_intensity + intensity)/2;
            prev_alpha = getAlphaMapOfLayer(prev_intensity, superR, 4, prev_invert);
            prev_dt = get_dt_of_alpha(prev_alpha, compute_dt_of_foreground);
            
            for (x = 0; x < dim[0]; ++x) 
                for (y = 0; y < dim[1]; ++y)
                {
                    if(!curr_alpha->value(x, y) && prev_alpha->value(x, y))// If there are pixels active between boundaries we smoothly interpolate them
                    {
                        //if(time1 > 1 && prev_intensity>124 && prev_intensity<131)//cope with black background (i.e. Y:0, Cb:128, Cr:128)
                            //output->set(x, y, 128); //Black background. (i.e. Y:0, Cb:128, Cr:128)
                        //else{
                            
                            float prev_dt_val = prev_dt->value(x, y);
                            float curr_dt_val = curr_dt->value(x, y);

                            float interp_alpha = prev_dt_val / ( prev_dt_val + curr_dt_val);
                            float interp_color = curr_bound_value * interp_alpha + prev_bound_value *  (1 - interp_alpha);
                            
                            output->set(x, y, interp_color);
                            
                        //}
                    }
                }
            
        } 
        else{   
            curr_bound_value = (clear_color + intensity)/2;
        } 
    
   }
    prev_bound_value = curr_bound_value;
    //process last layer. -Not sure if it's appropriate
         if(last_layer){
        //if(time1==1) inward_dt->NewwritePGM("inward.pgm");

        for (x = 0; x < dim[0]; ++x) 
            for (y = 0; y < dim[1]; ++y){
                if(curr_alpha->value(x, y))
                {
                    int interp_last_layer_value = prev_bound_value + (inward_dt->value(x, y)/10);
                    //if(time1==3) cout<<inward_dt->value(x, y)<<" "<<interp_last_layer_value<<" "<<intensity<<" ";
                    int MaxIntensity = (intensity+10) > 255 ? 255 : (intensity+10);
                    output->set(x, y, (interp_last_layer_value > MaxIntensity) ? MaxIntensity : interp_last_layer_value);
                
                }
              
            }
    }
    delete curr_alpha;
    delete prev_alpha;
    delete curr_dt;
    delete inward_dt;
    delete prev_dt;
}

/* This function use FT scheme to get each layer, but not precise. */
void DataManager::drawEachLayer(int l, int invert) {
    FIELD<float>* skel = new FIELD<float>(dim[0], dim[1]);
    auto data = skel->data();
    for (int i = 0; i < dim[0] * dim[1]; i++) {
        data[i] = 0;
    }
   
    layer_t *da = readLayer(l);
    int x, y, radius;

    for (unsigned int j = 0; j < da->size(); ++j) {
        x = (*da)[j].first;
        y = (*da)[j].second;
        radius = (*da)[j].third;

        skel->set(x, y, radius);

    }
    
    computeCUDADT(skel, 1);//skel has been changed to binary image.

    for (unsigned int x = 0; x < dim[0]; ++x) 
        for (unsigned int y = 0; y < dim[1]; ++y)
        {
            if(invert){
                if(!(skel->value(x,y)))
                    output->set(x, y, l);
            }
            else{
                if(skel->value(x,y))
                    output->set(x, y, l);
            }
            
        }
       
}


/* This function draws all disks. The alpha will be 1 at the location that should be drawn, otherwise 0. */
void DataManager::getAlphaMapOfLayer(int l, int invert) {
    /*FIELD<float>* skel = new FIELD<float>(dim[0], dim[1]);
    for (unsigned int x = 0; x < dim[0]; ++x) 
        for (unsigned int y = 0; y < dim[1]; ++y)
            skel->set(x, y, 0);
    */

    layer_t *da = readLayer(l);
    int x, y, radius;

    if (fbo == 0) {
        PRINT(MSG_ERROR, "Framebuffer object was not initialized. Creating new FBO (FIX THIS!)\n");
        initFBO();
    }

    /* Draw on a texture. */
    fbo->bind();
    if (SHADER == GLOBAL_ALPHA) {
        glClearColor(1.0, 1.0, 1.0, 1.0);
    }

    glEnable(GL_DEPTH_TEST);
    glClear(GL_DEPTH_BUFFER_BIT);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    sh_alpha->bind(); //draw circle rather than quads because of this shader.
    glBegin(GL_QUADS);
    for (unsigned int j = 0; j < da->size(); ++j) {
        x = (*da)[j].first;
        y = (*da)[j].second;
        radius = (*da)[j].third;

        //skel->set(x, y, 1);

        // Texture coordinates are in range [-1, 1]
        // Texcoord selects the texture coordinate
        // vertex call sets it to that location
       
         glTexCoord2f(-1.0, -1.0);
        glVertex2f(x - radius, y - radius);
        glTexCoord2f(-1.0, 1.0);
        glVertex2f(x - radius, y + radius+1);
        glTexCoord2f(1.0, 1.0);
        glVertex2f(x + radius+1, y + radius+1);
        glTexCoord2f(1.0, -1.0);
        glVertex2f(x + radius+1, y - radius);
        
    }
    glEnd();
    
    /*stringstream skelname;
    skelname<<"skel"<<l<<".pgm";
    skel->writePGM(skelname.str().c_str());
  
    delete skel;*/
    sh_alpha->unbind();
    fbo->unbind();

    float *data = (float *) malloc(dim[0] * dim[1] * sizeof (float));
    
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, fbo->texture->tex);

    // Altering range [0..1] -> [0 .. 255] 
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, data);

    unsigned int y_;
    for (unsigned int x = 0; x < dim[0]; ++x) 
        for (unsigned int y = 0; y < dim[1]; ++y)
        {
            y_ = dim[1] - 1 -y;
            if(invert){
                if(!*(data + y * dim[0] + x))
                    output->set(x, y_, l);
            }
            else{
                if(*(data + y * dim[0] + x))
                    output->set(x, y_, l);
            }
            
        }
       
}

FIELD<float>* DataManager::getAlphaMapOfLayer(int l, int superR, int time1, int invert) {
    /* FIELD<float>* skel = new FIELD<float>(dim[0], dim[1]);
    for (unsigned int x = 0; x < dim[0]; ++x) 
        for (unsigned int y = 0; y < dim[1]; ++y)
            skel->set(x, y, 0);
   */

    layer_t *da = readLayer(l);
    int x, y, radius;

    if (fbo == 0) {
        PRINT(MSG_ERROR, "Framebuffer object was not initialized. Creating new FBO (FIX THIS!)\n");
        initFBO();
    }

    /* Draw on a texture. */
    fbo->bind();
    if (SHADER == GLOBAL_ALPHA) {
        glClearColor(1.0, 1.0, 1.0, 1.0);
    }

    glEnable(GL_DEPTH_TEST);
    glClear(GL_DEPTH_BUFFER_BIT);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    sh_alpha->bind(); //draw circle rather than quads because of this shader.
    glBegin(GL_QUADS);
    for (unsigned int j = 0; j < da->size(); ++j) {
        x = (*da)[j].first;
        y = (*da)[j].second;
        radius = (*da)[j].third;

        //skel->set(x, y, 1);

        // Texture coordinates are in range [-1, 1]
        // Texcoord selects the texture coordinate
        // vertex call sets it to that location
       
         glTexCoord2f(-1.0, -1.0);
        glVertex2f(x - radius, y - radius);
        glTexCoord2f(-1.0, 1.0);
        glVertex2f(x - radius, y + radius+1);
        glTexCoord2f(1.0, 1.0);
        glVertex2f(x + radius+1, y + radius+1);
        glTexCoord2f(1.0, -1.0);
        glVertex2f(x + radius+1, y - radius);
        
    }
    glEnd();
    
    sh_alpha->unbind();
    fbo->unbind();

    //if (superR>1)
    //{
        FIELD<float>* layer = new FIELD<float>(dim[0], dim[1]);
        float *data = (float *) malloc(dim[0] * dim[1] * sizeof (float));
        
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, fbo->texture->tex);

        // Altering range [0..1] -> [0 .. 255] 
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, data);
    
        //FIELD<float> *im = new FIELD<float>(0, 0);
        //im->setAll(dim[0], dim[1], data);
        //im->writePGM("buffer.pgm");
        
        unsigned int y_;
        for (unsigned int x = 0; x < dim[0]; ++x) 
            for (unsigned int y = 0; y < dim[1]; ++y)
            {

                y_ = dim[1] - 1 -y;

                if(invert){
                    layer->set(x, y_, 1); //ensure that the init value is 1 everywhere.

                    if(*(data + y * dim[0] + x))
                        layer->set(x, y_, 0);
                }
                else{
                    layer->set(x, y_, 0); //ensure that the init value is 0 everywhere.

                    if(*(data + y * dim[0] + x))
                        layer->set(x, y_, 1);
                }
                
            }
       
    //}  

    free(data);
    return layer;            
}

/* Return all disks for a certain intensity */
layer_t *DataManager::readLayer(int l) {
    return (*skelpixel)[l];
}

/* When the lowest intensity is >0, we should set the clear color to i-1, where
* i is the first intensity with disks.
*/
void DataManager::setClearColor() {
    glClearColor(clear_color / 255.0, clear_color / 255.0, clear_color / 255.0, 0);

    PRINT(MSG_VERBOSE, "Clear color set to: %f, %f, %f (layer %d)\n", clear_color / 255.0, clear_color / 255.0, clear_color / 255.0, clear_color);
}

void DataManager::initFBO() {
    fbo = new Framebuffer(dim[0], dim[1], GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, true);
}

void DataManager::setAlphaShader(SHADER_TYPE st) {
    SHADER = st;

    if (st == LOCAL_ALPHA) {
        sh_alpha = new GLSLProgram("glsl/alpha.vert", "glsl/alpha.frag");
        sh_alpha->compileAttachLink();
        PRINT(MSG_VERBOSE, "Succesfully compiled shader 1 (local alpha computation)\n");
    } else if (st == GLOBAL_ALPHA) {
        sh_alpha = new GLSLProgram("glsl/nointerpolationinv.vert", "glsl/nointerpolationinv.frag");
        sh_alpha->compileAttachLink();
        PRINT(MSG_VERBOSE, "Succesfully compiled shader 1 (global alpha computation)\n");
    } else if (st == NORMAL) {
        sh_alpha = new GLSLProgram("glsl/nointerpolation.vert", "glsl/nointerpolation.frag");
        sh_alpha->compileAttachLink();
        PRINT(MSG_VERBOSE, "Succesfully compiled shader 1 (No interpolation)\n");
    } else {
        PRINT(MSG_ERROR, "Invalid shader. Did not compile.");
        exit(1);
    }
}