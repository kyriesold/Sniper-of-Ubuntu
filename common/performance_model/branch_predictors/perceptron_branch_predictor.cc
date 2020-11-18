#include "simulator.h"
#include "perceptron_branch_predictor.h"
#include <vector>
#include <iterator>
using namespace std;

PerceptronBranchPredictor::PerceptronBranchPredictor(String name, core_id_t core_id, UInt32 size)
    :BranchPredictor(name, core_id)
{  // TODO 先实例化
    weight.resize(num_of_history);
    globalhistory.resize(num_of_history);
    for(int i=0;i<num_of_history;i++){
        weight[i].push_back(1);   // w0 is always 1
        for(int j=0;j<6;j++){
            weight[i].push_back(0);
        }
    }
    for(int i=0;i<num_of_history;i++){
        globalhistory[i].push_back(1);   // x0 is always 1,bias
        for(int j=0;j<5;j++){
            globalhistory[i].push_back(0);
        }
    }    
}

PerceptronBranchPredictor::~PerceptronBranchPredictor(){
}


bool PerceptronBranchPredictor::predict(IntPtr ip, IntPtr target){  //predict后会执行update  ip和target能对应
    UInt32 index = ip % num_of_history;
    bool result;
    y=0;
    for(int i=0;i<6;i++){
        y+=weight[index][i+1]*globalhistory[index][i];
    }
    y+=weight[index][0];  //(sum of weight[i])+weight[0]
    if(y>=0){
        result=1;
    }
    else
    {
        result=0;
    }
    //printf("called predict %d %d %d %d %d",ip,target,index,y,result);
    return result;
}

// void PerceptronBranchPredictor::update_global_history(bool predicted, bool actual, IntPtr ip, IntPtr target,int y){
//     //we need to know the index, actual, prediect
//     if(predicted!=actual||y<threshold){
//         for(int i=1;i<num_of_history+1;i++){
//             weight[index][i]+=global_history[index][i-1]*actual;
//         }
//     }
    //global_history.erase(global_history.begin()+1);  //update global_history
    //global_history.push_back(actual);



void PerceptronBranchPredictor::update(bool predicted, bool actual, IntPtr ip, IntPtr target)
{
    updateCounters(predicted, actual);   
    UInt32 index = ip % num_of_history;  //这部分之后可以用上面的update_global_history替代
    int temp;
    if(actual)
        temp=1;
    else
        temp=-1;
    //printf("called update %d %d %d %d %d %d \n",predicted,actual,ip,target,index,y);
    //if(predicted!=actual||y<threshold){
        for(int i=1;i<7;i++){
            // if(weight[index][i]<50&&weight[index][i]>-50)   //防止weight过大
                weight[index][i]+=globalhistory[index][i-1]*temp;
        }
    //}  
    globalhistory[index].push_back(temp);
    globalhistory[index].erase(globalhistory[index].begin()+1); 
    // weight[index].push_back(1);
    // weight[index].erase(weight[index].begin()+1); 
}