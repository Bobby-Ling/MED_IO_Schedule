# 思路

## 模型抽象

- 输入
当前的磁头位置`HeadInfo {wrap, lpos, status}`和一组io`IOUint {id, wrap, startLpos, endLpos}`, 可以看成一组点, 之间存在寻址代价
- 输出
排序好的一组id

### 有向带权图的点对象

- 起始的磁头位置(index: 0)
磁头到每个io的距离计算: (磁头`{wrap, lpos, status}`, io`{wrap, startLpos, HEAD_RW}`)
- io(index: [1, IOVector.len])
io间的距离计算: (io_1`{wrap, endLpos, HEAD_RW}`, io_2`{wrap, startLpos, HEAD_RW}`)
- 虚拟点(index: IOVector.len+1)
需要一个到所有/起始结束点(TODO)对象距离为0的点, 这样可以把问题由开环TSP转化为一般的TSP.

### 将OTSP转化为TSP

不考虑起始点的话, 添加一个到所有点距离为0的虚拟点, 则任意点与虚拟点的距离可以不计入代价/成本计算, 
最终虚拟点可以在tour中的**任意位置**, 比如假定io路径: [2, 3, 0, 1], 则输出可以为[2, 3, 虚拟点, 0, 1]

#### 应用LKH算法的问题

LKH算法会假定目标路径是一个环, 并通过交换边等方式进行"进化", 这样就

### 期望的一次tour

以一个存在4个io点、1个磁头、1个虚拟点的距离(寻址代价)矩阵为例, 
io点index: [0,3], 磁头(起始点)index==4, 虚拟点(结束点)index==5;
io的路径: [2, 3, 0, 1], 要达到的routine, 如[4, 2, 3, 0, 1, 5, 4] (为io路径)
允许的route: 4-io: Dist; 4/io-5: 0; 5-4: 0;
不允许的route: io-4; 5-io
即5只能是5->4, 4只能是4->io, io只能是io->io或io->5

## 资源限制 

```c
MAX_IO_REQUESTS == 10000
(10000*10000)*4/1024/1024 = 381.47 // MB > 10MB
sqrt(10*1024*1024)/4 = 810 // 二维int数组最大存储点数
```

## 几个要注意的问题

- 磁头起始时选择最近的点可能造成最优解的忽略

## TODO 根据不同的io分布选择不同策略

1. 小测试集: 要关注磁头到第一个io点的距离
