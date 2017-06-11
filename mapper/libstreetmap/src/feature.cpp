/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
# include "feature.h"
# include "common.h"
#include "graphics_types.h"
 feature::feature(unsigned int index){
    colour =decideColour(getFeatureType(index));
    name=getFeatureName(index);
    
     numOfPoints=getFeaturePointCount(index);
     LatLon p1;
     featurePoints= new t_point[numOfPoints];
    
   for(int i=0;i<numOfPoints;i++){
       p1=  getFeaturePoint(index, i);
       t_point t (earth_RADIUS_IN_METERS*p1.lon() * cos(avgLatInter * deg_TO_RAD1) * deg_TO_RAD1,earth_RADIUS_IN_METERS*p1.lat()  * deg_TO_RAD1);
        featurePoints[i]= t;
  
   
   }
     
     isPolygon=false;
     area=0;
     if(numOfPoints>0){
         isPolygon=((featurePoints[0].x==featurePoints[numOfPoints-1].x)&&(featurePoints[0].y==featurePoints[numOfPoints-1].y) );
         if(isPolygon){
   for(int i=0;i<numOfPoints;i++){
     
       area+=(featurePoints[(i+1)%numOfPoints].x+featurePoints[i].x)*(featurePoints[(i+1)%numOfPoints].y-featurePoints[i].y)/2;
   }
   
   area = abs(area);
   pair<double, unsigned int> p(area,index);
  featuresByArea.push_back(p);
         }
         else
         {
             featuresWithNoArea.push_back(index);
         }
     }
     
center={0, 0};
    
 }
 
    t_point*  feature:: getPointsPointer(){
    
        return featurePoints;
    }
    
    t_point feature:: getPoint(int index){
    
     return featurePoints[index];
    }
    
    string feature:: getColour(){
        return colour;
    }
    
     string feature:: decideColour(FeatureType type){
        
        if(type==Greenspace||type==Golfcourse||type==Park){
        return "limegreen";
        }
        
           if(type==Unknown||type==Building){
        return "grey75";
        }
        if(type==Island)
        {
            return "darkgreen";
        }
        if(type==Lake||type==River||type==Stream){
           // setcolor(163,209,255);
           return "lightskyblue";
        }
      
     
        if(type==Shoreline){
         return "bisque";
        }
         if(type==Beach){
    return "yellow";
        }
        return"black";
    }
    int feature:: getNumOfPoints(){
    
        return numOfPoints;
    }
    
    t_point feature:: getCenterOfPoly() {

unsigned total_vertices=numOfPoints;
    //t_point centerOfPolygon = {0, 0};

    double signedArea = 0;

    double x0 = 0;

    double x1 = 0;

    double y0 = 0;

    double y1 = 0;

    double partialSignedArea = 0;

    //computer all vertices except for the last 



    unsigned vertex_count = 0;

    for (vertex_count = 0; vertex_count < (total_vertices - 1); vertex_count++) {

        x0 = featurePoints[vertex_count].x;

        y0 = featurePoints[vertex_count].y;

        x1 = featurePoints[vertex_count + 1].x;

        y1 = featurePoints[vertex_count + 1].y;

        partialSignedArea = x0 * y1 - x1*y0;

        signedArea += partialSignedArea;

        center.x += (x0 + x1) * partialSignedArea;

        center.y += (y0 + y1) * partialSignedArea;

    }

    //Do last vertex separately to avoid perfomring an expensive modulous operation in each iteration

    x0 = featurePoints[total_vertices - 1].x;

    y0 = featurePoints[total_vertices - 1].y;

    x1 = featurePoints[0].x;

    y1 = featurePoints[0].y;

    partialSignedArea = (x0 * y1) - (x1 * y0);

    signedArea += partialSignedArea;



    center.x += (x0 + x1) * partialSignedArea;

    center.y += (y0 + y1) * partialSignedArea;



    signedArea *= 0.5;

    center.x /= (6.0 * signedArea);

    center.y /= (6.0 * signedArea);
    return center;

}
    t_point feature:: getCenter(){
        return (center);
    }
 
  feature::~feature(){
   
   delete []featurePoints;
  
  }
  
  