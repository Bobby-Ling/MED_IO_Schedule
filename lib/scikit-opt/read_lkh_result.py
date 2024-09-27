# %%
def rotate_tour_list(tour_list):
    dimension = len(tour_list)
    # Step 1: Remove the point with value DIMENSION-1
    modified_list = [x for x in tour_list if x != dimension - 1]

    # Step 2: Rotate the list to start with the value DIMENSION
    if dimension in modified_list:
        index = modified_list.index(dimension)
        rotated_list = modified_list[index:] + modified_list[:index]
    else:
        rotated_list = modified_list  # If DIMENSION is not in the list, return as is

    return rotated_list

# %%
def read_tour_file(file_path):
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
    return rotate_tour_list(tour_list)[1:], tour_list