#include <stdio.h>
#include <stdlib.h>
#include "algorithm.h"

/**
 * @brief  算法接口
 * @param  input            输入参数
 * @param  output           输出参数
 * @return int32_t          返回成功或者失败，RETURN_OK 或 RETURN_ERROR
 */
int32_t IOScheduleAlgorithm(const InputParam *input, OutputParam *output)
{
    int32_t ret;

    /* 算法示例：先入先出算法 */
    output->len = input->ioVec.len;
    for (uint32_t i = 0; i < output->len; i++) {
        output->sequence[i] = input->ioVec.ioArray[i].id;
    }

    /* 调用公共函数示例：调用电机寻址、带体磨损、电机磨损函数 */
    HeadInfo start = {input->ioVec.ioArray[0].wrap, input->ioVec.ioArray[0].endLpos, HEAD_RW};
    HeadInfo end = {input->ioVec.ioArray[1].wrap, input->ioVec.ioArray[1].endLpos, HEAD_RW};
    int32_t seekT = 0;
    int32_t beltW = 0;
    int32_t motorW = 0;
       for (uint32_t i = 0; i < 10000; i++) {
           seekT = SeekTimeCalculate(&start, &end);
           beltW = BeltWearTimes(&start, &end, NULL);
           motorW = MotorWearTimes(&start, &end);
       }

    /* 调用公共函数示例：调用IO读写时间函数 */
    uint32_t rwT = ReadTimeCalculate(abs(input->ioVec.ioArray[0].endLpos - input->ioVec.ioArray[0].startLpos));

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
