
#ifndef ALGORITHM_H
#define ALGORITHM_H

#include "../public.h"

#ifdef __cplusplus
extern "C" {
#endif

double calculateCost(const HeadInfo *current, const HeadInfo *end);
int32_t AlgorithmRun(const InputParam *input, OutputParam *output);

#ifdef __cplusplus
}
#endif

#endif  // ALGORITHM_H