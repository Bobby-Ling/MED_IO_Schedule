#include "dp.h"
#include "../public.h"
#include "algorithm.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

/**
 * @brief  算法接口
 * @param  input            输入参数
 * @param  output           输出参数
 * @return int32_t          返回成功或者失败，RETURN_OK 或 RETURN_ERROR
 */
int32_t IOScheduleAlgorithmDp(const InputParam *input, OutputParam *output)
{
    output->len=input->ioVec.len;

    const Context ctx = {.input = input};
    // return TspDp(input->ioVec.len, output->sequence, getNodeDist, &ctx);
    const int MAT_SIZE = input->ioVec.len+2;
    int *matrix_2d = calloc(MAT_SIZE * MAT_SIZE, sizeof(int));
    getDistMatrix(input, (int *)matrix_2d);
    for (size_t i = 0; i < MAT_SIZE; i++)
    {
        for (size_t j = 0; j < MAT_SIZE; j++)
        {
            printf("%6d ", *(matrix_2d + i * MAT_SIZE + j));
        }
        printf("\n");
    }

    return RETURN_ERROR;
}

/**
 * @brief               TSP 动态规划解法，使用状态压缩，获取路径
 * @param num_nodes     结点数目(10, 10000)
 * @param path          返回的路径列表(调用者分配)
 * @param getNodeDist   接口函数 getNodeDist(A, B, ctx), 返回 A 到 B 的距离
 * @return              最短距离
 */
uint32_t TspDp(uint32_t num_nodes, uint32_t * path, uint32_t (getNodeDist)(uint32_t idx_from, uint32_t idx_to, const Context *ctx), const Context *ctx)
{
    uint32_t dp[1 << num_nodes][num_nodes];
    uint32_t prev[1 << num_nodes][num_nodes];  // 保存路径中的前一个节点

    // 初始化 dp 和 prev 数组
    for (int mask = 0; mask < (1 << num_nodes); ++mask) {
        for (int i = 0; i < num_nodes; ++i) {
            dp[mask][i] = INT_MAX;
            prev[mask][i] = -1;  // -1 表示没有前驱节点
        }
    }

    // 从起点 0 开始，初始状态 mask 表示只访问了节点 0
    dp[1][0] = 0;

    // 遍历每个可能的状态 mask
    for (uint32_t mask = 0; mask < (1 << num_nodes); ++mask) {
        for (uint32_t i = 0; i < num_nodes; ++i) {
            if (mask & (1 << i)) {  // 如果 i 点在 mask 中
                for (uint32_t j = 0; j < num_nodes; ++j) {
                    if (!(mask & (1 << j))) {  // 如果 j 点没有在 mask 中
                        uint32_t new_mask = mask | (1 << j);
                        uint32_t new_dist = dp[mask][i] + getNodeDist(i, j, ctx);
                        if (new_dist < dp[new_mask][j]) {
                            dp[new_mask][j] = new_dist;
                            prev[new_mask][j] = i;  // 记录到达 j 点的前一个点是 i
                        }
                    }
                }
            }
        }
    }

    // 找出从所有点访问完回到起点的最短路径
    uint32_t min_dist = INT_MAX;
    uint32_t last_node = -1;

    for (uint32_t i = 1; i < num_nodes; ++i) {
        uint32_t final_dist = dp[(1 << num_nodes) - 1][i] + getNodeDist(i, 0, ctx);
        if (final_dist < min_dist) {
            min_dist = final_dist;
            last_node = i;  // 最后一个访问的节点
        }
    }

    // 回溯路径，从最后一个访问的节点开始
    uint32_t mask = (1 << num_nodes) - 1;  // 所有节点都已访问
    uint32_t index = num_nodes - 1;  // 路径数组的索引

    path[index] = 0;  // 起点
    path[0] = last_node;  // 最后一个访问的节点

    for (int i = last_node; index > 0; --index) {
        i = prev[mask][i];
        mask ^= (1 << path[index]);  // 去掉已访问的节点
        path[index - 1] = i;  // 记录路径
    }

    return min_dist;
}
