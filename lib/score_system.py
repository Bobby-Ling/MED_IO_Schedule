# %%
import sys
import time
import psutil
import os
from libseek_model_wrapper import IO_Schedule
from pathlib import Path
# 添加上一级目录到 sys.path
sys.path.append(str(Path(__file__).resolve().parent.parent))
from dataset.dataset_gen import generate_tape_io_sequence

file_dir = Path(__file__).parent.parent / "dataset"
class ScoringSystem:
    def __init__(self, is_final_round=False):

        self.is_final_round = is_final_round
        # 基线读时延
        self.baseline_read_latency = 0
        # 调度算法读时延
        self.sorted_read_latency = 0
        # 基线磨损
        self.baseline_tape_wear = 0
        # 调度算法读磨损
        self.sorted_tape_wear = 0
        # 调度算法执行时间
        self.io_sorting_time = 0
        # 实际使用空间
        self.actual_space_used = 0
        # 错误IO调度
        self.error_io_requests = 0
        # 问题规模
        self.request_size = 0
        # 时间/磨损权重
        self.time_weight = 1
        self.wear_weight = 0

    def set_baseline_metrics(self, read_latency, tape_wear=0):
        self.baseline_read_latency = read_latency
        self.baseline_tape_wear = tape_wear

    def set_sorted_metrics(self, read_latency, tape_wear=0):
        self.sorted_read_latency = read_latency
        self.sorted_tape_wear = tape_wear

    def set_io_sorting_time(self, sorting_time):
        self.io_sorting_time = sorting_time

    def set_actual_space_used(self, space_used):
        self.actual_space_used = space_used

    def set_error_io_requests(self, error_count):
        self.error_io_requests = error_count

    def set_request_size(self, size):
        self.request_size = size

    def set_weights(self, time_weight, wear_weight):
        self.time_weight = time_weight
        self.wear_weight = wear_weight

    def calculate_algorithm_score(self):
        if self.is_final_round:
            time_score = (self.baseline_read_latency - self.sorted_read_latency) * self.time_weight
            wear_score = (self.baseline_tape_wear - self.sorted_tape_wear) * self.wear_weight
            return (time_score + wear_score) * 10
        else:
            return (self.baseline_read_latency - self.sorted_read_latency) * 10

    def calculate_time_bonus(self):
        return max(0, (20*1000 - self.io_sorting_time) * 10)

    def calculate_time_penalty(self):
        if self.io_sorting_time > 20*1000:
            return (self.io_sorting_time - 20*1000) * (self.request_size // 50 + (1 if self.request_size % 50 > 0 else 0))
        return 0

    def calculate_space_penalty(self):
        if self.actual_space_used > 10 * 1024 * 1024:  # 10M in bytes
            excess = self.actual_space_used - 10 * 1024 * 1024
            return excess * (self.request_size // 100 + (1 if self.request_size % 100 > 0 else 0))
        return 0

    def calculate_error_penalty(self):
        return 10 * self.error_io_requests

    def calculate_total_score(self):
        algorithm_score = self.calculate_algorithm_score()
        time_bonus = self.calculate_time_bonus()
        time_penalty = self.calculate_time_penalty()
        space_penalty = self.calculate_space_penalty()
        error_penalty = self.calculate_error_penalty()

        total_score = algorithm_score + time_bonus - time_penalty - space_penalty - error_penalty
        return total_score


# 获取当前进程的内存使用量（以 MB 为单位）
def get_memory_usage_in_MB():
    process = psutil.Process(os.getpid())  # 获取当前进程对象
    memory_info = process.memory_info()    # 获取内存信息
    return memory_info.rss / (1024 * 1024)  # 将内存使用量转为 MB


def run_scoring_system(
    is_final_round=False,
    io: IO_Schedule = None,
    io_count=0,
    method: IO_Schedule.METHOD = IO_Schedule.METHOD.Greedy,
):
    scorer = ScoringSystem(is_final_round)

    # 调用基线算法，求出基线时延
    io.execute(method=IO_Schedule.METHOD.SCAN)
    address_duration_before=io.address_duration()

    # 调用使用的算法
    path, result_str, addr_dur, run_time, mem_use = io.execute(method)
    address_duration_after=io.address_duration()

    # 设置类scorer中的各变量
    scorer.set_baseline_metrics(read_latency=address_duration_before)
    scorer.set_sorted_metrics(read_latency=address_duration_after)
    scorer.set_request_size=io_count
    scorer.set_actual_space_used(mem_use)
    scorer.set_error_io_requests(0)  # 假设没有错误
    scorer.set_io_sorting_time=run_time

    score = scorer.calculate_total_score()
    print(f"io_count={io_count}, 分数: {score}")
    return score


# %%
if __name__ == "__main__":
    METHOD = IO_Schedule.METHOD
    methods = [
        # METHOD.SCAN,
        # METHOD.LKH,
        # METHOD.BASE,
        # METHOD.Greedy,
        # METHOD.Greedy1,
        METHOD.LKH1,
    ]
    for method in methods:
        print(f"--------------------{method.name} 开始-------------------")
        total_score = 0
        # 考虑一般情况，io在前100%，随机分布，长度随机
        io_counts = [10, 50, 100, 1000, 5000, 10000]
        for i in range(len(io_counts)):
            print(f"----------------io_counts 数目: [{io_counts}]------------")
            data_set_file = file_dir / f"case_test_{i}.txt"
            if not os.path.exists(data_set_file):
                generate_tape_io_sequence(
                    io_area=1.0,
                    io_count=io_counts[i],
                    filename=data_set_file,
                )
            test = IO_Schedule(f"{file_dir}/../dataset/case_test_{i}.txt")
            score = run_scoring_system(io=test, io_count=io_counts[i], method=method)
            total_score += score

        # 考虑特殊情况，io为高斯分布，或io全是正向/反向分布

        print("\n\n----------------------------------------------------")
        print(f"{method.name} 算法总分为{total_score}")
        print("----------------------------------------------------\n\n")

# %%
