/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   m3Helper.h
 * Author: sampanth
 *
 * Created on April 7, 2017, 11:48 AM
 */
# include "common.h"
#ifndef M3HELPER_H
#define M3HELPER_H
void updateTravelTime(pathSeg & toAdd);
void addPathSegment(float heuristic,float travelTime,int currInter, int segId);
void visitNode(pathSeg current, float turn_penalty, int startId);
void visitSources(pathSeg current, float turn_penalty, bool isSource);
vector<unsigned> find_path_to_intrested_From(const unsigned intersect_id_start, vector<unsigned int> interest,const double turn_penalty);
vector<unsigned> constructPath(unsigned int intersect_id_start,unsigned int intersect_id_end);
void resetGraph(string name);
void resetGraph(int destination);
#endif /* M3HELPER_H */

