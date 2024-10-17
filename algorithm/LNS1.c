#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <float.h>
#include "../public.h"
#include "algorithm.h"
#include "LNS.h"

#define MAX_IO_REQUESTS 10000

// 动态参数设置
static int INITIAL_SOLUTIONS;
static int LNS_ITERATIONS;
static double SA_INITIAL_TEMP;
static double SA_COOLING_RATE;

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
static double bestCost = DBL_MAX;

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

// 改进的贪心算法
static void improvedGreedy() {
    HeadInfo currentHead = globalInput->headInfo;
    uint32_t remainingRequests = globalInput->ioVec.len;
    uint32_t currentIndex = 0;

    for (uint32_t i = 0; i < globalInput->ioVec.len; i++) {
        requests[i].id = i;
        requests[i].visited = false;
    }

    while (remainingRequests > 0) {
        double minCost = DBL_MAX;
        int32_t bestCandidate = -1;

        for (uint32_t j = 0; j < globalInput->ioVec.len; j++) {
            if (!requests[j].visited) {
                IOUint *end_io = &globalInput->ioVec.ioArray[j];
                HeadInfo end = {end_io->wrap, end_io->endLpos, HEAD_RW};
                double cost = calculateCost(&currentHead, &end);

                if (cost < minCost) {
                    minCost = cost;
                    bestCandidate = j;
                }
            }
        }

        sequence[currentIndex++] = bestCandidate;
        requests[bestCandidate].visited = true;
        IOUint *chosen_io = &globalInput->ioVec.ioArray[bestCandidate];
        currentHead.wrap = chosen_io->wrap;
        currentHead.lpos = chosen_io->endLpos;
        currentHead.status = HEAD_RW;
        remainingRequests--;
    }

    currentCost = calculateTotalCost(sequence, globalInput->ioVec.len);
}

// 2-opt局部搜索
static void twoOptLocalSearch() {
    bool improved;
    do {
        improved = false;
        for (uint32_t i = 0; i < globalInput->ioVec.len - 1; i++) {
            for (uint32_t j = i + 1; j < globalInput->ioVec.len; j++) {
                uint32_t tempSequence[MAX_IO_REQUESTS];
                for (uint32_t k = 0; k < globalInput->ioVec.len; k++) {
                    tempSequence[k] = sequence[k];
                }
                // 反转子序列
                for (uint32_t k = i, l = j; k < l; k++, l--) {
                    uint32_t temp = tempSequence[k];
                    tempSequence[k] = tempSequence[l];
                    tempSequence[l] = temp;
                }
                double newCost = calculateTotalCost(tempSequence, globalInput->ioVec.len);
                if (newCost < currentCost) {
                    for (uint32_t k = 0; k < globalInput->ioVec.len; k++) {
                        sequence[k] = tempSequence[k];
                    }
                    currentCost = newCost;
                    improved = true;
                }
            }
        }
    } while (improved);
}

// 改进的大规模邻域搜索（LNS）
static void improvedLargeNeighborhoodSearch() {
    uint32_t tempSequence[MAX_IO_REQUESTS];
    uint32_t destroySize = globalInput->ioVec.len < 100 ? globalInput->ioVec.len / 5 : globalInput->ioVec.len / 10;

    for (int iter = 0; iter < LNS_ITERATIONS; iter++) {
        for (uint32_t i = 0; i < globalInput->ioVec.len; i++) {
            tempSequence[i] = sequence[i];
        }

        uint32_t start = rand() % (globalInput->ioVec.len - destroySize);
        uint32_t end = start + destroySize;

        HeadInfo currentHead = globalInput->headInfo;
        if (start > 0) {
            IOUint *prev_io = &globalInput->ioVec.ioArray[tempSequence[start - 1]];
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

            uint32_t chosenRequest = tempSequence[bestPos];
            for (int j = bestPos; j > (int)i; j--) {
                tempSequence[j] = tempSequence[j - 1];
            }
            tempSequence[i] = chosenRequest;

            IOUint *chosen_io = &globalInput->ioVec.ioArray[chosenRequest];
            currentHead.wrap = chosen_io->wrap;
            currentHead.lpos = chosen_io->endLpos;
            currentHead.status = HEAD_RW;
        }

        double newCost = calculateTotalCost(tempSequence, globalInput->ioVec.len);

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

// 动态参数调整
static void adjustParameters(uint32_t ioCount) {
    if (ioCount < 100) {
        INITIAL_SOLUTIONS = 5;
        LNS_ITERATIONS = ioCount * 10;
        SA_INITIAL_TEMP = 100.0;
        SA_COOLING_RATE = 0.99;
    }
    else if (ioCount < 1000) {
        INITIAL_SOLUTIONS = 10;
        LNS_ITERATIONS = ioCount * 5;
        SA_INITIAL_TEMP = 500.0;
        SA_COOLING_RATE = 0.995;
    }
    else {
        INITIAL_SOLUTIONS = 15;
        LNS_ITERATIONS = ioCount;
        SA_INITIAL_TEMP = 1000.0;
        SA_COOLING_RATE = 0.997;
    }
}

int32_t IOScheduleAlgorithmLNS1(const InputParam *input, OutputParam *output) {
    globalInput = input;
    srand(time(NULL));

    adjustParameters(input->ioVec.len);

    // 生成多个初始解并选择最好的
    for (int i = 0; i < INITIAL_SOLUTIONS; i++) {
        improvedGreedy();
        if (currentCost < bestCost) {
            bestCost = currentCost;
            for (uint32_t j = 0; j < input->ioVec.len; j++) {
                bestSequence[j] = sequence[j];
            }
        }
    }

    // 应用2-opt局部搜索
    for (uint32_t i = 0; i < input->ioVec.len; i++) {
        sequence[i] = bestSequence[i];
    }
    currentCost = bestCost;
    twoOptLocalSearch();

    // 应用改进的大规模邻域搜索
    improvedLargeNeighborhoodSearch();

    // 设置输出
    output->len = input->ioVec.len;
    for (uint32_t i = 0; i < output->len; i++) {
        output->sequence[i] = globalInput->ioVec.ioArray[bestSequence[i]].id;
    }

    return RETURN_OK;
}