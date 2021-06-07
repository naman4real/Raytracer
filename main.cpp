#include<iostream>
#include<fstream>
#include<sstream>
#include<string>
#include<vector>
#include<cmath>
#include<limits>
#include<stdlib.h>
#include<stdio.h>
#include<time.h>
#include<algorithm>
#include<unordered_map>
#include<math.h>

//fundamental classes
#include "Vect.h"
#include "Color.h"
#include "Object.h"

// derived classes (not in the context of inheritence)
#include "Ray.h"
#include "Camera.h"
#include "Light.h"
#include "Sphere.h"
#include "Plane.h"
#include "Source.h"

using namespace std;

struct RGBType{
    double r;
    double g;
    double b;
};

void savebmp(const char *filename, int w, int h, int dpi, RGBType *data){
    FILE *f;
    int k =w*h;
    int s=4*k;
    int filesize = 54+s;
    double factor = 39.375;
    int m=static_cast<int>(factor);

    int ppm=dpi*m;
    unsigned char bmpfileheader[14]={'B','M',0,0,0,0,0,0,0,0,54,0,0,0};
    unsigned char bmpinfoheader[40]={40,0,0,0,0,0,0,0,0,0,0,0,1,0,24,0};

    bmpfileheader[2]=(unsigned char)(filesize);
    bmpfileheader[3]=(unsigned char)(filesize>>8);
    bmpfileheader[4]=(unsigned char)(filesize>>16);
    bmpfileheader[5]=(unsigned char)(filesize>>24);

    bmpinfoheader[4]=(unsigned char)(w);
    bmpinfoheader[5]=(unsigned char)(w>>8);
    bmpinfoheader[6]=(unsigned char)(w>>16);
    bmpinfoheader[7]=(unsigned char)(w>>24);

    bmpinfoheader[8]=(unsigned char)(h);
    bmpinfoheader[9]=(unsigned char)(h>>8);
    bmpinfoheader[10]=(unsigned char)(h>>16);
    bmpinfoheader[11]=(unsigned char)(h>>24);

    bmpinfoheader[21]=(unsigned char)(s);
    bmpinfoheader[22]=(unsigned char)(s>>8);
    bmpinfoheader[23]=(unsigned char)(s>>16);
    bmpinfoheader[24]=(unsigned char)(s>>24);

    bmpinfoheader[25]=(unsigned char)(ppm);
    bmpinfoheader[26]=(unsigned char)(ppm>>8);
    bmpinfoheader[27]=(unsigned char)(ppm>>16);
    bmpinfoheader[28]=(unsigned char)(ppm>>24);

    bmpinfoheader[29]=(unsigned char)(ppm);
    bmpinfoheader[30]=(unsigned char)(ppm>>8);
    bmpinfoheader[31]=(unsigned char)(ppm>>16);
    bmpinfoheader[32]=(unsigned char)(ppm>>24);

    f=fopen(filename,"wb");
    fwrite(bmpfileheader,1,14,f);
    fwrite(bmpinfoheader,1,40,f);

    for(int i=0;i<k;i++){
        RGBType rgb = data[i];

        double red = (data[i].r)*255;
        double green = (data[i].g)*255;
        double blue = (data[i].b)*255;
        unsigned char color[3] = {(unsigned char)floor(blue),(unsigned char)floor(green),(unsigned char)floor(red)};

        fwrite(color,1,3,f);

    }

    fclose(f);






}


tuple<Object*,double> findClosestObject(vector<Object*> sceneObjects,Ray currentRay)
{
    //retrun index of closest object to the camera
    vector<double> intersections;
    unordered_map<double,Object*> intersectionValue_Object;
    for(int i=0;i<sceneObjects.size();i++){
        intersections.push_back(sceneObjects[i]->findIntersection(currentRay));
        intersectionValue_Object[sceneObjects[i]->findIntersection(currentRay)]=sceneObjects[i];
    }
    Object* closestObject = NULL;
    if(intersections.size()>0){
        sort(intersections.begin(),intersections.end());
        for(auto intersectionValue:intersections){
            if(intersectionValue>0){
                closestObject=intersectionValue_Object[intersectionValue];     
                return tuple<Object*,double>(closestObject,intersectionValue);
            }
        }
    }
    return tuple<Object*,double>(NULL,-1);
}

Color getColorAt(Vect intersectionPosition,Vect intersectingRayDir,vector<Object*> sceneObjects,Object* closestObject,double closestIntersectionValue,vector<Source*> lightSources,double accuracy,double ambientLight){
    Color intersectionObjectColor = closestObject->getColor();
    Vect intersectionObjectNormal = closestObject->getNormalAt(intersectionPosition);
    
    // add bias in the intersection position
    intersectionPosition = intersectionPosition.vectAdd(intersectionObjectNormal.vectMultiply(0.00001));
    if(intersectionObjectColor.getColorSpecial()==2){
        //checkered floor pattern
        int square = (int)floor(intersectionPosition.getVectX()) + (int)floor(intersectionPosition.getVectZ());
        if(square%2==0){
            //red tile
            intersectionObjectColor.setColorRed(1);
            intersectionObjectColor.setColorGreen(0);
            intersectionObjectColor.setColorBlue(0);
        }else{
            //white tile
            intersectionObjectColor.setColorRed(1);
            intersectionObjectColor.setColorGreen(1);
            intersectionObjectColor.setColorBlue(1);
        }

    }
    Color finalColor = closestObject->getColor().colorScalar(ambientLight);

    if(intersectionObjectColor.getColorSpecial()>0 && intersectionObjectColor.getColorSpecial()<=1){
        Vect reflectionDir = intersectionObjectNormal.vectMultiply(-2*intersectionObjectNormal.dotProduct(intersectingRayDir)).vectAdd(intersectingRayDir).normalize(); 
        Ray reflectionRay = Ray(intersectionPosition,reflectionDir);
        tuple<Object*,double> secondaryClosestIntersection = findClosestObject(sceneObjects,reflectionRay);
        Object* secondaryObject = get<0>(secondaryClosestIntersection);
        double secondaryIntersectionValue = get<1>(secondaryClosestIntersection);
        if(secondaryObject){
            if(secondaryIntersectionValue>accuracy){
                // add bias in the intersection position
                Vect reflectionIntersectionPosition = intersectionPosition.vectAdd(reflectionDir.vectMultiply(secondaryIntersectionValue)).vectAdd(intersectionObjectNormal.vectMultiply(0.0000001));
                Color reflectionIntersectionColor = getColorAt(reflectionIntersectionPosition,reflectionDir,sceneObjects,secondaryObject,secondaryIntersectionValue,lightSources,accuracy,ambientLight);
                finalColor=finalColor.colorAdd(reflectionIntersectionColor.colorScalar(intersectionObjectColor.getColorSpecial()));
            }
        }
    }


    for(int lightIndex=0;lightIndex<lightSources.size();lightIndex++){

        double shadowRayDirX = lightSources[lightIndex]->getLightPosition().getVectX()-intersectionPosition.getVectX();
        double shadowRayDirY = lightSources[lightIndex]->getLightPosition().getVectY()-intersectionPosition.getVectY();
        double shadowRayDirZ = lightSources[lightIndex]->getLightPosition().getVectZ()-intersectionPosition.getVectZ();
        Vect shadowRayDir = Vect(shadowRayDirX,shadowRayDirY,shadowRayDirZ).normalize();
        //------------OR-------------------
        //Vect lighDir = lightSources[lightIndex]->getLightPosition().vectAdd(intersectionPosition.negative()).normalize();
        double angle = intersectionObjectNormal.dotProduct(shadowRayDir);
        if(angle>0){
            bool shadowed=false;      
            double distanceToLight = lightSources[lightIndex]->getLightPosition().vectAdd(intersectionPosition.negative()).normalize().magnitude();
            Ray shadowRay(intersectionPosition,shadowRayDir);
            tuple<Object*,double> closestIntersection = findClosestObject(sceneObjects,shadowRay);
            if(get<0>(closestIntersection)){
                if(accuracy < get<1>(closestIntersection) && get<1>(closestIntersection)<= distanceToLight){
                    shadowed=true;
                }
            }
            if(!shadowed){
                finalColor = finalColor.colorAdd(intersectionObjectColor.colorMultiply(lightSources[lightIndex]->getLightColor()).colorScalar(angle));
                // calculate specular 
                if(0<intersectionObjectColor.getColorSpecial() && intersectionObjectColor.getColorSpecial()<=1){

                    Vect reflectedDir = intersectionObjectNormal.vectMultiply(2*intersectionObjectNormal.dotProduct(shadowRayDir)).vectAdd(shadowRayDir.negative()).normalize();
                    double RdotE = reflectedDir.dotProduct(intersectingRayDir.negative());
                    if(RdotE<0){RdotE=0;}
                    if(RdotE>1){RdotE=1;}
                    Color specularColor = lightSources[lightIndex]->getLightColor().colorScalar(1.2*pow(RdotE,12)*intersectionObjectColor.getColorSpecial());
                    finalColor = finalColor.colorAdd(specularColor);
                }          
            }
            else{
                finalColor = finalColor.colorMultiply(intersectionObjectColor);
            }
        }  
    }
    return finalColor.clip();
}

int thisone;

int main(){
    cout<<"rendering.."<<endl;
    clock_t start,end;
    start=clock();
    // desired height, width and dpi of the image 
    int dpi=72;
    int width=640;
    int height = 480;
    double aspectRatio = (double)width/height;
    double ambientLight = 0.2;
    double accuracy = 0.000001;

    int n=width*height;
    RGBType *pixels = new RGBType[n];

    //defining X,Y,Z axes and origin
    Vect O(0,0,0);
    Vect P(-1.25,-0.5,-1.75);
    Vect X(1,0,0);
    Vect Y(0,1,0);
    Vect Z(0,0,1);

    // defining camera parameters and orientation
    Vect camPos(3,1.5,-4);
    Vect look_at(0,0,0);
    Vect diff_btw(camPos.getVectX()-look_at.getVectX(),camPos.getVectY()-look_at.getVectY(),camPos.getVectZ()-look_at.getVectZ());
    Vect camDir = diff_btw.negative().normalize();
    Vect camRight = Y.crossProduct(camDir).normalize();
    Vect camDown = camRight.crossProduct(camDir);

    //defining scene with the camera
    Camera scene_cam(camPos,camDir,camRight,camDown);

    //defining colors for light
    Color white_light (1,1,1,0);
    Color pretty_green (0.5,1.0,0.5,0.3);
    Color tile_floor (1,1,1,2);
    Color gray(0.5,0.5,0.5,0);
    Color black(0.0,0.0,0.0,0);
    Color pretty_blue(0.5,0.5,1,0.3);

    //create a light source
    Vect light_position(-7,10,-10);
    Light scene_light(light_position,white_light);
    vector<Source*> lightSources;
    // vector of light sources
    lightSources.push_back(dynamic_cast<Source*>(&scene_light));

    //instantiate scene objects
    Sphere sphere1 (O,1,pretty_green);
    Sphere sphere2 (P,0.5,pretty_blue);
    Plane scene_plane (Y,-1, tile_floor);
    vector<Object*> sceneObjects;
    sceneObjects.push_back(dynamic_cast<Object*>(&sphere1));
    sceneObjects.push_back(dynamic_cast<Object*>(&sphere2));
    sceneObjects.push_back(dynamic_cast<Object*>(&scene_plane));
    
    double xAmt, yAmt;

    for(int x=0;x<width;x++){
        for(int y=0;y<height;y++){
            thisone=y*width+x;
            if(width>height){
                xAmt =((x+0.5)/width)*aspectRatio - ((width-height)/(double)height)/2;
                yAmt = ((height-y) + 0.5)/height;
            }
            else if(height>width){
                xAmt = (x+0.5)/width;
                yAmt = (((height-y)+0.5)/height)/aspectRatio - ((height-width)/(double)width)/2;
            }
            else{
                xAmt = (x+0.5)/width;
                yAmt = ((height-y)+0.5)/height;
            }
            Vect cam_ray_origin = scene_cam.getCameraPosition();
            Vect cam_ray_direction = camDir.vectAdd(camRight.vectMultiply(xAmt-0.5).vectAdd(camDown.vectMultiply(yAmt-0.5))).normalize();
            Ray cam_ray(cam_ray_origin,cam_ray_direction);

            //find ray intersection with the closest object
            tuple<Object*,double> closestIntersection = findClosestObject(sceneObjects,cam_ray);

            //Vect shadowRayOrigin = 
            Object* closestObject = get<0>(closestIntersection);
            double closestIntersectionValue = get<1>(closestIntersection);
            if(closestObject){
                if(closestIntersectionValue>accuracy){

                    double shadowRayOriginX = cam_ray_origin.getVectX()+closestIntersectionValue*cam_ray_direction.getVectX();
                    double shadowRayOriginY = cam_ray_origin.getVectY()+closestIntersectionValue*cam_ray_direction.getVectY();
                    double shadowRayOriginZ = cam_ray_origin.getVectZ()+closestIntersectionValue*cam_ray_direction.getVectZ();
                    Vect intersectionPosition = Vect(shadowRayOriginX,shadowRayOriginY,shadowRayOriginZ);
                    //------------OR-------------------
                    //Vect intersectionPosition = cam_ray_origin.vectAdd(cam_ray_direction.vectMultiply(closestIntersectionValue));
                    
                    Vect intersectingRayDir = cam_ray_direction;

                    Color intersectionColor = getColorAt(intersectionPosition,intersectingRayDir,sceneObjects,closestObject,closestIntersectionValue,lightSources,accuracy,ambientLight);
                    pixels[thisone].r=intersectionColor.getColorRed();
                    pixels[thisone].g=intersectionColor.getColorGreen();
                    pixels[thisone].b=intersectionColor.getColorBlue();
                }
                // if(closestIntersectionValue>accuracy){
                //     Color finalColor = closestObject->getColor().colorScalar(ambientLight);
                //     double shadowRayOriginX = cam_ray_origin.getVectX()+closestIntersectionValue*cam_ray_direction.getVectX();
                //     double shadowRayOriginY = cam_ray_origin.getVectY()+closestIntersectionValue*cam_ray_direction.getVectY();
                //     double shadowRayOriginZ = cam_ray_origin.getVectZ()+closestIntersectionValue*cam_ray_direction.getVectZ();
                //     Vect shadowRayOrigin = Vect(shadowRayOriginX,shadowRayOriginY,shadowRayOriginZ);
                //     double shadowRayDirX = light_position.getVectX()-shadowRayOriginX;
                //     double shadowRayDirY = light_position.getVectY()-shadowRayOriginY;
                //     double shadowRayDirZ = light_position.getVectZ()-shadowRayOriginZ;
                //     Vect shadowRayDir = Vect(shadowRayDirX,shadowRayDirY,shadowRayDirZ).normalize();
                //     Vect intersectionObjectNormal = closestObject->getNormalAt(shadowRayOrigin);
                //     float angle = intersectionObjectNormal.dotProduct(shadowRayDir);
                //     bool shadowed = false;
                //     if(angle>0){
                //         Ray shadowRay(shadowRayOrigin,shadowRayDir);
                //         tuple<Object*,double> closestShadowIntersection = findClosestObject(sceneObjects,shadowRay);
                //         Object* closestShadowObject = get<0>(closestShadowIntersection);
                //         double closestShadowIntersectionValue = get<1>(closestShadowIntersection);            
                //         if(closestShadowObject && closestShadowIntersectionValue>accuracy ){ 
                //             shadowed=true;
                //         }
                //     }     
                //     if(!shadowed){
                //         // finalColor = finalColor.colorAdd(intersectionObjectColor.colorMultiply(lightSources[lightIndex]->getLightColor()).colorScalar(angle));
                //         finalColor=finalColor.colorAdd(closestObject->getColor().colorMultiply(scene_light.getLightColor()).colorScalar(angle));
                //         pixels[thisone].r=finalColor.getColorRed();
                //         pixels[thisone].g=finalColor.getColorGreen();
                //         pixels[thisone].b=finalColor.getColorBlue();
                //     }
                // }
                
    
            }
            else{
                pixels[thisone].r=0;
                pixels[thisone].g=0;
                pixels[thisone].b=0;       
            }        
        }
    }

    savebmp("scene.bmp",width,height,dpi,pixels);
    delete pixels;
    end=clock();
    float timeTaken = ((float)end - (float)start)/1000;

    cout<<"image rendered in "<<timeTaken<<" seconds!";

    return 0;
}