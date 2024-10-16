import time
import psutil
from libseek_model_wrapper import IO_Schedule
class ScoringSystem:
    def __init__(self, is_final_round=False):
        self.is_final_round = is_final_round
        self.baseline_read_latency = 0
        self.sorted_read_latency = 0
        self.baseline_tape_wear = 0
        self.sorted_tape_wear = 0
        self.io_sorting_time = 0
        self.actual_space_used = 0
        self.error_io_requests = 0
        self.request_size = 0
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
        return max(0, (20 - self.io_sorting_time) * 10)

    def calculate_time_penalty(self):
        if self.io_sorting_time > 20:
            return (self.io_sorting_time - 20) * (self.request_size // 50 + (1 if self.request_size % 50 > 0 else 0))
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

def run_scoring_system(is_final_round=False):
    scorer = ScoringSystem(is_final_round)

    # 设置基准指标
    scorer.set_baseline_metrics(100, 50 if is_final_round else 0)

    # 模拟算法执行
    start_time = time.time()
    # 这里应该是实际的算法执行
    time.sleep(2)  # 模拟2秒的执行时间
    end_time = time.time()

    # 设置排序后的指标
    scorer.set_sorted_metrics(80, 40 if is_final_round else 0)
    scorer.set_io_sorting_time(end_time - start_time)
    scorer.set_actual_space_used(psutil.Process().memory_info().rss)
    scorer.set_error_io_requests(0)  # 假设没有错误
    scorer.set_request_size(1000)

    total_score = scorer.calculate_total_score()
    print(f"总分: {total_score}")


