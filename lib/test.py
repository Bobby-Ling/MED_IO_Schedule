# %%
# from libseek_model_wrapper import *

# start  = HeadInfo(wrap=0, lpos=100, status=1)
# target = HeadInfo(wrap=2, lpos=110, status=0)
# seg_wear_info = TapeBeltSegWearInfo()

# print(seek_time_calculate(start, target))
# print(belt_wear_times(start, target, seg_wear_info))

# %%
from pathlib import Path
from libseek_model_wrapper import IO_Schedule
file_dir = Path(__file__).parent

import numpy as np
from scipy.sparse.csgraph import minimum_spanning_tree
import networkx as nx

def solve_open_atsp_mst(distance_matrix):
    # 步骤1：构建最小生成树
    # 使用scipy生成最小生成树, 得到的是稀疏矩阵
    mst = minimum_spanning_tree(distance_matrix).toarray()

    # 步骤2：将最小生成树转换为无向图
    G = nx.Graph()
    n = distance_matrix.shape[0]

    # 添加节点和边
    for i in range(n):
        for j in range(n):
            if mst[i, j] != 0:  # 如果有边
                G.add_edge(i, j, weight=distance_matrix[i, j])

    # 步骤3：使用DFS或BFS获取路径 (DFS确保顺序)
    start_node = 0  # 可以从第一个城市开始
    path = list(nx.dfs_preorder_nodes(G, start_node))

    # 步骤4：处理成开环路径，不需要回到起点
    # 路径已经由 DFS 确保城市顺序的访问
    return np.array(path)

# %%
test1 = IO_Schedule(str(file_dir / "../dataset/case_8.txt"))
distance_matrix = test1.get_dist_matrix()[:-2, :-2]
# test1.run(IO_Schedule.METHOD.Greedy)
path = solve_open_atsp_mst(distance_matrix)
test1.path = path + 1
print(f'addressingDuration: {test1.address_duration()}ms')
test1.plot_path()
test1.run(IO_Schedule.METHOD.Greedy)
print(f'addressingDuration: {test1.address_duration()}ms')
test1.plot_path()

# %%
