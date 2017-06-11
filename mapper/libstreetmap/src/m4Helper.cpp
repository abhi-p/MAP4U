/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "common.h"
#include "m4.h"
#include "m3.h"
#include "m3Helper.h"
#include "m4Helper.h"
void addPaths(vector<unsigned int> & pathSoFar,vector<unsigned int> & pathToAdd){
    
    int size=pathToAdd.size();
 
    for(int i=0;i<size;i++){
       pathSoFar.push_back(pathToAdd[i]);
 }
}
void addPaths(vector<unsigned int> & pathSoFar,unsigned int intersect_id_start,unsigned int intersect_id_end){

 vector<unsigned int> temp;
   unsigned int currentId=intersect_id_end;
   
    while(currentId!=intersect_id_start){
   
        temp.push_back(travelTimes[currentId].second);
        currentId=streetSegmentDataBase[travelTimes[currentId].second]->getOtherIntersectionID(currentId);
       
    }
    
    int size=temp.size();
    if(size>0){
    for(int i=0;i<size;i++){
       pathSoFar.push_back(temp[size-i-1]);
 }
    }
  
}

void resetGraph(const vector<DeliveryInfo>& deliveries){

    int numOfInterscetions= getNumberOfIntersections();
       travelTimes.resize(numOfInterscetions);
        deliver.resize(numOfInterscetions);
            pair<float,int> p (-2.0,-1);
    for(int i=0;i< numOfInterscetions;i++){
    graph[i].visited=false;
     graph[i].relativeTime=0;
    travelTimes[i]=p;
    deliver[i].dropsLeft=0;
     deliver[i].pickUpScore=0;
     deliver[i].dropOffAbleToDo.clear();
     deliver[i].totalDrops=0;
    }

            int size= deliveries.size();
            
            for(int i=0;i<size;i++){
                deliver[deliveries[i].dropOff].dropsLeft++;
            deliver[deliveries[i].pickUp].pickUpScore++;
             deliver[deliveries[i].pickUp].dropOffAbleToDo.push_back(deliveries[i].dropOff);
             deliver[deliveries[i].dropOff].totalDrops++;
            }
}



vector<unsigned int> createPath(vector<unsigned int> deliveryOrder,  const std::vector<unsigned>& depots,float turn_penalty){

    auto middle=createDeliveryRoute(deliveryOrder);

   auto source= find_path_to_intrested_From(deliveryOrder[0], depots,turn_penalty);
 addPaths(source,middle);
    


}

void resetGraph(vector<unsigned>& depots) {
    int numOfIntersections = getNumberOfIntersections();
    travelTimes.clear();
    travelTimes.resize(getNumberOfIntersections());

    pair<float, int> p(-1.0, -2);
    for (int i = 0; i < numOfIntersections; i++) {

        travelTimes[i] = p;
        graph[i].visited = false;
        graph[i].relativeTime = 0;
           graph[i].wantedPOINear=false;

    }

     int numOfDepots= depots.size();
    for(int i=0;i< numOfDepots;i++){
    graph[depots[i]].wantedPOINear=true;
    }
}
vector<unsigned int > createDeliveryRoute(vector<unsigned int> deliveryOrder){
    vector<unsigned int > route;
    int size= deliveryOrder.size();
    
    
    if(size>0){
        int start=deliveryOrder[0];
    for(int i=1;i<size;i++){
        
    addPaths(route,start,deliveryOrder[i]);
    start= deliveryOrder[i];
    }
    }
    return route;
}

std::vector<unsigned> find_path_to_intrested_From(const unsigned intersect_id_start, vector<unsigned int> interest,const double turn_penalty){
    vector<unsigned int> path;
    resetGraph(interest);
    addPathSegment(0,1/graph[intersect_id_start].relativeTime,intersect_id_start, 0);
    float shortestTimeInverse=-1; 
    int shortestIntersection=-1;
      
     while(!pathQueue.empty()){
    
    pathSeg topPath= pathQueue.top();
   
   pathQueue.pop();
   
    if(shortestTimeInverse<topPath.travelTime){
        
        updateTravelTime(topPath);
        
        if(graph[topPath.currInter].wantedPOINear){
       shortestIntersection=topPath.currInter;  
       shortestTimeInverse=1/travelTimes[shortestIntersection].first;  
       }

   if(!graph[topPath.currInter].visited){
        visitSources(topPath,turn_penalty, graph[topPath.currInter].wantedPOINear); 
   }
   }
 
}
 if(shortestIntersection>=0){
    path=constructPath(intersect_id_start, shortestIntersection);
}

        endInter=shortestIntersection;

        return path;   
}

std::vector<unsigned> find_path_to_intrested_To(const unsigned intersect_id_start, vector<unsigned int> interest,const double turn_penalty){
    vector<unsigned int> path;
    resetGraph(interest);
    addPathSegment(0,1/graph[intersect_id_start].relativeTime,intersect_id_start, 0);
    float shortestTimeInverse=-1; 
    int shortestIntersection=-1;
      
     while(!pathQueue.empty()){
    
    pathSeg topPath= pathQueue.top();
   
   pathQueue.pop();
   
    if(shortestTimeInverse<topPath.travelTime){
        
        updateTravelTime(topPath);
        
        if(graph[topPath.currInter].wantedPOINear){
       shortestIntersection=topPath.currInter;  
       shortestTimeInverse=1/travelTimes[shortestIntersection].first;  
       }

   if(!graph[topPath.currInter].visited){
        visitNode(topPath,intersect_id_start,turn_penalty); 
   }
   }
 
}
 if(shortestIntersection>=0){
    path=constructPath(intersect_id_start, shortestIntersection);
}

        endInter=shortestIntersection;

        return path;   
}