/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */


# include "common.h"
# include "m3.h"
# include "m1.h"
 priority_queue<pathSeg> pathQueue;
 vector<pair<float, int>> travelTimes;
void updateTravelTime(pathSeg & toAdd){
    //cout<<"Comparing Travel time "<< travelTimes[toAdd.currInter].first<<" and "<<1/toAdd.travelTime<<endl;
    if (1 / travelTimes[toAdd.currInter].first < toAdd.travelTime) {
      //   cout<<"Updating Travel time from "<< travelTimes[toAdd.currInter].first<<" to "<<1/toAdd.travelTime<<endl;
        travelTimes[toAdd.currInter].first = 1 / toAdd.travelTime;
        travelTimes[toAdd.currInter].second = toAdd.segId;
        graph[toAdd.currInter].visited=false;
    }

}

void addPathSegment(float heuristic,float travelTime,int currInter, int segId){
pathSeg first;
    first.currInter = currInter;
    first.heuristic = heuristic;
    first.segId = segId;
    first.travelTime = travelTime;
    pathQueue.push(first);
}

void visitNode(pathSeg current, float turn_penalty, int startId) {

    int interId = current.currInter;
    auto adjacent = graph[interId].adjacent;
    int numOfSegs = graph[interId].adjacent.size();


    for (int i = 0; i < numOfSegs; i++) {

        pathSeg toAdd;
        toAdd.segId = adjacent[i].second;
        toAdd.currInter = streetSegmentDataBase[toAdd.segId]->getOtherIntersectionID(interId);
        // cout<<"Adding "<<toAdd.currInter<<endl;
        toAdd.travelTime = 1 / current.travelTime;
        toAdd.travelTime += adjacent[i].first;
        if (startId != interId && streetSegmentDataBase[toAdd.segId]->getStreetId() != streetSegmentDataBase[current.segId]->getStreetId()) {
            toAdd.travelTime += turn_penalty;
        }
        toAdd.heuristic = toAdd.travelTime + graph[toAdd.currInter].relativeTime;
        toAdd.travelTime = 1 / toAdd.travelTime;
        pathQueue.push(toAdd);
    }
    graph[interId].visited = true;

}

vector<unsigned> constructPath(unsigned int intersect_id_start,unsigned int intersect_id_end){

 vector<unsigned int> temp,path;
   unsigned int currentId=intersect_id_end;
   
    while(currentId!=intersect_id_start){
   
        temp.push_back(travelTimes[currentId].second);
        currentId=streetSegmentDataBase[travelTimes[currentId].second]->getOtherIntersectionID(currentId);
       
    }
    
    int size=temp.size();
    if(size>0){
    path.resize(size);
    for(int i=0;i<size;i++){
        path[i]=temp[size-i-1];
 }
    }
    return path;
}

void visitSources(pathSeg current, float turn_penalty, bool isSource){
    int interId = current.currInter;
    auto adjacent = graph[interId].source;
    int numOfSegs = graph[interId].source.size();


    for (int i = 0; i < numOfSegs; i++) {

        pathSeg toAdd;
        toAdd.segId = adjacent[i].second;
        toAdd.currInter = streetSegmentDataBase[toAdd.segId]->getOtherIntersectionID(interId);
        // cout<<"Adding "<<toAdd.currInter<<endl;
        toAdd.travelTime = 1 / current.travelTime;
        toAdd.travelTime += adjacent[i].first;
        if (!isSource && streetSegmentDataBase[toAdd.segId]->getStreetId() != streetSegmentDataBase[current.segId]->getStreetId()) {
            toAdd.travelTime += turn_penalty;
        }
        toAdd.heuristic = toAdd.travelTime + graph[toAdd.currInter].relativeTime;
        toAdd.travelTime = 1 / toAdd.travelTime;
        pathQueue.push(toAdd);
    }
    graph[interId].visited = true;

}

void resetGraph(string name){
    int numOfIntersections= getNumberOfIntersections();
  //  travelTimes.clear();
    travelTimes.resize(getNumberOfIntersections());
  
     auto type= poiByString[name];
    
    pair<float,int> p (-2.0,-1);
    
    int inter=find_closest_intersection(poiDataBase[type[0]]->getLatLon());
    
    for(int i=0;i< numOfIntersections;i++){
        travelTimes[i]=p;
        graph[i].visited=false;
     graph[i].wantedPOINear=false;
     graph[i].relativeTime=calcDistBetTwo(i, inter)*0.036;
    }
    
     int numOfPois= type.size();
    for(int i=0;i< numOfPois;i++){
        auto closestInter= find_closest_intersection(poiDataBase[type[i]]->getLatLon()); 
    graph[closestInter].wantedPOINear=true;
    }
    
}

void resetGraph(int destination) {
    int numOfIntersections = getNumberOfIntersections();
    travelTimes.clear();
    travelTimes.resize(getNumberOfIntersections());

    pair<float, int> p(-1.0, -2);
    for (int i = 0; i < numOfIntersections; i++) {

        travelTimes[i] = p;
        graph[i].visited = false;
        graph[i].relativeTime = calcDistBetTwo(i, destination)*0.036;

    }
}