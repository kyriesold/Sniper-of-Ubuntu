#ifndef PERCEPTRON_BRANCH_PREDICTOR_H
#define PERCEPTRON_BRANCH_PREDICTOR_H

#include "branch_predictor.h"
#include <vector>
using namespace std;

class PerceptronBranchPredictor : public BranchPredictor
{
public:
    vector<vector<int>> weight;
    vector<vector<int>> numhistory;
   PerceptronBranchPredictor(String name, core_id_t core_id);
   ~PerceptronBranchPredictor();

   bool predict(IntPtr ip, IntPtr target);

   void update_global_history(bool predicted, bool actual, IntPtr ip, IntPtr target);
   int row=20;int col=20;
private:
    UInt32 threshold;   // decide when the training is done
    static const int size = 20;
    static const int num_of_history = 20;
    //std::vector<bool> m_bits;
};





#endif