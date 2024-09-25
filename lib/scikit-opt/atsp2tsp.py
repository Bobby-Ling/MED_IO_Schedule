# %%
import numpy as np

def asym2sym(distance_matrix: np.ndarray):
    atsp = distance_matrix + np.diag(np.ones(len(distance_matrix)) * np.iinfo(np.int32).min)
    max_mat = np.ones(shape=atsp.shape, dtype=np.int32) * np.iinfo(np.int32).max
    distance_matrix_sym = np.block([[max_mat, atsp], [atsp, max_mat]])
    return distance_matrix_sym

# %%
if __name__ == '__main__':
    from pathlib import Path
    import sys
    import numpy as np

    # 修正相对路径
    file_dir = Path(__file__).parent
    sys.path.append(str(file_dir / '..'))
    import libseek_model_wrapper as lsm

    dataset_file = str(file_dir / "../../dataset/case_1.txt")
    input_param = lsm.InputParam()
    input_param.from_case_file(dataset_file)

    num_points = input_param.ioVec.len
    distance_matrix = np.array(lsm.get_dist_matrix(input_param))
    # distance_matrix = distance_matrix[:3, :3]
    distance_matrix_sym = asym2sym(distance_matrix)
    # %%
