#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "algorithm.h"

#define MAX_IO_REQUESTS MAX_IO_NUM
#define WEIGHT_SEEK 1         //寻道时间
#define WEIGHT_ABRASION 0     //带体磨损
#define WEIGHT_MOTOR 0        //电机磨损

typedef struct {
    uint32_t id;
    double cost;
    bool visited;
}IORequest;
/**
 * @brief  代价计算接口
 * @param  current            当前磁头信息
 * @param  io                 io列表
 * @return double             返回加权后的代价
 */
static double calculateCost(const HeadInfo *current, const HeadInfo *end) {
    double seekTime = SeekTimeCalculate(current, end);
    double beltWear = BeltWearTimes(current, end, NULL);
    double motorWear = MotorWearTimes(current, end);

    return WEIGHT_SEEK * seekTime + WEIGHT_ABRASION * beltWear + WEIGHT_MOTOR * motorWear;
}

/**
 * @brief  算法接口
 * @param  input            输入参数
 * @param  output           输出参数
 * @return int32_t          返回成功或者失败，RETURN_OK 或 RETURN_ERROR
 */
int32_t IOScheduleAlgorithm(const InputParam *input, OutputParam *output)
{
    // int32_t ret;

    // /* 算法示例：先入先出算法 */
    // output->len = input->ioVec.len;
    // for (uint32_t i = 0; i < output->len; i++) {
    //     output->sequence[i] = input->ioVec.ioArray[i].id;
    // }

    // /* 调用公共函数示例：调用电机寻址、带体磨损、电机磨损函数 */
    // HeadInfo start = {input->ioVec.ioArray[0].wrap, input->ioVec.ioArray[0].endLpos, HEAD_RW};
    // HeadInfo end = {input->ioVec.ioArray[1].wrap, input->ioVec.ioArray[1].endLpos, HEAD_RW};
    // int32_t seekT = 0;
    // int32_t beltW = 0;
    // int32_t motorW = 0;
    //    for (uint32_t i = 0; i < 10000; i++) {
    //        seekT = SeekTimeCalculate(&start, &end);
    //        beltW = BeltWearTimes(&start, &end, NULL);
    //        motorW = MotorWearTimes(&start, &end);
    //    }

    // /* 调用公共函数示例：调用IO读写时间函数 */
    // uint32_t rwT = ReadTimeCalculate(abs(input->ioVec.ioArray[0].endLpos - input->ioVec.ioArray[0].startLpos));

    // return RETURN_OK;
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

/**
 * @brief  算法运行的主入口
 * @param  input            输入参数
 * @param  output           输出参数
 * @return uint32_t          返回成功或者失败，RETURN_OK 或 RETURN_ERROR
 */
int32_t AlgorithmRun(const InputParam *input, OutputParam *output)
{
    int32_t ret;

    ret = IOScheduleAlgorithm(input, output);

    return RETURN_OK;
}
