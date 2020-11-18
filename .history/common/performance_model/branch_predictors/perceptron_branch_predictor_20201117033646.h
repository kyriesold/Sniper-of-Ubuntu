#ifndef PERCEPTRON_BRANCH_PREDICTOR_H
#define PERCEPTRON_BRANCH_PREDICTOR_H

#include "branch_predictor.h"
#include <vector>

class PerceptronBranchPredictor : public BranchPredictor
{
public:
   PerceptronBranchPredictor(String name, core_id_t core_id);
   ~PerceptronBranchPredictor();

   bool predict(IntPtr ip, IntPtr target);

   void update_global_history(bool predicted, bool actual, IntPtr ip, IntPtr target);

private:
    UInt32 threshold;   // decide when the training is done
    int global_history[index][num_of_history];  //According to paper, the best "num_of_history" 12-62
    vector<vector<int>> weight;   //weight[index][0]=1   need a bias
    vector<vector<int>> global_history;
};





#endif