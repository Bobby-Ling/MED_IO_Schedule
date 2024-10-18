#ifndef LKH_EMBAEDED_H
#define LKH_EMBAEDED_H

#include "algorithm.h"
void ReadParameters();
static void Read_EDGE_WEIGHT_SECTION();
void ReadProblem();
int LKM_main();
int32_t IOScheduleAlgorithmLKH_embeded(const InputParam *input, OutputParam *output);

#endif // LKH_EMBAEDED_H
