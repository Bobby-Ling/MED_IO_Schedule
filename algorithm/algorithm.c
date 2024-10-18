#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/time.h>
#include "algorithm.h"
#include "dp.h"
#include "greedy.h"
#include "greedy1.h"
#include "LKH0.h"
#include "LKH1.h"
#include "LKH_embeded.h"
#include "base.h"
#include "base_SCAN.h"
#include "LNS.h"
#include "LNS1.h"
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
 * @param ctx
 * @return 返回 idx_from 到 idx_to 的距离
 */
uint32_t getNodeDist(uint32_t idx_from, uint32_t idx_to, const Context *ctx)
{
    const InputParam *input = ctx->input;
    const int IO_NUM = input->ioVec.len;

    const int BEGIN_HEADER_IDX = IO_NUM;
    const int VIRTUAL_POINT_IDX = IO_NUM + 1;

    // 起始点只允许出, 不允许入
    const int START_IDX = BEGIN_HEADER_IDX;
    // 结束点只允许入, 不允许出
    // 为了避免环, 需要指定一个虚拟点作为结束点, 要保证任何点可以没有代价地到达虚拟点, 且路径不能通过虚拟点传递
    // 因此, 任意IO点到虚拟点距离为0, 虚拟点和起始点到自己的距离为INF
    const int END_IDX = VIRTUAL_POINT_IDX;
    // START_IDX -> START_IDX     0         2.
    // START_IDX ->   END_IDX     INF       3.1
    // START_IDX ->    IO_IDX     Dist      3.2
    //   END_IDX -> START_IDX     0         1.1
    //   END_IDX ->   END_IDX     INF       1.2
    //   END_IDX ->    IO_IDX     INF       1.2
    //    IO_IDX -> START_IDX     INF       4.1
    //    IO_IDX ->   END_IDX     0         4.2
    //    IO_IDX ->    IO_IDX     Dist/0    4.3/2.
    IOUint *from_io = NULL;
    IOUint *to_io = NULL;

    if (idx_from == END_IDX) {
        // 1. END_IDX
        if (idx_to = START_IDX) {
            // 1.1 END_IDX只允许到START_IDX
            return 0;
        } else {
            // 1.2 END_IDX一般不允许出
            // return INT_MAX;
            return 0;
        }
    } else if (idx_from == idx_to){
        // 2. 相等
        return 0;
    } else if (idx_from == START_IDX) {
        // 3. START_IDX只允许出
        if (idx_to == END_IDX) {
            // 3.1 START_IDX不能到END_IDX
            // return INT_MAX;
            return 0;
        } else {
            // 3.2 此时idx_to在为IO_IDX
            HeadInfo from = input->headInfo;
            to_io = &input->ioVec.ioArray[idx_to];
            HeadInfo to = {to_io->wrap, to_io->endLpos, HEAD_RW};
            return calculateCost(&from, &to);
        }
    } else {
        // 4. IO_IDX
        if (idx_to == START_IDX) {
            // 4.1 START_IDX不允许入
            // return INT_MAX;
            return 0;
        } else if (idx_to == END_IDX) {
            // 4.2 任意IO点到虚拟点距离为0
            return 0;
        } else {
            // 4.3 IO->IO
            from_io = &input->ioVec.ioArray[idx_from];
            to_io = &input->ioVec.ioArray[idx_to];
            HeadInfo from = {from_io->wrap, from_io->endLpos, HEAD_RW};
            HeadInfo to = {to_io->wrap, to_io->endLpos, HEAD_RW};
            return calculateCost(&from, &to);
        }
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

double algorithmRunningDuration;
double getAlgorithmRunningDuration() {
    return algorithmRunningDuration;
}

/**
 * @brief  算法接口
 * @param  input            输入参数
 * @param  output           输出参数
 * @param  METHOD           算法选择
 * @return int32_t          返回成功或者失败，RETURN_OK 或 RETURN_ERROR
 */
int32_t IOScheduleAlgorithm(const InputParam *input, OutputParam *output, int METHOD)
{
    /* 统计算法运行时间 */
    struct timeval start, end;
    gettimeofday(&start, NULL);
    switch (METHOD)
    {
    case 0:
        // printf("IOScheduleAlgorithmGreedy:\n");
        IOScheduleAlgorithmGreedy(input, output);
        break;
    case 1:
        // printf("IOScheduleAlgorithmLKH1:\n");
        IOScheduleAlgorithmLKH1(input, output);
        break;
    case 2:
        // printf("IOScheduleAlgorithmLKH:\n");
        IOScheduleAlgorithmLKH(input, output);
        break;
    case 3:
        // printf("IOScheduleAlgorithmBase:\n");
        IOScheduleAlgorithmBase(input, output);
        break;
    case 4:
        // printf("IOScheduleAlgorithmElevator:\n");
        IOScheduleAlgorithmElevator(input, output);
        break;
    case 5:
        // printf("IOScheduleAlgorithmGreedy1:\n");
        IOScheduleAlgorithmGreedy1(input, output);
        break;
    case 6:
        // printf("IOScheduleAlgorithmLNS:\n");
        IOScheduleAlgorithmLNS(input, output);
        break;
    case 7:
        // printf("IOScheduleAlgorithmLNS1:\n");
        IOScheduleAlgorithmLNS1(input, output);
        break;
    case 8:
        // printf("IOScheduleAlgorithmLKH_embeded:\n");
        IOScheduleAlgorithmLKH_embeded(input, output);
        break;
    case 9:
        // printf("IOScheduleAlgorithmLKH_embeded & IOScheduleAlgorithmLNS:\n");
        if (input->ioVec.len < 800) {
            IOScheduleAlgorithmLKH_embeded(input, output);
        }
        else {
            IOScheduleAlgorithmLNS(input, output);
        }
        break;
    default:
    }
    gettimeofday(&end, NULL);  // 记录结束时间
    long seconds, useconds;    // 秒数和微秒数
    seconds = end.tv_sec - start.tv_sec;
    useconds = end.tv_usec - start.tv_usec;
    /* 总毫秒数 */
    algorithmRunningDuration = ((seconds) * 1000000 + useconds) / 1000.0;
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

    // e.g. $ METHOD=2 ./project_hw -f ../dataset/case_5.txt
    int METHOD = atoi(getenv("METHOD") ? getenv("METHOD") : "4");
    ret = IOScheduleAlgorithm(input, output, METHOD);

    return RETURN_OK;
}
