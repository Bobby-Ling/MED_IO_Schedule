#ifndef BASE_SCAN_H
#define BASE_SCAN_H

#include "algorithm.h"
#define MAX_IO_REQUESTS MAX_IO_NUM

typedef struct {
    uint32_t id;
    double cost;
    bool visited;
}IORequest;
int32_t IOScheduleAlgorithmElevator(const InputParam *input, OutputParam *output);

typedef struct {
    int32_t direction;  // 1 表示向上扫描（从低到高），-1 表示向下扫描（从高到低）
    uint32_t farthestRequest;  // 用于记录最远的请求索引
} ScheduleState;

#endif // BASE_SCAN_H
