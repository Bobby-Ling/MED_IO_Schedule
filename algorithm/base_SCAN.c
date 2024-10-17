#include "../public.h"
#include "algorithm.h"
#include "base_SCAN.h"
#include <math.h>


int32_t IOScheduleAlgorithmElevator(const InputParam *input, OutputParam *output) {
    IORequest requests[MAX_IO_REQUESTS];
    HeadInfo currentHead = {input->headInfo.wrap, input->headInfo.lpos, input->headInfo.status};
    ScheduleState state = {1, 0};  // 开始时方向向上 (1 表示从起始位置到终点)

    // 初始化请求，将所有请求标记为未访问
    for (uint32_t i = 0; i < input->ioVec.len; i++) {
        requests[i].id = input->ioVec.ioArray[i].id;
        requests[i].visited = false;
    }
    output->len = input->ioVec.len;

    // 主循环，处理所有 IO 请求
    for (uint32_t i = 0; i < output->len; i++) {
        double minCost = __DBL_MAX__;
        int32_t nextIndex = -1;
        bool foundRequestInDirection = false;

        // 在当前方向上查找代价最小的请求
        for (uint32_t j = 0; j < input->ioVec.len; j++) {
            if (!requests[j].visited) {
                IOUint *end_io = &input->ioVec.ioArray[j];
                HeadInfo end = {end_io->wrap, end_io->endLpos, HEAD_RW};
                double cost = calculateCost(&currentHead, &end);

                // 确定请求的方向 (1 表示递增，-1 表示递减)
                int32_t requestDirection = (end.lpos > currentHead.lpos) ? 1 : -1;

                // 只选择当前方向的请求
                if (requestDirection == state.direction) {
                    if (cost < minCost) {
                        minCost = cost;
                        nextIndex = j;
                        foundRequestInDirection = true;
                    }
                }
            }
        }

        // 如果当前方向没有请求，切换方向
        if (!foundRequestInDirection) {
            state.direction *= -1;  // 改变方向
            i--;  // 保持循环计数不变，重新查找请求
            continue;
        }

        // 如果找到了下一个请求，将其添加到输出中并标记为已访问
        if (nextIndex != -1) {
            output->sequence[i] = requests[nextIndex].id;
            requests[nextIndex].visited = true;

            // 更新磁头位置
            currentHead.wrap = input->ioVec.ioArray[nextIndex].wrap;
            currentHead.lpos = input->ioVec.ioArray[nextIndex].endLpos;
            currentHead.status = HEAD_RW;
        } else {
            return RETURN_ERROR;  // 理论上不会出现这个情况
        }
    }

    return RETURN_OK;
}
