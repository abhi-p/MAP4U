/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
//108a5560-b702-4a46-af30-aa5dd29954a6
#include <queue>

#include "m4.h"
#include "m3Helper.h"
#include "m3.h"
#include "common.h"
#include "m4Helper.h"
#include "m3Helper.h"
vector<deliverInfo> deliver;
//vector<pair<float, int>> travelTimes;
int spotsToVisit=0;
//void resetGraph(vector<DeliveryInfo>& deliveries);



vector <unsigned> tcVersion1(const std::vector<DeliveryInfo>& deliveries,   const std::vector<unsigned>& depots,   const float turn_penalty);
std::vector<unsigned> twoOptSwap(std::vector<unsigned>& path, int i, int k);
std::vector<unsigned> bestRoute(const std::vector<DeliveryInfo>& deliveries,std::vector<unsigned>& path, const float turn_penalty );

float findTimeBetweenIntersections(const unsigned intersect_id_start, const unsigned intersect_id_end, const double turn_penalty);

bool courier_path_is_legal(const std::vector<DeliveryInfo>& deliveries, const std::vector<unsigned>& path);





std::vector<unsigned> traveling_courier(const std::vector<DeliveryInfo>& deliveries,   const std::vector<unsigned>& depots,   const float turn_penalty){
    vector<unsigned> visitingOrder=tcVersion1(deliveries,  depots,   turn_penalty);
    
//    
//    
//    if (courier_path_is_legal(deliveries,path))
//        cout<<"it works"<<endl;
//    else if (!(courier_path_is_legal(deliveries,path)))
//        cout<<"it not works"<<endl;
//
//            cout<<deliveries[0].pickUp<<endl;
//
    return visitingOrder;
}

int clear(int intersection){

    int remove=0;
    
    remove+=deliver[intersection].pickUpScore;
    auto drops= deliver[intersection].dropOffAbleToDo;
    remove+=deliver[intersection].totalDrops;
    for(int i=0;i<drops.size();i++){
        deliver[drops[i]].dropsLeft-=1;
    }
    deliver[intersection].dropsLeft=0;
    deliver[intersection].pickUpScore=0;
    deliver[intersection].dropOffAbleToDo.clear();
    deliver[intersection].totalDrops=0;
    int numOfInterscetions= getNumberOfIntersections();
    
         pair<float,int> p (-2.0,-1);
         
    for(int i=0;i< numOfInterscetions;i++){
    graph[i].visited=false;
    travelTimes[i]=p;
    }
         
    return remove;
}




std::vector<unsigned> tcVersion1(const std::vector<DeliveryInfo>& deliveries,   const std::vector<unsigned>& depots,   const float turn_penalty){
    int size= depots.size();
    spotsToVisit=2*deliveries.size();
    
    for(int j=0;j<size;j++){
        
resetGraph(deliveries);

vector<unsigned int> completePath;

bool done=false;

int visited=0;


int startInter= depots[j];
// for(int i=0;i<deliveries.size();i++){
//  
//      cout<<" Pick up at "<<deliveries[i].pickUp<<" delivery at "<<deliveries[i].dropOff<<endl;
//  }
  
while(visited!=spotsToVisit&&!done){
vector<unsigned int> partialPath;
pathSeg first;
 first.currInter = startInter;
 first.heuristic = 0;
 first.segId = -1;
 first.travelTime = 1;
 pathQueue.push(first);
  float shortestTimeInverse=-1; 
  int shortestIntersection=-1;

 

 while(!pathQueue.empty()){
 
     auto top= pathQueue.top();
     pathQueue.pop();
     
        
    if(shortestTimeInverse<top.travelTime&&deliver[top.currInter].dropsLeft<=0){
        
        updateTravelTime(top);
        
        if(deliver[top.currInter].totalDrops+deliver[top.currInter].pickUpScore>0){
       shortestIntersection=top.currInter;  
       shortestTimeInverse=1/travelTimes[shortestIntersection].first;  
       }

   if(!graph[top.currInter].visited){
        visitNode(top, turn_penalty,startInter); 
   }
   }

 }

 
  if(shortestIntersection>=0){
     // cout<< shortestIntersection<<endl;
 addPaths(completePath, startInter,shortestIntersection);  
 
 visited+=clear(shortestIntersection);
 
 startInter=shortestIntersection;
  }

  
  else{
  done=true;
  }
  
} 

if(!done){

auto partialPath =find_path_between_intersections(startInter, depots[j], turn_penalty);
   vector<unsigned int> temp;
if(partialPath.size()==0){
    return temp;
}

 addPaths(completePath,partialPath);

    return completePath;
}

}
   vector<unsigned int> temp;

return temp;
 
    
}


std::vector<unsigned> twoOptSwap(std::vector<unsigned>& path, int i, int k) {
       
    std::vector<unsigned> newPath;
    newPath.resize(path.size());
    
    for(int j=0; j<=i-1; j++){
        newPath[j]=path[j];
    }
    
    int n=k;
    for (int j=i; j<=k; j++){
        
        if(j<=n){
            newPath[j]=path[n];
            n--;
        }
    }
  
  
    for(int j=k+1; j<path.size(); j++){
        newPath[j]=path[j];
    }
    
    return newPath;
//    
//        1. take route[1] to route[i-1] and add them in order to new_route
//       2. take route[i] to route[k] and add them in reverse order to new_route
//       3. take route[k+1] to end and add them in order to new_route
//       return new_route;
   }




std::vector<unsigned> bestRoute(const std::vector<DeliveryInfo>& deliveries,std::vector<unsigned>& path, const float turn_penalty ){
    
    int time=0;
    float bestTime, newTime =0;
    std::vector<unsigned> newPath;
    
   
        
        for(int i=0; i<path.size()-1; i++){  
            bestTime+= findTimeBetweenIntersections(path[i], path[i+1], turn_penalty);        
        }
        
        for(int i=0; i<path.size()-1; i++){
            for(int k=i+1; k<path.size(); k++){
                
                newTime=0;
                newPath= twoOptSwap(path, i, k);
                
                for(int j=0; j<path.size()-1; j++){  
                    newTime+= findTimeBetweenIntersections(newPath[j], newPath[j+1], turn_penalty);        
                }
                
                if(newTime<bestTime&&courier_path_is_legal(deliveries,newPath)){
                    path=newPath;
                    bestTime=newTime;
                }
                
                
            }
             
            
        }
    return path;
    }



void pick_up_at_intersection(const std::vector<DeliveryInfo>& deliveries,
                             const std::multimap<unsigned,size_t>& intersections_to_pick_up,
                             const unsigned curr_intersection,
                             std::vector<bool>& deliveries_picked_up);

void drop_off_at_intersection(const std::vector<DeliveryInfo>& deliveries,
                              const std::multimap<unsigned,size_t>& intersections_to_drop_off,
                              const std::vector<bool>& deliveries_picked_up,
                              const unsigned curr_intersection,
                              std::vector<bool>& deliveries_dropped_off);

bool delivered_all_packages(const std::vector<DeliveryInfo>& deliveries,
                            const std::vector<bool>& deliveries_picked_up,
                            const std::vector<bool>& deliveries_dropped_off);


bool courier_path_is_legal(const std::vector<DeliveryInfo>& deliveries,
                           const std::vector<unsigned>& path) {

         unsigned curr_intersection;

  
        //We store whether each delivery has been picked-up or dropped-off
        std::vector<bool> deliveries_picked_up(deliveries.size(), false);
        std::vector<bool> deliveries_dropped_off(deliveries.size(), false);

        //We also build fast-lookups from: intersection id to DeliveryInfo index for both
        //pickUp and dropOff intersections
        std::multimap<unsigned,size_t> intersections_to_pick_up; //Intersection_ID -> deliveries index
        std::multimap<unsigned,size_t> intersections_to_drop_off; //Intersection_ID -> deliveries index

        //Load the look-ups
        for(size_t delivery_idx = 0; delivery_idx < deliveries.size(); ++delivery_idx) {
            unsigned pick_up_intersection = deliveries[delivery_idx].pickUp;
            unsigned drop_off_intersection = deliveries[delivery_idx].dropOff;
            //<<deliveries[delivery_idx].pickUp<<endl;

            intersections_to_pick_up.insert(std::make_pair(pick_up_intersection, delivery_idx));
            intersections_to_drop_off.insert(std::make_pair(drop_off_intersection, delivery_idx));
        }

        //We verify the path by walking along each segment and:
        //  * Checking that the next step along the path is valid (see traverse_segment())
        //  * Recording any package pick-ups (see pick_up_at_intersection())
        //  * Recording any package drop-offs (see drop_off_at_intersection())
        for (size_t path_idx = 0; path_idx < path.size(); ++path_idx) {
           
curr_intersection = path[path_idx];
if (curr_intersection==73593)
    cout<<"something works"<<endl;
//cout<<curr_intersection<<endl;
       //     unsigned next_intersection; //Set by traverse_segment
            //if ((path_idx+1)!=path.size())
          //  unsigned next_intersection=path[path_idx+1];
            
           // StreetSegmentInfo seg_info = getStreetSegmentInfo(path[path_idx]);

//      bool seg_traverse_forward;
//       if (seg_info.from == curr_intersection) {
//        //We take care to check 'from' first. This ensures
//        //we get a reasonable traversal direction even in the 
//        //case of a self-looping one-way segment
//
//        //Moving forwards
//        seg_traverse_forward = true;
//
//    } else if (seg_info.to == curr_intersection) {
//        //Moving backwards
//        seg_traverse_forward = false;
//
//    } 
    //
    //Advance to the next intersection
    //
    //next_intersection = (seg_traverse_forward) ? seg_info.to : seg_info.from;

            //Process packages
            pick_up_at_intersection(deliveries, 
                                    intersections_to_pick_up, 
                                    curr_intersection, 
                                    deliveries_picked_up);

            drop_off_at_intersection(deliveries,
                                     intersections_to_drop_off, 
                                     deliveries_picked_up,
                                     curr_intersection, 
                                     deliveries_dropped_off);

            //Advance
           // if ((path_idx+1)!=path.size())
            //curr_intersection = path[path_idx+1];
        }

       

        //
        //Check everything was delivered
        //
        if(!delivered_all_packages(deliveries, deliveries_picked_up, deliveries_dropped_off)) {
           
            return false;
        }

    

    //Everything validated
    return true;

}





void pick_up_at_intersection(const std::vector<DeliveryInfo>& /*deliveries*/,
                             const std::multimap<unsigned,size_t>& intersections_to_pick_up,
                             const unsigned curr_intersection,
                             std::vector<bool>& deliveries_picked_up) {
    //
    //Check if we are picking up packages
    //

    //Find all the deliveries picking-up from this intersection
    auto range_pair = intersections_to_pick_up.equal_range(curr_intersection);
    
    //Mark each delivery as picked-up
    for(auto key_value_iter = range_pair.first; key_value_iter != range_pair.second; ++key_value_iter) {
        size_t delivery_idx = key_value_iter->second; 

        deliveries_picked_up[delivery_idx] = true;
        //cout<<delivery_idx<<endl;



    }
}

void drop_off_at_intersection(const std::vector<DeliveryInfo>& /*deliveries*/,
                              const std::multimap<unsigned,size_t>& intersections_to_drop_off,
                              const std::vector<bool>& deliveries_picked_up,
                              const unsigned curr_intersection,
                              std::vector<bool>& deliveries_dropped_off) {
    //
    //Check if we are dropping-off packages
    //

    //Find all the deliveries dropping-off to this intersection
    auto range_pair = intersections_to_drop_off.equal_range(curr_intersection);

    //Mark each delivery dropped-off
    for(auto key_value_iter = range_pair.first; key_value_iter != range_pair.second; ++key_value_iter) {
        size_t delivery_idx = key_value_iter->second; 

        if(deliveries_picked_up[delivery_idx]) {
            //Can only drop-off if the delivery was already picked-up
            deliveries_dropped_off[delivery_idx] = true;


        }
    }
}



bool delivered_all_packages(const std::vector<DeliveryInfo>& deliveries,
                            const std::vector<bool>& deliveries_picked_up,
                            const std::vector<bool>& deliveries_dropped_off) {

    //
    //Check how many undelivered packages there are
    //
    size_t undelivered_packages = 0;
    for(size_t delivery_idx = 0; delivery_idx < deliveries.size(); ++delivery_idx) {
        if(!deliveries_dropped_off[delivery_idx]) {

            ++undelivered_packages;
        } else {
            cout<<"delivery made"<<endl;
            //If it was dropped-off it must have been picked-up
            assert(deliveries_picked_up[delivery_idx]);
        }
    }

    //
    //Report to the user the missing packages
    //
    if(undelivered_packages > 0) {
        return false;
    }

    //All delivered
    return true;
}









 
