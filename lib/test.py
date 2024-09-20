from libseek_model_wrapper import *

start  = HeadInfo(wrap=0, lpos=100, status=1)
target = HeadInfo(wrap=2, lpos=110, status=0)
seg_wear_info = TapeBeltSegWearInfo()

print(seek_time_calculate(start, target))
print(belt_wear_times(start, target, seg_wear_info))
