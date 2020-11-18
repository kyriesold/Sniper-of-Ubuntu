#ifndef PERCEPTRON_BRANCH_PREDICTOR_H
#define PERCEPTRON_BRANCH_PREDICTOR_H

#include "branch_predictor.h"
#include <vector>
using namespace std;

class PerceptronBranchPredictor : public BranchPredictor
{
public:
    vector<vector<int>> weight;
    vector<vector<int>> globalhistory;
   PerceptronBranchPredictor(String name, core_id_t core_id, UInt32 size);
   ~PerceptronBranchPredictor();

   bool predict(IntPtr ip, IntPtr target);

   void update(bool predicted, bool actual, IntPtr ip, IntPtr target);
private:
    int y=0;
    UInt32 threshold=100;   // decide when the training is done
    static const int size_set;
    static const int num_of_history=1024;
    //std::vector<bool> m_bits;
};





#endif