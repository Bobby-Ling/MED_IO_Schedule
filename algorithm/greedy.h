#include "algorithm.h"

#define MAX_IO_REQUESTS MAX_IO_NUM

typedef struct {
    uint32_t id;
    double cost;
    bool visited;
}IORequest;

int32_t IOScheduleAlgorithmGreedy(const InputParam *input, OutputParam *output);
