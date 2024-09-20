# 思路

## 模型抽象

- 输入
当前的磁头位置`HeadInfo {wrap, lpos, status}`和一组`IOUint {id, wrap, startLpos, endLpos}`
- 输出
排序好的一组id

### 有向带权图的点对象

- 起始的磁头位置(index: 0)
磁头到每个io的距离计算: (磁头`{wrap, lpos, status}`, io`{wrap, startLpos, HEAD_RW}`)
- io(index: [1, IOVector.len])
io间的距离计算: (io_1`{wrap, endLpos, HEAD_RW}`, io_2`{wrap, startLpos, HEAD_RW}`)
- 虚拟点(index: IOVector.len+1)
需要一个到所有/起始结束点(TODO)对象距离为0的点, 这样可以把问题由开环TSP转化为一般的TSP.

## 资源限制 

```c
MAX_IO_REQUESTS == 10000
(10000*10000)*4/1024/1024 = 381.47 //MB > 20MB 
```

## 几个要注意的问题

- 磁头起始时选择最近的点可能造成最优解的忽略
- 空间复杂度要小于O(n^2)
