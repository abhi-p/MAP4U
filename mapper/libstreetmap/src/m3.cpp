#include"common.h"
#include"m3.h"
#include"m3Helper.h"
#include"m1.h"
#include "m2.h"
#include <tuple>
#include <queue>
// unit's first element is the heuristic value, second is the actual tavel value, 
//third is the id of the previous intersection, fourth is the id of the actual intersection


















double compute_path_travel_time(const std::vector<unsigned>& path, const double turn_penalty){
    double totalTime = 0;
    int size = path.size();
    if (size > 0) {
        totalTime += streetSegmentDataBase[path[0]]->getSegTravelTime();
        unsigned int currentStreetId = streetSegmentDataBase[path[0]]->getStreetId();

        for (int i = 1; i < size; i++) {

            if (streetSegmentDataBase[path[i]]->getStreetId() != currentStreetId) {
                totalTime += turn_penalty;
                currentStreetId = streetSegmentDataBase[path[i]]->getStreetId();
            }
            totalTime += streetSegmentDataBase[path[i]]->getSegTravelTime();
        }
    }
    return totalTime;
}

std::vector<unsigned> find_path_between_intersections(const unsigned intersect_id_start, const unsigned intersect_id_end, const double turn_penalty) {
    vector<unsigned int> path;
    resetGraph(intersect_id_end);
    float shortestTimeInverse = -1;

if(intersect_id_start!=intersect_id_end){
    
addPathSegment(0,1/graph[intersect_id_start].relativeTime,intersect_id_start, 0);

while(!pathQueue.empty()){
    
    pathSeg topPath= pathQueue.top();
   
   pathQueue.pop();
   
    if(shortestTimeInverse<topPath.travelTime){
       // cout<<"Comparing "<<shortestTimeInverse<<" and "<< topPath.travelTime<<endl;
       updateTravelTime(topPath);
  
      
   if(!graph[topPath.currInter].visited){
   //cout<<"Visiting new nodes"<<endl;
        visitNode(topPath, turn_penalty,intersect_id_start); 
   }
       
  shortestTimeInverse=1/travelTimes[intersect_id_end].first;     
}
}

if(shortestTimeInverse>=0){
    path=constructPath(intersect_id_start, intersect_id_end);
}
}

    return path;
}

std::vector<unsigned> find_path_to_point_of_interest(const unsigned intersect_id_start, const std::string point_of_interest_name,const double turn_penalty){
    vector<unsigned int> path;
    resetGraph(point_of_interest_name);
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
        visitNode(topPath, turn_penalty,intersect_id_start); 
   }
   }
 
}
 if(shortestIntersection>=0){
    path=constructPath(intersect_id_start, shortestIntersection);
}

        endInter=shortestIntersection;

        return path;   
}
float findTimeBetweenIntersections(const unsigned intersect_id_start, const unsigned intersect_id_end, const double turn_penalty) {

    vector<unsigned int> path;
    resetGraph(intersect_id_end);
    float shortestTimeInverse = -1;

    if (intersect_id_start != intersect_id_end) {

        addPathSegment(0, 1 / graph[intersect_id_start].relativeTime, intersect_id_start, 0);

        while (!pathQueue.empty()) {

            pathSeg topPath = pathQueue.top();

            pathQueue.pop();

            if (shortestTimeInverse < topPath.travelTime) {
                // cout<<"Comparing "<<shortestTimeInverse<<" and "<< topPath.travelTime<<endl;
                updateTravelTime(topPath);


                if (!graph[topPath.currInter].visited) {
                    //cout<<"Visiting new nodes"<<endl;
                    visitNode(topPath, turn_penalty, intersect_id_start);
                }

                shortestTimeInverse = 1 / travelTimes[intersect_id_end].first;
            }
        }

        if (shortestTimeInverse >= 0) {
            path = constructPath(intersect_id_start, intersect_id_end);
        }
    }

    return (1/shortestTimeInverse);

}
