/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

# include"Poi.h"
# include"common.h"

Poi:: Poi(unsigned int index){
   type=getPointOfInterestType(index);
     name=getPointOfInterestName(index);
     
    position = getPointOfInterestPosition(index);
poiByString[name].push_back(index);
poiByString[type].push_back(index);
}

 void Poi:: setXY(pair<double,double> t){
     xy=t;
 }

LatLon Poi:: getLatLon(){
      return position;
  } 
 pair<double,double> Poi:: getXY(){
     return xy;

 }
 
 string Poi:: getType(){
     return type;
 }
 
 
    string  Poi::getName(){
    
        return name;
    }
