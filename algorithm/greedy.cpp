#include "../public.h"
#include "greedy.h"
#include "algorithm.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <set>
#include <vector>

#include <queue>    // For std::priority_queue
#include <tuple>    // For std::tuple
using namespace std;

// int32_t IOScheduleAlgorithmGreedy(const InputParam *input, OutputParam *output) {
//     // 使用优先队列（最小堆）来存储请求
//     auto compare = [](const std::tuple<double, uint32_t> &a, const std::tuple<double, uint32_t> &b) {
//         return std::get<0>(a) > std::get<0>(b); // 最小堆，所以比较器要返回大于
//         };
//     std::priority_queue<std::tuple<double, uint32_t>, std::vector<std::tuple<double, uint32_t>>, decltype(compare)> pq(compare);

//     HeadInfo currentHead = {input->headInfo.wrap, input->headInfo.lpos, input->headInfo.status};
//     output->len = input->ioVec.len;

//     // 初始化优先队列
//     for (uint32_t i = 0; i < input->ioVec.len; ++i) {
//         IOUint *req = &input->ioVec.ioArray[i];
//         HeadInfo end = {req->wrap, req->endLpos, HEAD_RW}; // 假设HEAD_RW是一个已定义的常量
//         double cost = calculateCost(&currentHead, &end);
//         pq.emplace(cost, i); // 将成本和请求索引一起存储
//     }

//     // 从优先队列中取出最小成本的请求，直到所有请求都已处理
//     for (uint32_t i = 0; i < output->len; ++i) {
//         if (pq.empty()) {
//             return RETURN_ERROR; // 如果队列为空，则输入数据可能有误
//         }

//         auto [cost, nextIndex] = pq.top();
//         pq.pop();


//         // 更新当前头位置
//         currentHead.wrap = input->ioVec.ioArray[nextIndex].wrap;
//         currentHead.lpos = input->ioVec.ioArray[nextIndex].endLpos;
//         // 注意：这里我们没有更新currentHead.status为HEAD_RW，因为这可能取决于具体的IO操作类型（读/写）
//         // 如果需要，请根据实际情况进行更新

//         // 将选中的请求添加到输出序列
//         output->sequence[i] = input->ioVec.ioArray[nextIndex].id;
//     }

//     return RETURN_OK;
// }



// /**
//  * @brief  算法接口
//  * @param  input            输入参数
//  * @param  output           输出参数
//  * @return int32_t          返回成功或者失败，RETURN_OK �? RETURN_ERROR
//  */
// int32_t IOScheduleAlgorithmGreedy(const InputParam *input, OutputParam *output) {
//     IORequest requests[MAX_IO_REQUESTS];
//     HeadInfo currentHead = {input->headInfo.wrap, input->headInfo.lpos, input->headInfo.status};
//     // Initialize requests
//     for (uint32_t i = 0; i < input->ioVec.len; i++) {
//         requests[i].id = input->ioVec.ioArray[i].id;
//         requests[i].visited = false;
//     }
//     output->len = input->ioVec.len;

//     for (uint32_t i = 0;i < output->len;i++) {
//         double minCost = __DBL_MAX__;
//         int32_t nextIndex = -1;
//         // Find the next request with minimum cost
//         for (uint32_t j = 0; j < input->ioVec.len; j++) {
//             if (!requests[j].visited) {
//                 IOUint *end_io = &input->ioVec.ioArray[j];
//                 HeadInfo end = {end_io->wrap, end_io->endLpos, HEAD_RW};
//                 double cost = calculateCost(&currentHead, &end);
//                 if (cost < minCost) {
//                     minCost = cost;
//                     nextIndex = j;
//                 }
//             }
//         }
//         if (nextIndex == -1) {
//             // This should not happen if the input is valid
//             return RETURN_ERROR;
//         }
//         // Add the selected request to the output sequence
//         output->sequence[i] = requests[nextIndex].id;
//         requests[nextIndex].visited = true;
//         // Update current head position
//         currentHead.wrap = input->ioVec.ioArray[nextIndex].wrap;
//         currentHead.lpos = input->ioVec.ioArray[nextIndex].endLpos;
//         currentHead.status = HEAD_RW;
//     }
//     return RETURN_OK;
// }

#define MAX_BATCH_SIZE 5

//被视为"附近"的阈值距离。
#define NEARBY_THRESHOLD 100  
#define FAR_THRESHOLD 10000
//用于跟踪调度状态，包括当前方向和最远请求的索引。
typedef struct {
    int32_t direction;  // 1 for increasing, -1 for decreasing
    uint32_t farthestRequest;
} ScheduleState;

/**
 * @brief  算法接口
 * @param  input            输入参数
 * @param  output           输出参数
 * @return int32_t          返回成功或者失败，RETURN_OK �? RETURN_ERROR
 */
int32_t IOScheduleAlgorithmGreedy(const InputParam *input, OutputParam *output) {

    IORequest requests[MAX_IO_REQUESTS];
    HeadInfo currentHead = {input->headInfo.wrap, input->headInfo.lpos, input->headInfo.status};
    ScheduleState state = {1, 0};  // Start with increasing direction
    
    // Initialize requests
    for (uint32_t i = 0; i < input->ioVec.len; i++) {
        requests[i].id = input->ioVec.ioArray[i].id;
        requests[i].visited = false;
    }
    
    output->len = 0;
    
    while (output->len < input->ioVec.len) {
        double minCost = __DBL_MAX__;
        int32_t nextIndex = -1;
        uint32_t batchSize = 0;
        uint32_t batchIndices[MAX_BATCH_SIZE];
        
        // Find the next request(s) with minimum cost
        for (uint32_t j = 0; j < input->ioVec.len; j++) {
            if (!requests[j].visited) {
                IOUint *end_io = &input->ioVec.ioArray[j];
                HeadInfo end = {end_io->wrap, end_io->endLpos, HEAD_RW};
                double cost = calculateCost(&currentHead, &end);
                
                // Consider direction
                int32_t requestDirection = ((currentHead.wrap%2)==end.wrap%2) ? 1 : -1;
                if (requestDirection == state.direction) {
                    cost *= 0.8;  // Favor requests in the same direction
                }
                
                // Check for nearby requests
                if (abs((int)(end.lpos - currentHead.lpos)) <= NEARBY_THRESHOLD) {
                    cost *= 0.9;  // Favor nearby requests
                }
                
                // Consider farthest request
                if ((j == state.farthestRequest<=FAR_THRESHOLD)) {
                    cost *= 0.8;  // Strongly favor the farthest request
                }
                
                if (cost < minCost) {
                    minCost = cost;
                    nextIndex = j;
                    batchSize = 1;
                    batchIndices[0] = j;
                } else if (cost == minCost && batchSize < MAX_BATCH_SIZE) {
                    batchIndices[batchSize++] = j;
                }
            }
        }
        
        if (nextIndex == -1) {
            return RETURN_ERROR;
        }
        
        // 处理批量请求
        for (uint32_t i = 0; i < batchSize; i++) {
            uint32_t index = batchIndices[i];
            output->sequence[output->len++] = requests[index].id;
            requests[index].visited = true;
        }
        
        // 更新当前磁头位置和状态
        currentHead.wrap = input->ioVec.ioArray[nextIndex].wrap;
        currentHead.lpos = input->ioVec.ioArray[nextIndex].endLpos;
        currentHead.status = HEAD_RW;
        
        state.direction = ((currentHead.wrap%2)==(input->ioVec.ioArray[nextIndex].wrap%2)) ? 1 : -1;
        
        // 更新最远请求
        state.farthestRequest = 0;
        uint32_t maxDistance = 0;
        for (uint32_t j = 0; j < input->ioVec.len; j++) {
            if (!requests[j].visited) {
                uint32_t distance = abs((int)(input->ioVec.ioArray[j].endLpos - currentHead.lpos));
                if (distance > maxDistance&&distance<FAR_THRESHOLD) {
                    maxDistance = distance;
                    state.farthestRequest = j;
                }
                
            }
        }
    }
    
    return RETURN_OK;
}
// struct IONode {
//     uint32_t id;
//     uint32_t index;
//     double cost;

//     bool operator<(const IONode& other) const {
//         if (cost != other.cost) {
//             return cost < other.cost;
//         }
//         return id < other.id;
//     }
// };

// double calculateCost(const HeadInfo* start, const IOUint* end) {
//     HeadInfo endHead = {end->wrap, end->endLpos, HEAD_RW};
//     uint32_t seekTime = SeekTimeCalculate(start, &endHead);
//     uint32_t readTime = ReadTimeCalculate(end->endLpos - end->startLpos);
//     return static_cast<double>(seekTime + readTime);
// }

// int32_t IOScheduleAlgorithmBalancedTree(const InputParam *input, OutputParam *output) {
//     std::vector<bool> visited(input->ioVec.len, false);
//     HeadInfo currentHead = input->headInfo;
//     std::set<IONode> tree;

//     output->len = input->ioVec.len;
//     output->sequence = new uint32_t[output->len];

//     // Initialize the balanced tree
//     for (uint32_t i = 0; i < input->ioVec.len; i++) {
//         double cost = calculateCost(&currentHead, &input->ioVec.ioArray[i]);
//         tree.insert({input->ioVec.ioArray[i].id, i, cost});
//     }

//     for (uint32_t i = 0; i < output->len; i++) {
//         if (tree.empty()) {
//             delete[] output->sequence;
//             return RETURN_ERROR;
//         }

//         // Get the node with minimum cost
//         auto minNode = tree.begin();
//         uint32_t nextIndex = minNode->index;

//         // Add the selected request to the output sequence
//         output->sequence[i] = minNode->id;
//         visited[nextIndex] = true;

//         // Remove the selected node from the tree
//         tree.erase(minNode);

//         // Update current head position
//         currentHead.wrap = input->ioVec.ioArray[nextIndex].wrap;
//         currentHead.lpos = input->ioVec.ioArray[nextIndex].endLpos;
//         currentHead.status = HEAD_RW;

//         // Update costs for remaining nodes
//         std::vector<IONode> nodesToUpdate;
//         for (const auto& node : tree) {
//             nodesToUpdate.push_back(node);
//         }
//         tree.clear();

//         for (const auto& node : nodesToUpdate) {
//             double newCost = calculateCost(&currentHead, &input->ioVec.ioArray[node.index]);
//             tree.insert({node.id, node.index, newCost});
//         }
//     }

//     return RETURN_OK;
// }
// #ifdef __cplusplus
// extern "C" {
// #endif

//     int32_t IOScheduleAlgorithmGreedy(const InputParam *input, OutputParam *output) {
//     // 使用优先队列（最小堆）来存储请求
//         auto compare = [](const std::tuple<double, uint32_t> &a, const std::tuple<double, uint32_t> &b) {
//             return std::get<0>(a) > std::get<0>(b); // 最小堆，所以比较器要返回大�?
//             };
//         std::priority_queue<std::tuple<double, uint32_t>, std::vector<std::tuple<double, uint32_t>>, decltype(compare)> pq(compare);

//         HeadInfo currentHead = {input->headInfo.wrap, input->headInfo.lpos, input->headInfo.status};
//         output->len = input->ioVec.len;

//         // 初�?�化优先队列
//         for (uint32_t i = 0; i < input->ioVec.len; ++i) {
//             IOUint *req = &input->ioVec.ioArray[i];
//             HeadInfo end = {req->wrap, req->endLpos, HEAD_RW}; // 假�?�HEAD_RW�?一�?已定义的常量
//             double cost = calculateCost(&currentHead, &end);
//             pq.emplace(cost, i); // 将成�?和�?�求索引一起存�?
//         }

//         // 从优先队列中取出最小成�?的�?�求，直到所有�?�求都�??处理
//         for (uint32_t i = 0; i < output->len; ++i) {
//             if (pq.empty()) {
//                 return RETURN_ERROR; // 如果队列为空，则输入数据�?能有�?�?
//             }

//             auto [cost, nextIndex] = pq.top();
//             pq.pop();

//             output->sequence[i] = input->ioVec.ioArray[nextIndex].id;

//             // 更新当前头位�?
//             currentHead.wrap = input->ioVec.ioArray[nextIndex].wrap;
//             currentHead.lpos = input->ioVec.ioArray[nextIndex].endLpos;
//             // 注意：这里我�?没有更新currentHead.status为HEAD_RW，因为这�?能取决于具体的IO操作类型（�??/写）
//             // 如果需要，请根�?实际情况进�?�更�?

//             // 如果需要，�?以从优先队列�?移除或更新已经�?�问过的请求的成�?（但这通常不是必�?�的，因为我�?不会再�?�选择它们�?
//         }

//         return RETURN_OK;
//     }

// #ifdef __cplusplus
// }
// #endif


// int32_t IOScheduleAlgorithmGreedy(const InputParam *input, OutputParam *output) {
//     IORequest requests[MAX_IO_REQUESTS];
//     HeadInfo currentHead = {input->headInfo.wrap, input->headInfo.lpos, input->headInfo.status};

//     // 初�?�化请求
//     for (uint32_t i = 0; i < input->ioVec.len; i++) {
//         requests[i].id = input->ioVec.ioArray[i].id;
//         requests[i].visited = 0; // 使用0表示�?访问�?1表示已�?�问
//     }
//     output->len = input->ioVec.len;

//     for (uint32_t i = 0; i < output->len; i++) {
//         int32_t nextIndex = -1;
//         double minCost = __DBL_MAX__;

//         // 在未访问的�?�求�?找到最小成�?的�?�求
//         for (uint32_t j = i; j < input->ioVec.len; j++) { // 从i开始可以减少不必�?�的检�?
//             if (!requests[j].visited) {
//                 IOUint *end_io = &input->ioVec.ioArray[j];
//                 HeadInfo end = {end_io->wrap, end_io->endLpos, HEAD_RW}; // 假�?�HEAD_RW�?一�?宏或常量
//                 double cost = calculateCost(&currentHead, &end);
//                 if (cost < minCost) {
//                     minCost = cost;
//                     nextIndex = j;
//                 }
//             }
//         }

//         if (nextIndex == -1) {
//             // 如果输入有效，则不应该发生这种情�?
//             return RETURN_ERROR;
//         }

//         // 将选中的�?�求与当前位置i的�?�求交换（实际上，我�?�?需要更新输出序列和标�?�为已�?�问�?
//         output->sequence[i] = requests[nextIndex].id;
//         requests[nextIndex].visited = 1;

//         // 但是，为了保持requests数组的顺序（虽然不是必需的，但有助于理解），我们�?以进行交�?
//         // 注意：这种交换在性能上可能不�?最优的，因为它引入了�?��?�的复制操作
//         // 在实际应用中，可以省略这一步，�?更新输出序列和�?�问标�??
//         if (nextIndex != i) {
//             IORequest temp = requests[i];
//             requests[i] = requests[nextIndex];
//             requests[nextIndex] = temp;
//             // 注意：这里没有更新currentHead，因为我�?已经知道了nextIndex请求的位�?
//             // 在实际情况下，�?�果需要更新currentHead，则应�?�根据nextIndex请求的实际位�?来更�?
//             // 但在这个算法�?，我�?�?在找到最小成�?请求后才更新currentHead，所以这里不需要更�?
//         }

//         // 更新当前头位�?（这一步应该根据nextIndex请求的实际位�?来执行）
//         currentHead.wrap = input->ioVec.ioArray[nextIndex].wrap;
//         currentHead.lpos = input->ioVec.ioArray[nextIndex].endLpos;
//         // 注意：通常我们不需要更新currentHead.status，除非有特定的逻辑要求
//         // 在这里，我们假�?�HEAD_RW�?�?一�?用于计算成本的占位�?�，不需要在每�?�迭代后更新
//     }

//     return RETURN_OK;
// }