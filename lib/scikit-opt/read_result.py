# %%
def rotate_tour_list(tour_list):
    target = len(tour_list)
    # Step 1: Remove the point with value DIMENSION-1
    modified_list = [x for x in tour_list if x != target - 1]

    # Step 2: Rotate the list to start with the value DIMENSION
    if target in modified_list:
        index = modified_list.index(target)
        rotated_list = modified_list[index:] + modified_list[:index]
    else:
        rotated_list = modified_list  # If DIMENSION is not in the list, return as is

    return rotated_list[1:]

# %%
def rotate_list(tour_list, target):
    modified_list = tour_list

    # Step 2: Rotate the list to start with the value DIMENSION
    if target in modified_list:
        index = modified_list.index(target)
        rotated_list = modified_list[index:] + modified_list[:index]
    else:
        rotated_list = modified_list  # If DIMENSION is not in the list, return as is

    return rotated_list

# %%
from pathlib import Path
def read_tour_file(file_path = Path(__file__).parent / '../LKH/LKH.result'):
    tour_list = []
    with open(file_path, 'r') as file:
        in_tour_section = False
        for line in file:
            line = line.strip()
            if line == "TOUR_SECTION":
                in_tour_section = True
            elif line == "-1":
                break
            elif in_tour_section:
                tour_list.append(int(line))
    return rotate_tour_list(tour_list), tour_list

# %%
if __name__ == '__main__':
    from pathlib import Path
    file_dir = Path(__file__).parent

    path, raw_path = read_tour_file()
    print(path)
    print(raw_path)
# %%
