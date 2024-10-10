
#ifndef ALGORITHM_H
#define ALGORITHM_H

#include "../public.h"

#ifdef __cplusplus
extern "C" {
#endif

double calculateCost(const HeadInfo *current, const HeadInfo *end);
int32_t IOScheduleAlgorithm(const InputParam *input, OutputParam *output, int METHOD);
int32_t AlgorithmRun(const InputParam *input, OutputParam *output);

/**
 * @brief 捕获的局部变量
 */
typedef struct {
    const InputParam *input;
} Context;

uint32_t getNodeDist(uint32_t idx_from, uint32_t idx_to, const Context *ctx);
void getDistMatrix(const InputParam *input, int *matrix_2d);


#ifdef __cplusplus
}
#endif

#endif  // ALGORITHM_H