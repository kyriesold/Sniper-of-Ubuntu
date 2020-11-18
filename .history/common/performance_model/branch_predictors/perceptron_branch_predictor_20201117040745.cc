#include "simulator.h"
#include "perceptron_branch_predictor.h"

PerceptronBranchPredictor::PerceptronBranchPredictor(String name, core_id_t core_id){  // TODO 先实例化
}

PerceptronBranchPredictor::~PerceptronBranchPredictor(){
}

bool PerceptronBranchPredictor::predict(IntPtr ip, IntPtr target){
    int y=0;
    for(int i=0;i<num_of_history;i++){
        y+=weight[index][i+1]+global_history[index][i]+weight[index][0];
    }
    return y>0;
}

void PerceptronBranchPredictor::update_global_history(bool predicted, bool actual, IntPtr ip, IntPtr target,int y){
    //we need to know the index, actual, prediect
    if(predicted!=actual||y<threshold){
        for(int i=1;i<num_of_history+1;i++){
            weight[index][i]+=global_history[index][i-1]*actual;
        }
    }
    global_history.erase(global_history.begin()+1);  //update global_history
    global_history.push_back(actual);

}

void PerceptronBranchPredictor::update(bool predicted, bool actual, IntPtr ip, IntPtr target)
{
   updateCounters(predicted, actual);   
   UInt32 index = ip % m_bits.size();  //这部分之后可以用上面的update_global_history替代
   m_bits[index] = actual;   
}