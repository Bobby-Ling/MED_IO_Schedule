#include "../public.h"
#include "algorithm.h"
#include "greedy1.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#define MAX_BATCH_SIZE 5

//被视为"附近"的阈值距离。
#define NEARBY_THRESHOLD 100
#define FAR_THRESHOLD 10000
//用于跟踪调度状态，包括当前方向和最远请求的索引。
typedef struct {
    int32_t direction;  // 1 for increasing, -1 for decreasing
    uint32_t farthestRequest;
} ScheduleState;

/**
 * @brief  算法接口
 * @param  input            输入参数
 * @param  output           输出参数
 * @return int32_t          返回成功或者失败，RETURN_OK �? RETURN_ERROR
 */
int32_t IOScheduleAlgorithmGreedy1(const InputParam *input, OutputParam *output) {

    IORequest requests[MAX_IO_REQUESTS];
    HeadInfo currentHead = {input->headInfo.wrap, input->headInfo.lpos, input->headInfo.status};
    ScheduleState state = {1, 0};  // Start with increasing direction

    // Initialize requests
    for (uint32_t i = 0; i < input->ioVec.len; i++) {
        requests[i].id = input->ioVec.ioArray[i].id;
        requests[i].visited = false;
    }

    output->len = 0;

    while (output->len < input->ioVec.len) {
        double minCost = __DBL_MAX__;
        int32_t nextIndex = -1;
        uint32_t batchSize = 0;
        uint32_t batchIndices[MAX_BATCH_SIZE];

        // Find the next request(s) with minimum cost
        for (uint32_t j = 0; j < input->ioVec.len; j++) {
            if (!requests[j].visited) {
                IOUint *end_io = &input->ioVec.ioArray[j];
                HeadInfo end = {end_io->wrap, end_io->endLpos, HEAD_RW};
                double cost = calculateCost(&currentHead, &end);

                // Consider direction
                int32_t requestDirection = ((currentHead.wrap % 2) == end.wrap % 2) ? 1 : -1;
                if (requestDirection == state.direction) {
                    cost *= 0.8;  // Favor requests in the same direction
                }

                // Check for nearby requests
                if (abs((int)(end.lpos - currentHead.lpos)) <= NEARBY_THRESHOLD) {
                    cost *= 0.9;  // Favor nearby requests
                }

                // Consider farthest request
                if ((j == state.farthestRequest <= FAR_THRESHOLD)) {
                    cost *= 0.8;  // Strongly favor the farthest request
                }

                if (cost < minCost) {
                    minCost = cost;
                    nextIndex = j;
                    batchSize = 1;
                    batchIndices[0] = j;
                }
                else if (cost == minCost && batchSize < MAX_BATCH_SIZE) {
                    batchIndices[batchSize++] = j;
                }
            }
        }

        if (nextIndex == -1) {
            return RETURN_ERROR;
        }

        // 处理批量请求
        for (uint32_t i = 0; i < batchSize; i++) {
            uint32_t index = batchIndices[i];
            output->sequence[output->len++] = requests[index].id;
            requests[index].visited = true;
        }

        // 更新当前磁头位置和状态
        currentHead.wrap = input->ioVec.ioArray[nextIndex].wrap;
        currentHead.lpos = input->ioVec.ioArray[nextIndex].endLpos;
        currentHead.status = HEAD_RW;

        state.direction = ((currentHead.wrap % 2) == (input->ioVec.ioArray[nextIndex].wrap % 2)) ? 1 : -1;

        // 更新最远请求
        state.farthestRequest = 0;
        uint32_t maxDistance = 0;
        for (uint32_t j = 0; j < input->ioVec.len; j++) {
            if (!requests[j].visited) {
                uint32_t distance = abs((int)(input->ioVec.ioArray[j].endLpos - currentHead.lpos));
                if (distance > maxDistance && distance < FAR_THRESHOLD) {
                    maxDistance = distance;
                    state.farthestRequest = j;
                }

            }
        }
    }

    return RETURN_OK;
}