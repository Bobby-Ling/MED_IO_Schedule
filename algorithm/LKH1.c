#include "../public.h"
#include "LKH1.h"
#include "algorithm.h"
#include <unistd.h>
#include <stdlib.h>
#include <string.h>


uint32_t getNodeDistNoHead(uint32_t idx_from, uint32_t idx_to, const Context *ctx) {
    const InputParam *input = ctx->input;
    const int IO_NUM = input->ioVec.len;

    // const int BEGIN_HEADER_IDX = IO_NUM;
    // const int VIRTUAL_POINT_IDX = IO_NUM + 1;
    const int VIRTUAL_POINT_IDX = IO_NUM;

    // 起始点只允许出, 不允许入
    // const int START_IDX = BEGIN_HEADER_IDX;
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
        // if (idx_to = START_IDX) {
        //     // 1.1 END_IDX只允许到START_IDX
        //     return 0;
        // }
        // else {
         // 1.2 END_IDX一般不允许出
         // return INT_MAX;
        return 0;
    // }
    }
    else if (idx_from == idx_to) {
     // 2. 相等
        return 0;
    }
    // else if (idx_from == START_IDX) {
    //  // 3. START_IDX只允许出
    //     if (idx_to == END_IDX) {
    //         // 3.1 START_IDX不能到END_IDX
    //         // return INT_MAX;
    //         return 0;
    //     }
    //     else {
    //      // 3.2 此时idx_to在为IO_IDX
    //         HeadInfo from = input->headInfo;
    //         to_io = &input->ioVec.ioArray[idx_to];
    //         HeadInfo to = {to_io->wrap, to_io->endLpos, HEAD_RW};
    //         return calculateCost(&from, &to);
    //     }
    // }
    else {
     // 4. IO_IDX
        // if (idx_to == START_IDX) {
        //     // 4.1 START_IDX不允许入
        //     // return INT_MAX;
        //     return 0;
        // }
        /* else  */if (idx_to == END_IDX) {
         // 4.2 任意IO点到虚拟点距离为0
            return 0;
        }
        else {
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
void getDistMatrixNoHead(const InputParam *input, int *matrix_2d) {
    const int IO_NUM = input->ioVec.len;
    const int MAT_SIZE = IO_NUM + 1;
    const Context ctx = {.input = input};

    for (size_t i = 0; i < MAT_SIZE; i++) {
        for (size_t j = 0; j < MAT_SIZE; j++) {
            *(matrix_2d + i * MAT_SIZE + j) = getNodeDist(i, j, &ctx);
        }
    }
}

void rotate_list(int *tour_list, int size, int target) {
    // 查找 target 在 tour_list 中的索引
    int index = -1;
    for (int i = 0; i < size; i++) {
        if (tour_list[i] == target) {
            index = i;
            break;
        }
    }

    // 如果找到了目标值 target
    if (index != -1) {
        // 创建一个临时数组来存储旋转后的结果
        int *rotated_list = (int *)malloc(size * sizeof(int));

        // 从找到的索引开始复制后面的部分
        int rotated_index = 0;
        for (int i = index; i < size; i++) {
            rotated_list[rotated_index++] = tour_list[i];
        }

        // 复制从开头到找到索引前的部分
        for (int i = 0; i < index; i++) {
            rotated_list[rotated_index++] = tour_list[i];
        }

        // 将旋转后的列表复制回原列表
        for (int i = 0; i < size; i++) {
            tour_list[i] = rotated_list[i];
        }

        // 释放临时数组
        free(rotated_list);
    }
}

/**
 * @brief  LKH算法
 * @param  input            输入参数
 * @param  output           输出参数
 * @return int32_t          返回成功或者失败，RETURN_OK 或 RETURN_ERROR
 */
int32_t IOScheduleAlgorithmLKH1(const InputParam *input, OutputParam *output) {
    // 获取距离矩阵
    const Context ctx = {.input = input};
    const int MAT_SIZE = input->ioVec.len + 1;
    int *matrix_2d = (int *)calloc(MAT_SIZE * MAT_SIZE, sizeof(int));
    getDistMatrixNoHead(input, (int *)matrix_2d);

    // 保存距离矩阵至文件
    chdir("../lib/LKH/");
    FILE *matrix_file = fopen("io.atsp", "w+");

    fprintf(matrix_file, "NAME : IO\n");
    fprintf(matrix_file, "TYPE : ATSP\n");
    fprintf(matrix_file, "DIMENSION : %d\n", MAT_SIZE);
    fprintf(matrix_file, "VEHICLES : 1\n");
    fprintf(matrix_file, "EDGE_WEIGHT_TYPE: EXPLICIT\n");
    fprintf(matrix_file, "EDGE_WEIGHT_FORMAT: FULL_MATRIX\n");
    fprintf(matrix_file, "EDGE_WEIGHT_SECTION\n");

    for (size_t i = 0; i < MAT_SIZE; i++) {
        for (size_t j = 0; j < MAT_SIZE; j++) {
            fprintf(matrix_file, "%6d ", *(matrix_2d + i * MAT_SIZE + j));
        }
        fprintf(matrix_file, "\n");
    }

    fclose(matrix_file);

    // 运行LKH
    // OUTPUT_TOUR_FILE = LKH.result
    system("./LKH io.par");

    // 解析输出文件
    FILE *file = fopen("LKH.result", "r");

    char line[256];
    int *path = (int *)calloc(MAT_SIZE, sizeof(int));;
    int found_tour_section = 0;
    int index = 0;

    // 逐行读取文件内容
    while (fgets(line, sizeof(line), file)) {
        // 查找 TOUR_SECTION 部分
        if (strncmp(line, "TOUR_SECTION", 12) == 0) {
            found_tour_section = 1;
            continue;  // 跳过 TOUR_SECTION 行
        }

        // 如果已经找到 TOUR_SECTION，开始读取整数
        if (found_tour_section) {
            int value;
            sscanf(line, "%d", &value);
            // 如果遇到 -1 结束
            if (value == -1) {
                break;
            }
            // 将读取到的整数保存到 path 数组中
            if (index < MAT_SIZE) {
                path[index++] = value;
            }
        }
    }

    // 输出 path 数组
    printf("Tour Path:\n");
    // 2 5 3 4 1 MAT_SIZE=5, VIRTUAL_POINT_IDX=4
    for (int i = 0; i < MAT_SIZE; i++) {
        printf("%d ", path[i]);
    }
    printf("\n");

    rotate_list(path, MAT_SIZE, MAT_SIZE);

    printf("Rotated Tour Path:\n");
    // 5 3 4 1 2 MAT_SIZE=5, VIRTUAL_POINT_IDX=4
    for (int i = 0; i < MAT_SIZE; i++) {
        printf("%d ", path[i]);
    }
    printf("\n");

    // 输出Output
    output->len = input->ioVec.len;
    for (size_t r_i = 0, w_i = 0; r_i < MAT_SIZE; r_i++) {
        int index = path[r_i] - 1;
        if (index < output->len) {
            output->sequence[w_i] = input->ioVec.ioArray[index].id;
            // printf("%ld %d %d\n",r_i, index, input->ioVec.ioArray[index].id);
            w_i++;
        }
    }

    for (size_t i = 0; i < output->len; i++) {
        printf("%d ", output->sequence[i]);
    }


    // 释放动态分配的内存并关闭文件
    free(matrix_2d);
    free(path);
    fclose(file);

    chdir("../../build/");
    return RETURN_OK;
}
