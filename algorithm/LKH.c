#include "../public.h"
#include "LKH.h"
#include "algorithm.h"
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief  LKH算法
 * @param  input            输入参数
 * @param  output           输出参数
 * @return int32_t          返回成功或者失败，RETURN_OK 或 RETURN_ERROR
 */
int32_t IOScheduleAlgorithmLKH(const InputParam *input, OutputParam *output)
{
    // 获取距离矩阵
    const Context ctx = {.input = input};
    const int MAT_SIZE = input->ioVec.len+2;
    int *matrix_2d = (int *)calloc(MAT_SIZE * MAT_SIZE, sizeof(int));
    getDistMatrix(input, (int *)matrix_2d);

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

    for (size_t i = 0; i < MAT_SIZE; i++)
    {
        for (size_t j = 0; j < MAT_SIZE; j++)
        {
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
    for (int i = 0; i < MAT_SIZE; i++) {
        printf("%d ", path[i]);
    }
    printf("\n");

    // 输出Output
    output->len=input->ioVec.len;
    for (size_t r_i = 0, w_i = 0; r_i < MAT_SIZE; r_i++)
    {
        int index = path[r_i]-1;
        if (index < output->len) {
            output->sequence[w_i] = input->ioVec.ioArray[index].id;
            // printf("%ld %d %d\n",r_i, index, input->ioVec.ioArray[index].id);
            w_i++;
        }
    }

    for (size_t i = 0; i < output->len; i++)
    {
        printf("%d ", output->sequence[i]);
    }


    // 释放动态分配的内存并关闭文件
    free(matrix_2d);
    free(path);
    fclose(file);

    chdir("../../build/");
    return RETURN_OK;
}
