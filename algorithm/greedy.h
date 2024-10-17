#ifndef GREEDY_H
#define GREEDY_H

#include "algorithm.h"

#ifdef __cplusplus

#define MAX_IO_REQUESTS MAX_IO_NUM

typedef struct {
    uint32_t id;
    double cost;
    bool visited;
}IORequest;

extern "C" {
#endif

int32_t IOScheduleAlgorithmGreedy(const InputParam *input, OutputParam *output);


#ifdef __cplusplus
}
#endif



#endif // GREEDY_H