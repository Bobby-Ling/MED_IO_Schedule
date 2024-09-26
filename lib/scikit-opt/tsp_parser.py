# %%
def read_tsp_file(filename: str) -> list:
    with open(filename, 'r') as file:
        lines = file.readlines()

    # 初始化变量
    dimension = 0
    coordinates = []
    reading_coords = False

    for line in lines:
        line = line.strip()

        if line.startswith('DIMENSION'):
            # 读取 DIMENSION 行
            dimension = int(line.split(':')[1].strip())
        elif line.startswith('NODE_COORD_SECTION'):
            # 开始读取节点坐标
            reading_coords = True
        elif line.startswith('EOF'):
            # 结束文件
            break
        elif reading_coords:
            # 读取坐标
            parts = line.split()
            if len(parts) >= 3:
                # 取出 x 和 y 坐标
                x = float(parts[1])
                y = float(parts[2])
                coordinates.append([x, y])

    return coordinates

# %%
if __name__ == '__main__':
    import numpy as np
    from pathlib import Path
    file_dir = Path(__file__).parent
    points_coordinate = np.array(read_tsp_file(str(file_dir / '../ALL_tsp/u159.tsp')))

    from scipy import spatial
    import matplotlib.pyplot as plt

    distance_matrix = spatial.distance.cdist(points_coordinate, points_coordinate, metric='euclidean')

    plt.plot(points_coordinate[:-1, 0], points_coordinate[:-1, 1], 'or')
    plt.show()

# %%
