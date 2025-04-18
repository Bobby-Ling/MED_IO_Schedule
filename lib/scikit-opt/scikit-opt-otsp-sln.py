import os
import sys
import numpy as np
from scipy import spatial
import matplotlib.pyplot as plt

# from sko.GA import GA_TSP
# 项目主页 https://github.com/guofei9987/scikit-opt
# 文档 https://scikit-opt.github.io/scikit-opt/#/zh/README

# %%
sys.path.append('..')
import libseek_model_wrapper as lsm

# 本文件用来快速验证算法效果, 可以使用libseek_model_wrapper中封装好的C函数

# %%
# num_points = 50
# np.random.seed(0)
# points_coordinate = np.random.rand(num_points, 2)  # generate coordinate of points

# %%
from pathlib import Path
file_dir = Path(__file__).parent
from tsp_parser import read_tsp_file
points_coordinate = np.array(read_tsp_file(str(file_dir / '../ALL_tsp/u159.tsp')))
num_points = len(points_coordinate)

# %%
start_point = [[0,0]]
end_point = [[1,1]]
points_coordinate = np.concatenate([points_coordinate, start_point, end_point])
distance_matrix = spatial.distance.cdist(points_coordinate, points_coordinate, metric='euclidean')
mat_size = distance_matrix.shape[0]
# %%
INF = np.iinfo(np.int32).max
distance_matrix[:num_points, num_points] = np.ones(shape=(num_points, )) * INF
distance_matrix[:num_points+1, num_points+1] = np.zeros(shape=(num_points+1, ))
distance_matrix[num_points+1, :num_points+1] = np.ones(shape=(num_points+1, )) * INF

# %%
def cal_total_distance(routine: np.ndarray):
    '''The objective function. input routine, return total distance.
    cal_total_distance(np.arange(num_points))
    '''
    OPTION = 4

    match OPTION:
        case 0:
            # TSP闭环
            # routine-routine[0], 如[2, 3, 0, 1, 2]
            num_points = routine.shape[0] - 2
            routine = routine[:num_points]
            routine = np.concatenate([routine, [routine[0]]])
            return sum([distance_matrix[routine[i], routine[i + 1]] for i in range(routine.shape[0]-1)])

        case 1:
            # TSP开环
            # 路径仅routine, 如[2, 3, 0, 1]
            num_points = routine.shape[0] - 2
            routine = routine[:num_points]
            return sum([distance_matrix[routine[i], routine[i + 1]] for i in range(routine.shape[0]-1)])

        case 2:
            # TSP开环+指定起始结束点
            # 起始点-routine-结束点, 如[4, 2, 3, 0, 1, 5]
            num_points = routine.shape[0] - 2
            routine = routine[:num_points]
            routine = np.concatenate([[num_points], routine, [num_points+1]])
            # 每次计算总代价会加入 起始点-中间点 和 中间点-结束点 的信息纳入
            return sum([distance_matrix[routine[i], routine[i + 1]] for i in range(routine.shape[0]-1)])

        case 3:
            # TSP开环+指定起始点(这是我们需要的)
            # 起始点-routine, 如[4, 2, 3, 0, 1]
            num_points = routine.shape[0] - 2
            routine = np.concatenate([[num_points], routine])
            # 每次计算总代价会加入 起始点-中间点 的信息纳入
            return sum([distance_matrix[routine[i], routine[i + 1]] for i in range(routine.shape[0]-1)])

        case 4:
            # TSP开环+指定起始点(使用距离矩阵的方式, 总代价计算类似一般接受距离矩阵的求解器)
            # 要达到的routine, 如[4, 2, 3, 0, 1, 5, 4] ([2, 3, 0, 1]为io路径)
            # 允许的route: 4-io: Dist; 4/io-5: 0; 5-4: 0;
            # 不允许的route: io-4; 5-io
            # 即5只能是5->4, 4只能是4->io, io只能是io->io或io->5
            num_points, = routine.shape
            # 每次计算总代价会加入 起始点-中间点 和 中间点-虚拟点 的信息纳入
            return sum([distance_matrix[routine[i], routine[i + 1]] for i in range(routine.shape[0]-1)])

        case 5:
            # TSP闭环+指定起始点(使用距离矩阵的方式, 总代价计算类似一般接受距离矩阵的求解器)
            # 要达到的routine, 如[4, 2, 3, 0, 1, 4] ([2, 3, 0, 1]为io路径)
            # 允许的route: 4-io: Dist;
            # 不允许的route: io-4;
            # 即4只能是4->io, io只能是io->io
            num_points, = routine.shape
            num_points -= 1
            # 每次计算总代价会加入 起始点-中间点 的信息纳入
            return sum([distance_matrix[routine[i], routine[i + 1]] for i in range(routine.shape[0]-1)])

# %%
best_x, best_y = 0, 0
best_points, best_distance = 0, 0
METHOD = 2
match METHOD:
    case 0:
        from sko.GA import GA_TSP

        ga_tsp = GA_TSP(func=cal_total_distance, n_dim=mat_size, size_pop=200, max_iter=2000, prob_mut=0.7)
        best_points, best_distance = ga_tsp.run()
        best_x, best_y = ga_tsp.generation_best_X, ga_tsp.generation_best_Y

    case 1:
        from sko.ACA import ACA_TSP

        aca = ACA_TSP(func=cal_total_distance, n_dim=mat_size, size_pop=50, max_iter=200, distance_matrix=distance_matrix)
        best_points, best_distance = aca.run()
        best_x, best_y = aca.x_best_history, aca.y_best_history

    case 2:
        from sko.SA import SA_TSP

        sa_tsp = SA_TSP(func=cal_total_distance, x0=range(mat_size), T_max=100, T_min=1, L=15)
        best_points, best_distance = sa_tsp.run()
        best_x, best_y = sa_tsp.best_x_history, sa_tsp.best_y_history

# %%
best_points_coordinate = points_coordinate[best_points, :]
# %%
# fig, ax = plt.subplots(1, 2)
# # %%
# ax[0].plot(best_points_coordinate[:, 0], best_points_coordinate[:, 1], 'o-r')
# ax[1].plot(ga_tsp.generation_best_Y)
# plt.show()

# %%
plt.plot(best_points_coordinate[:-1, 0], best_points_coordinate[:-1, 1], 'o-r')
plt.show()

# %%
print(best_y[-1])
plt.plot(best_y)
plt.show()

# %%