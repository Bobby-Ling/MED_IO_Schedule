# %%
# from libseek_model_wrapper import *

# start  = HeadInfo(wrap=0, lpos=100, status=1)
# target = HeadInfo(wrap=2, lpos=110, status=0)
# seg_wear_info = TapeBeltSegWearInfo()

# print(seek_time_calculate(start, target))
# print(belt_wear_times(start, target, seg_wear_info))

# %%
from pathlib import Path
from libseek_model_wrapper import IO_Schedule
file_dir = Path(__file__).parent

test1 = IO_Schedule(str(file_dir / "../dataset/case_10.txt"))
test1.run(IO_Schedule.METHOD.Greedy)
test1.address_duration()
test1.plot_path()

# %%
