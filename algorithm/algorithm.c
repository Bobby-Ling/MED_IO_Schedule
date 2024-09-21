#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "algorithm.h"
#include "dp.h"
#include "greedy.h"
#include "LKH.h"

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
 * @brief idx_from 到 idx_to 的距离
 * @param idx_from
 * @param idx_to
 * @param
 * @return 返回 idx_from 到 idx_to 的距离
 */
uint32_t getNodeDist(uint32_t idx_from, uint32_t idx_to,const Context *ctx)
{
    const InputParam *input = ctx->input;
    const int IO_NUM = input->ioVec.len;

    const int BEGIN_HEADER_IDX = IO_NUM;
    const int VIRTUAL_POINT_IDX = IO_NUM + 1;

    IOUint *from_io = NULL;
    IOUint *to_io = NULL;

    if (idx_from == idx_to) {
        return 0;
    } else if (idx_from == VIRTUAL_POINT_IDX || idx_to == VIRTUAL_POINT_IDX) {
        return 0;
    } else if (idx_from < IO_NUM && idx_to < IO_NUM) {
        from_io = &input->ioVec.ioArray[idx_from];
        to_io = &input->ioVec.ioArray[idx_to];
        HeadInfo from = {from_io->wrap, from_io->endLpos, HEAD_RW};
        HeadInfo to = {to_io->wrap, to_io->endLpos, HEAD_RW};
        return calculateCost(&from, &to);
    } else if (idx_from == BEGIN_HEADER_IDX) {
        // 此时idx_to在[0, IO_NUM-1]内
        HeadInfo from = input->headInfo;
        to_io = &input->ioVec.ioArray[idx_to];
        HeadInfo to = {to_io->wrap, to_io->endLpos, HEAD_RW};
        return calculateCost(&from, &to);
    } else if (idx_to == BEGIN_HEADER_IDX) {
        // 此时idx_from在[0, IO_NUM-1]内
        // 磁头位置只允许出边, 不允许入边
        return INT_MAX;
    } else {
        printf("error: not supposed to be here");
        return -1;
    }
}

/**
 * @brief 获取距离矩阵
 * @param input
 * @param matrix_2d
 */
void getDistMatrix(const InputParam *input, int *matrix_2d)
{
    const int IO_NUM = input->ioVec.len;
    const int MAT_SIZE = IO_NUM+2;
    const Context ctx = {.input = input};

    for (size_t i = 0; i < MAT_SIZE; i++)
    {
        for (size_t j = 0; j < MAT_SIZE; j++)
        {
            *(matrix_2d + i * MAT_SIZE + j) = getNodeDist(i, j, &ctx);
        }
    }
}

/**
 * @brief  算法接口
 * @param  input            输入参数
 * @param  output           输出参数
 * @return int32_t          返回成功或者失败，RETURN_OK 或 RETURN_ERROR
 */
int32_t IOScheduleAlgorithm(const InputParam *input, OutputParam *output)
{

    // return IOScheduleAlgorithmDp(input, output);
    return IOScheduleAlgorithmLKH(input, output);
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
