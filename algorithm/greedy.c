#include "../public.h"
#include "greedy.h"
#include "algorithm.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

/**
 * @brief  算法接口
 * @param  input            输入参数
 * @param  output           输出参数
 * @return int32_t          返回成功或者失败，RETURN_OK 或 RETURN_ERROR
 */
int32_t IOScheduleAlgorithmGreedy(const InputParam *input, OutputParam *output)
{
    IORequest requests[MAX_IO_REQUESTS];
    HeadInfo currentHead={input->headInfo.wrap, input->headInfo.lpos, input->headInfo.status};
    // Initialize requests
    for (uint32_t i = 0; i < input->ioVec.len; i++) {
        requests[i].id = input->ioVec.ioArray[i].id;
        requests[i].visited = false;
    }
    output->len=input->ioVec.len;

    for(uint32_t i=0;i<output->len;i++){
        double minCost=__DBL_MAX__;
        int32_t nextIndex=-1;
        // Find the next request with minimum cost
        for (uint32_t j = 0; j < input->ioVec.len; j++) {
            if (!requests[j].visited) {
                IOUint *end_io = &input->ioVec.ioArray[j];
                HeadInfo end = {end_io->wrap, end_io->endLpos, HEAD_RW};
                double cost = calculateCost(&currentHead, &end);
                if (cost < minCost) {
                    minCost = cost;
                    nextIndex = j;
                }
            }
        }
        if (nextIndex == -1) {
            // This should not happen if the input is valid
            return RETURN_ERROR;
        }
        // Add the selected request to the output sequence
        output->sequence[i] = requests[nextIndex].id;
        requests[nextIndex].visited = true;
        // Update current head position
        currentHead.wrap = input->ioVec.ioArray[nextIndex].wrap;
        currentHead.lpos = input->ioVec.ioArray[nextIndex].endLpos;
        currentHead.status = HEAD_RW;
    }
    return RETURN_OK;
}