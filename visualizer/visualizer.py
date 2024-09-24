# %%
from pathlib import Path
import sys
import os

# %%
from dataset_parser import parse_case
from generate_tracing_json import sln_json_to_profiling_json
from webui import start_server

file_dir = Path(__file__).parent

# %%
if len(sys.argv) != 4:
    print("usage: python visualizer.py case_1.txt result.txt profiling.json")
    exit(-1)

dataset_file = sys.argv[1]
result_file = sys.argv[2]
profiling_json_file = sys.argv[3]

sln_json = parse_case(dataset_file, result_file)
profiling_json = sln_json_to_profiling_json(sln_json)
# print(profiling_json)
with open(profiling_json_file, "w") as profiling_file:
    profiling_file.write(profiling_json)

# %%
# start_server()