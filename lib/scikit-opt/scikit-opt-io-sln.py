# %%
import os
from pathlib import Path
import re
import sys
from matplotlib import pyplot as plt
import numpy as np

# from sko.GA import GA_TSP
# 项目主页 https://github.com/guofei9987/scikit-opt
# 文档 https://scikit-opt.github.io/scikit-opt/#/zh/README

# %%
sys.path.append('..')
import libseek_model_wrapper as lsm

# %%
# 本文件用来快速验证算法效果, 可以使用libseek_model_wrapper中封装好的C函数
# 修正相对路径
file_dir = Path(__file__).parent

dataset_file = str(file_dir / "../../dataset/case_1.txt")
input_param = lsm.InputParam()
input_param.from_case_file(dataset_file)

# %%
num_points = input_param.ioVec.len
start_point = [[0,0]]
end_point = [[1,1]]
distance_matrix = np.array(lsm.get_dist_matrix(input_param))

# %%
def cal_total_distance(routine: np.ndarray):
    '''The objective function. input routine, return total distance.
    cal_total_distance(np.arange(num_points))
    '''
    OPTION = 3

    match OPTION:
        case 0:
            # TSP闭环
            # routine-routine[0], 如[2, 3, 0, 1, 2]
            routine = np.concatenate([routine, [routine[0]]])
            return sum([distance_matrix[routine[i], routine[i + 1]] for i in range(routine.shape[0]-1)])

        case 1:
            # TSP开环
            # 路径仅routine, 如[2, 3, 0, 1]
            return sum([distance_matrix[routine[i], routine[i + 1]] for i in range(routine.shape[0]-1)])

        case 2:
            # TSP开环+指定起始结束点
            # 起始点-routine-结束点, 如[4, 2, 3, 0, 1, 5]
            num_points, = routine.shape
            routine = np.concatenate([[num_points], routine, [num_points+1]])
            # 每次计算总代价会加入 起始点-中间点 的信息纳入
            return sum([distance_matrix[routine[i], routine[i + 1]] for i in range(routine.shape[0]-1)])

        case 3:
            # TSP开环+指定起始点(这是我们需要的)
            # 起始点-routine, 如[4, 2, 3, 0, 1]
            num_points, = routine.shape
            routine = np.concatenate([[num_points], routine])
            # 每次计算总代价会加入 起始点-中间点 和 中间点-结束点 的信息纳入
            return sum([distance_matrix[routine[i], routine[i + 1]] for i in range(routine.shape[0]-1)])
# %%
best_x, best_y = 0, 0
best_points, best_distance = 0, 0
METHOD = 1
match METHOD:
    case 0:
        from sko.GA import GA_TSP

        ga_tsp = GA_TSP(func=cal_total_distance, n_dim=num_points, size_pop=50, max_iter=200, prob_mut=0.6)
        best_points, best_distance = ga_tsp.run()
        best_x, best_y = ga_tsp.generation_best_X, ga_tsp.generation_best_Y

    case 1:
        from sko.ACA import ACA_TSP

        aca = ACA_TSP(func=cal_total_distance, n_dim=num_points, size_pop=50, max_iter=200, distance_matrix=distance_matrix[:-2, :-2])
        best_points, best_distance = aca.run()
        best_x, best_y = aca.x_best_history, aca.y_best_history

    case 2:
        from sko.SA import SA_TSP

        sa_tsp = SA_TSP(func=cal_total_distance, x0=range(num_points), T_max=100, T_min=1, L=150)
        best_points, best_distance = sa_tsp.run()
        best_x, best_y = sa_tsp.best_x_history, sa_tsp.best_y_history

# %%
address_duration = lsm.address_duration(dataset_file, [id:=idx+1 for idx in best_points])
print(f"本算法 addressDuration: {address_duration} ms")

# %%
# 目前最优值, 约等于address_duration
print(best_y[-1])
plt.plot(best_y)
plt.show()

# %%
# 执行algorithm.c的算法
import os
import re
def execCmd(cmd):
    r = os.popen(cmd)
    text = r.read()
    r.close()
    return text

os.chdir(file_dir / '../../build/')
print('贪心算法:')
result_greedy = execCmd(f'METHOD=0 ./project_hw -f ../dataset/{Path(dataset_file).name}')
# print(result_greedy)
addr_dur_regex = r'\s*addressingDuration:\s*(\d+)\s*\(ms\)\s*'
print(f"贪心算法 addressDuration: {re.findall(addr_dur_regex, result_greedy)} ms")
print('LKH算法:')
result_LKH = execCmd(f'METHOD=2 ./project_hw -f ../dataset/{Path(dataset_file).name}')
# print(result_LKH)
print(f"LKH算法 addressDuration: {re.findall(addr_dur_regex, result_LKH)} ms")
os.chdir(file_dir)

# %%