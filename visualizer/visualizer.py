import sys
import os

from dataset_parser import parse_case
from generate_tracing_json import sln_json_to_profiling_json


script_path = os.path.abspath(__file__)
current_working_directory = os.getcwd()
relative_path = os.path.relpath(script_path, current_working_directory)
relative_path_prefix = os.path.dirname(relative_path)

if len(sys.argv) != 4:
    print("usage: python visualizer.py case_1.txt result.txt profiling.json")
    exit(-1)

dataset_file = sys.argv[1]
result_file = sys.argv[2]
profiling_json_file = sys.argv[3]

sln_json = parse_case(dataset_file, result_file)
profiling_json = sln_json_to_profiling_json(sln_json)
print(profiling_json)
with open(profiling_json_file, "w") as profiling_file:
    profiling_file.write(profiling_json)


