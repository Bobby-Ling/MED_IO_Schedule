#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "algorithm.h"
#include "dp.h"
#include "greedy.h"

#define WEIGHT_SEEK 1         //寻道时间
#define WEIGHT_ABRASION 0     //带体磨损
#define WEIGHT_MOTOR 0        //电机磨损

/**
 * @brief  代价计算接口
 * @param  current            当前磁头信息
 * @param  io                 io列表
 * @return double             返回加权后的代价
 */
double calculateCost(const HeadInfo *current, const HeadInfo *end) {
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

    return IOScheduleAlgorithmDp(input, output);
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
