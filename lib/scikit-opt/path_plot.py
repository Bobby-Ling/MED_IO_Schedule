# %%
from pathlib import Path
import sys
import numpy as np
from scipy import spatial
import matplotlib.pyplot as plt

# %%
file_dir = Path(__file__).parent
sys.path.append(str(file_dir / '..'))
import libseek_model_wrapper as lsm

dataset_file = str(file_dir / "../../dataset/case_1.txt")
input_param = lsm.InputParam()
input_param.from_case_file(dataset_file)

# %%
X = [input_param.ioVec.ioArray[i].startLpos for i in range(input_param.ioVec.len)]
Y = [input_param.ioVec.ioArray[i].wrap for i in range(input_param.ioVec.len)]
X = np.concatenate([[input_param.headInfo.lpos], X])
Y = np.concatenate([[input_param.headInfo.wrap], Y])
# %%
io_coordinates = np.concatenate([X[:, np.newaxis], Y[:, np.newaxis]], axis=1)
# %%
from read_lkh_result import read_tour_file
path, raw_path = read_tour_file()
path = np.concatenate([[0], path])
raw_path = np.concatenate([[0], raw_path])
raw_path = raw_path[raw_path < len(path)]
# %%
print(path)
plt.gca().invert_yaxis()
for i in range(len(path) - 1):
    random_color = np.random.rand(3,)
    color = 'red' if i == 0 else 'blue'
    plt.annotate('',
                 xy=io_coordinates[path[i + 1]],
                 xytext=io_coordinates[path[i]],
                 arrowprops=dict(arrowstyle='->', color=color))
plt.plot(io_coordinates[path, 0], io_coordinates[path, 1],  'o-')
plt.show()
plt.close()
# %%
print(raw_path)
plt.gca().invert_yaxis()
for i in range(len(path) - 1):
    random_color = np.random.rand(3,)
    color = 'red' if i == 0 else 'blue'
    plt.annotate('',
                 xy=io_coordinates[raw_path[i + 1]],
                 xytext=io_coordinates[raw_path[i]],
                 arrowprops=dict(arrowstyle='->', color=color))
plt.plot(io_coordinates[raw_path, 0], io_coordinates[raw_path, 1],  'o-')
plt.show()
# %%
