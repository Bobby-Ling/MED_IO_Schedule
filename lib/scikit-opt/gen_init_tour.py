# %%
def save_initial_tour(path_idx: list[int], filename: str):
    """
    Saves a list of integers as a tour in the specified file. The tour is terminated by -1.

    Parameters:
    path (list[int]): The list of node integers representing the tour.
    filename (str): The name of the file where the tour will be saved.
    """
    try:
        # Open the file in write mode
        with open(filename, 'w') as file:
            file.write('TOUR_SECTION\n')
            # Write each node in the path to the file
            for node in path_idx:
                file.write(f"{node}\n")
            # Write the terminating -1
            file.write("-1\n")
        print(f"Tour successfully saved to {filename}")
    except Exception as e:
        print(f"An error occurred: {e}")
