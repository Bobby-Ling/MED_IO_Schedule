# %%
from pathlib import Path
import sys
import numpy as np
import matplotlib.pyplot as plt

# %%
def plot_path(path: list[int], io_coordinates: np.ndarray):
    plt.gca().invert_yaxis()
    # 添加索引标签
    for idx, point in enumerate(path):
        plt.text(io_coordinates[point, 0], io_coordinates[point, 1], str(idx), fontsize=12, ha='right')
    for i in range(len(path) - 1):
        random_color = np.random.rand(3,)
        color = 'red' if i == 0 else 'blue'
        plt.annotate('',
                    xy=io_coordinates[path[i + 1]],
                    xytext=io_coordinates[path[i]],
                    arrowprops=dict(arrowstyle='->', color=color))
    plt.plot(io_coordinates[path, 0], io_coordinates[path, 1],  'o-')
    plt.show()

# %%
file_dir = Path(__file__).parent
sys.path.append(str(file_dir / '..'))
import libseek_model_wrapper as lsm

dataset_file = str(file_dir / "../../dataset/case_5.txt")
input_param = lsm.InputParam()
input_param.from_case_file(dataset_file)
distance_matrix = np.array(lsm.get_dist_matrix(input_param))
io_coordinates = lsm.get_io_coordinates(dataset_file)

# %%
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
with open(dataset_file + '.result') as result_file:
    result_greedy_path = eval(result_file.read())
print('LKH算法:')
result_LKH = execCmd(f'METHOD=2 ./project_hw -f ../dataset/{Path(dataset_file).name}')
# print(result_LKH)
print(f"LKH算法 addressDuration: {re.findall(addr_dur_regex, result_LKH)} ms")
with open(dataset_file + '.result') as result_file:
    result_LKH_path = eval(result_file.read())
os.chdir(file_dir)

# %%
from read_result import *
path_no_head, raw_path_no_head = read_tour_file()
path = np.concatenate([[0], path_no_head])
raw_path_no_head = np.array(raw_path_no_head)
raw_path_no_head = raw_path_no_head[raw_path_no_head < len(path)].tolist()
raw_path = np.concatenate([[0], raw_path_no_head])
# %%
# print(path_no_head)
path_cost = sum(distance_matrix[path_no_head[i] - 1, path_no_head[i + 1] - 1] for i in range(len(path_no_head) - 1))
address_duration = lsm.address_duration(dataset_file, path_no_head)
print(f'path_cost: {path_cost}')
print(f'address_duration: {address_duration}')
# plot_path(path, io_coordinates)
# %%
# print(raw_path_no_head)
path_cost = sum(distance_matrix[raw_path_no_head[i] - 1, raw_path_no_head[i + 1] - 1] for i in range(len(raw_path_no_head) - 1))
address_duration = lsm.address_duration(dataset_file, raw_path_no_head)
print(f'path_cost: {path_cost}')
print(f'address_duration: {address_duration}')
# plot_path(raw_path, io_coordinates)
# %%
print(lsm.address_duration(dataset_file, rotate_list(raw_path_no_head, result_greedy_path[0])))
# %%