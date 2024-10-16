#include "../public.h"
#include "algorithm.h"
#include "base.h"
#include <math.h>
/**
 * @brief  算法接口
 * @param  input            输入参数
 * @param  output           输出参数
 * @return int32_t          返回成功或者失败，RETURN_OK 或 RETURN_ERROR
 */
int32_t IOScheduleAlgorithmBase(const InputParam *input, OutputParam *output)
{

    /* 算法示例：先入先出算法 */
    output->len = input->ioVec.len;
    for (uint32_t i = 0; i < output->len; i++) {
        output->sequence[i] = input->ioVec.ioArray[i].id;
    }

    return RETURN_OK;
}