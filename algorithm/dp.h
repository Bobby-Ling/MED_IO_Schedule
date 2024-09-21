#include "algorithm.h"

/**
 * @brief               TSP 动态规划解法，使用状态压缩，获取路径
 * @param num_nodes     结点数目(10, 10000)
 * @param path          返回的路径列表(调用者分配)
 * @param getNodeDist   接口函数 getNodeDist(A, B, ctx), 返回 A 到 B 的距离
 * @return              最短距离
 */
uint32_t TspDp(uint32_t num_nodes, uint32_t *path, uint32_t (getNodeDist)(uint32_t idx_from, uint32_t idx_to, const Context *ctx), const Context *ctx);

/**
 * @brief  算法接口
 * @param  input            输入参数
 * @param  output           输出参数
 * @return int32_t          返回成功或者失败，RETURN_OK 或 RETURN_ERROR
 */
int32_t IOScheduleAlgorithmDp(const InputParam *input, OutputParam *output);
