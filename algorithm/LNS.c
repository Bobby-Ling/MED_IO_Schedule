#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <float.h>
#include "../public.h"
#include "algorithm.h"
#include "LNS.h"
#include <math.h>
#define MAX_IO_REQUESTS 10000
static int INITIAL_SOLUTIONS = 10;
static int LNS_ITERATIONS = 1000;
static int SA_INITIAL_TEMP = 1000.0;
static int SA_COOLING_RATE = 0.995;

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

// 使用现有的数据结构
typedef struct {
    uint32_t id;
    bool visited;
} IORequest;
// 全局变量
static const InputParam *globalInput;
static IORequest requests[MAX_IO_REQUESTS];
static uint32_t sequence[MAX_IO_REQUESTS];
static uint32_t bestSequence[MAX_IO_REQUESTS];
static double currentCost;
static double bestCost = 1.79769313486231570814527423731704357e+308;

// 计算总代价
static double calculateTotalCost(uint32_t *seq, uint32_t len) {
    double totalCost = 0;
    HeadInfo currentHead = globalInput->headInfo;

    for (uint32_t i = 0; i < len; i++) {
        uint32_t index = seq[i];
        IOUint *end_io = &globalInput->ioVec.ioArray[index];
        HeadInfo end = {end_io->wrap, end_io->endLpos, HEAD_RW};
        totalCost += calculateCost(&currentHead, &end);
        currentHead = end;
    }

    return totalCost;
}

// 随机贪心算法
static void randomGreedy() {
    HeadInfo currentHead = globalInput->headInfo;
    uint32_t remainingRequests = globalInput->ioVec.len;
    uint32_t currentIndex = 0;

    for (uint32_t i = 0; i < globalInput->ioVec.len; i++) {
        requests[i].id = i;
        requests[i].visited = false;
    }

    while (remainingRequests > 0) {
        double minCosts[3] = {DBL_MAX, DBL_MAX, DBL_MAX};
        int32_t candidates[3] = {-1, -1, -1};

        for (uint32_t j = 0; j < globalInput->ioVec.len; j++) {
            if (!requests[j].visited) {
                IOUint *end_io = &globalInput->ioVec.ioArray[j];
                HeadInfo end = {end_io->wrap, end_io->endLpos, HEAD_RW};
                double cost = calculateCost(&currentHead, &end);

                if (cost < minCosts[2]) {
                    minCosts[2] = cost;
                    candidates[2] = j;
                    for (int k = 1; k >= 0; k--) {
                        if (minCosts[k] > minCosts[k+1]) {
                            double tempCost = minCosts[k];
                            minCosts[k] = minCosts[k+1];
                            minCosts[k+1] = tempCost;
                            int32_t tempCandidate = candidates[k];
                            candidates[k] = candidates[k+1];
                            candidates[k+1] = tempCandidate;
                        }
                    }
                }
            }
        }

        int32_t chosen = candidates[rand() % (remainingRequests < 3 ? remainingRequests : 3)];
        sequence[currentIndex++] = chosen;
        requests[chosen].visited = true;
        IOUint *chosen_io = &globalInput->ioVec.ioArray[chosen];
        currentHead.wrap = chosen_io->wrap;
        currentHead.lpos = chosen_io->endLpos;
        currentHead.status = HEAD_RW;
        remainingRequests--;
    }

    currentCost = calculateTotalCost(sequence, globalInput->ioVec.len);
}

// 大规模邻域搜索（LNS）
static void largeNeighborhoodSearch() {
    uint32_t tempSequence[MAX_IO_REQUESTS];
    uint32_t destroySize = globalInput->ioVec.len / 10;  // 破坏规模为总请求数的10%

    for (int iter = 0; iter < LNS_ITERATIONS; iter++) {
        // 复制当前序列
        for (uint32_t i = 0; i < globalInput->ioVec.len; i++) {
            tempSequence[i] = sequence[i];
        }

        // 随机选择要破坏的子序列
        uint32_t start = rand() % (globalInput->ioVec.len - destroySize);
        uint32_t end = start + destroySize;

        // 使用贪心算法重建子序列
        HeadInfo currentHead = globalInput->headInfo;
        if (start > 0) {
            IOUint *prev_io = &globalInput->ioVec.ioArray[tempSequence[start-1]];
            currentHead.wrap = prev_io->wrap;
            currentHead.lpos = prev_io->endLpos;
            currentHead.status = HEAD_RW;
        }

        for (uint32_t i = start; i < end; i++) {
            double minCost = DBL_MAX;
            int32_t bestPos = i;

            for (uint32_t j = i; j < end; j++) {
                IOUint *end_io = &globalInput->ioVec.ioArray[tempSequence[j]];
                HeadInfo endHead = {end_io->wrap, end_io->endLpos, HEAD_RW};
                double cost = calculateCost(&currentHead, &endHead);

                if (cost < minCost) {
                    minCost = cost;
                    bestPos = j;
                }
            }

            // 将选中的请求插入到最佳位置
            uint32_t chosenRequest = tempSequence[bestPos];
            for (int j = bestPos; j > (int)i; j--) {
                tempSequence[j] = tempSequence[j-1];
            }
            tempSequence[i] = chosenRequest;

            IOUint *chosen_io = &globalInput->ioVec.ioArray[chosenRequest];
            currentHead.wrap = chosen_io->wrap;
            currentHead.lpos = chosen_io->endLpos;
            currentHead.status = HEAD_RW;
        }

        double newCost = calculateTotalCost(tempSequence, globalInput->ioVec.len);

        // 模拟退火决定是否接受新解
        double temperature = SA_INITIAL_TEMP * pow(SA_COOLING_RATE, iter);
        if (newCost < currentCost ||
            (double)rand() / RAND_MAX < exp((currentCost - newCost) / temperature)) {
            for (uint32_t i = 0; i < globalInput->ioVec.len; i++) {
                sequence[i] = tempSequence[i];
            }
            currentCost = newCost;

            if (currentCost < bestCost) {
                bestCost = currentCost;
                for (uint32_t i = 0; i < globalInput->ioVec.len; i++) {
                    bestSequence[i] = sequence[i];
                }
            }
        }
    }
}

int32_t IOScheduleAlgorithmLNS(const InputParam *input, OutputParam *output) {
    INITIAL_SOLUTIONS = 10;
    LNS_ITERATIONS = max((int)(min(input->ioVec.len, 1000)) + 10, 500);
    SA_INITIAL_TEMP = 1000.0;
    SA_COOLING_RATE = 0.995;

    // 由于可能在动态库多次调用, 因此每次都要手动重置!!!
    currentCost = 0;
    bestCost = 1.79769313486231570814527423731704357e+308;
    globalInput = input;
    srand(time(NULL));

    // 生成多个初始解并选择最好的
    for (int i = 0; i < INITIAL_SOLUTIONS; i++) {
        randomGreedy();
        if (currentCost < bestCost) {
            bestCost = currentCost;
            for (uint32_t j = 0; j < input->ioVec.len; j++) {
                bestSequence[j] = sequence[j];
            }
        }
    }

    // 应用大规模邻域搜索
    for (uint32_t i = 0; i < input->ioVec.len; i++) {
        sequence[i] = bestSequence[i];
    }
    currentCost = bestCost;
    largeNeighborhoodSearch();

    // 设置输出
    output->len = input->ioVec.len;
    for (uint32_t i = 0; i < output->len; i++) {
        output->sequence[i] = globalInput->ioVec.ioArray[bestSequence[i]].id;
    }

    return RETURN_OK;
}