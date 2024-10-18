# %%
from concurrent.futures import ThreadPoolExecutor, as_completed
import datetime
import json
import sys
import time
import psutil
import os
from libseek_model_wrapper import IO_Schedule, LNS_Param
from pathlib import Path
# 添加上一级目录到 sys.path
sys.path.append(str(Path(__file__).resolve().parent.parent))
from dataset.dataset_gen import generate_tape_io_sequence

file_dir = Path(__file__).parent

class Result:

    def __init__(self):
        self.path = []
        self.result_str = ""
        self.addr_dur = 0
        self.run_time = 0.0
        self.mem_use = 0.0
        self.score = 0.0
        self.algorithm_score = 0.0
        self.time_bonus = 0.0
        self.time_penalty = 0.0
        self.space_penalty = 0.0
        self.error_penalty = 0.0


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
        if self.io_sorting_time > 20 * 1000:
            return (self.io_sorting_time - 20*1000) * (self.request_size // 50 + (1 if self.request_size % 50 > 0 else 0))
        return 0

    def calculate_space_penalty(self):
        if self.actual_space_used > 10 * 1024:  # 10M in bytes
            excess = self.actual_space_used - 10 * 1024
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
        return (
            total_score,
            algorithm_score,
            time_bonus,
            time_penalty,
            space_penalty,
            error_penalty,
        )


def run_scoring_system(
    is_final_round=False,
    io: IO_Schedule = None,
    io_count=0,
    method: IO_Schedule.METHOD = IO_Schedule.METHOD.Greedy,
):
    scorer = ScoringSystem(is_final_round)
    result = Result()

    # 调用基线算法，求出基线时延
    io.execute_ffi(method=IO_Schedule.METHOD.SCAN)
    address_duration_before=io.address_duration()

    # 调用使用的算法
    path, result_str, addr_dur, run_time, mem_use = io.execute_ffi(method)
    print(result_str)
    result.path = path
    result.result_str = result_str
    result.addr_dur = addr_dur
    result.run_time = run_time
    result.mem_use = mem_use

    address_duration_after=io.address_duration()

    # 设置类scorer中的各变量
    scorer.set_baseline_metrics(read_latency=address_duration_before)
    scorer.set_sorted_metrics(read_latency=address_duration_after)
    scorer.set_request_size(io_count)
    scorer.set_actual_space_used(mem_use)
    scorer.set_error_io_requests(0)  # 假设没有错误
    scorer.set_io_sorting_time(run_time)

    score, algorithm_score, time_bonus, time_penalty, space_penalty, error_penalty = (
        scorer.calculate_total_score()
    )
    result.score = score
    result.algorithm_score = algorithm_score
    result.time_bonus = time_bonus
    result.time_penalty = time_penalty
    result.space_penalty = space_penalty
    result.error_penalty = error_penalty

    print(f"io_count={io_count}, 分数: {score}")
    return score, result

import matplotlib.pyplot as plt
import numpy as np


# %%
import matplotlib.pyplot as plt
import pickle
from datetime import datetime


class IO_Schedule_METHOD_JSON(json.JSONEncoder):
    def default(self, obj):  # -> Any | Any:
        if isinstance(obj, IO_Schedule.METHOD):
            return obj.name
        return json.JSONEncoder.default(self, obj)


def visualize_results(
    results: dict[IO_Schedule.METHOD, dict[int, Result]],
    io_counts: list[int],
    plot: bool = True,
):
    metrics = [
        "addr_dur",
        "run_time",
        # "mem_use",   # Uncomment or comment to include/exclude this metric
        "score",
        "algorithm_score",
        "time_bonus",
        "time_penalty",
        # "space_penalty",
        # "error_penalty",
    ]

    # Only keep the active metrics (those that are not commented out)
    active_metrics = [metric for metric in metrics]

    num_metrics = len(active_metrics)
    num_methods = len(results)
    x_values = range(len(io_counts))  # Use indices as x-values

    # Determine the layout of the subplots dynamically based on the number of metrics
    num_rows = (
        num_metrics + 2
    ) // 3  # Ensures the grid is filled efficiently (2-3 columns)
    fig, axes = plt.subplots(num_rows, 3, figsize=(20, 5 * num_rows))
    fig.suptitle("IO Scheduling Results Comparison", fontsize=16)

    # If only one row of axes, ensure `axes` is treated as a 1D array
    axes = axes.flatten() if num_metrics > 1 else [axes]

    # Plot each metric on its own axis
    for idx, metric in enumerate(active_metrics):
        ax = axes[idx]

        for method, result_dict in results.items():
            # Extract the metric values corresponding to each io_count
            values = [
                getattr(result_dict[io_count], metric)
                for io_count in io_counts
                if io_count in result_dict
            ]
            ax.plot(x_values, values, marker="o", label=method.name)

        ax.set_title(f'{metric.replace("_", " ").title()}')
        ax.set_xlabel("IO Count Index")
        ax.set_ylabel("Value")
        ax.set_xticks(x_values)
        ax.set_xticklabels(io_counts)
        ax.legend()
        ax.grid(True)

    # Remove any empty subplots if not all slots in the grid are filled
    if num_metrics < num_rows * 3:
        for empty_ax in axes[num_metrics:]:
            empty_ax.remove()

    plt.tight_layout()

    # Save the figure
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    fig_file_name = f"../docs/result/pics/result_{timestamp}.png"
    plt.savefig(fig_file_name)

    # Save the figure as a pickle file
    with open(f"{fig_file_name}.pickle", "wb") as f:
        pickle.dump(fig, f)

    # Optionally display the plot
    if plot:
        plt.show()

# %%
def judge():
    METHOD = IO_Schedule.METHOD
    methods = [
        # METHOD.BASE,
        # METHOD.SCAN,
        # METHOD.Greedy1,
        METHOD.Greedy,
        # METHOD.LKH,
        # METHOD.LKH1,
        METHOD.LNS,
        METHOD.LNS1,
    ]
    # 考虑一般情况，io在前100%，随机分布，长度随机
    io_counts: list[int] = [10, 50, 100, 1000, 5000, 10000]
    # io_counts = [10, 50, 100, 1000]
    # io_counts = [10, 50]
    results: dict[IO_Schedule.METHOD, dict[int, Result]] = {}
    for method in methods:
        print(f"---------------------{method.name} 开始--------------------")
        total_score = 0
        method_result = {}
        for i, io_count in enumerate(io_counts):
            print(f"-----------------io_counts 数目: [{io_count}]--------------")
            data_set_file = file_dir / f"case_test_{i}.txt"
            if not os.path.exists(data_set_file):
                generate_tape_io_sequence(
                    io_area=1.0,
                    io_count=io_count,
                    filename=data_set_file,
                )
            test = IO_Schedule(f"{file_dir}/../dataset/case_test_{i}.txt")
            score, result = run_scoring_system(
                io=test, io_count=io_count, method=method
            )
            method_result[io_count] = result
            total_score += score
        results[method] = method_result
        # 考虑特殊情况，io为高斯分布，或io全是正向/反向分布

        print("\n\n----------------------------------------------------")
        print(f"{method.name} 算法总分为{total_score}")
        print("----------------------------------------------------\n\n")

    visualize_results(results, io_counts)


def run_single_io(data_set_file: str, io_count: int, method: IO_Schedule.METHOD):
    if not os.path.exists(data_set_file):
        generate_tape_io_sequence(
            io_area=1.0,
            io_count=io_count,
            filename=data_set_file,
        )
    test = IO_Schedule(data_set_file)
    score, result = run_scoring_system(io=test, io_count=io_count, method=method)
    return io_count, result


# %%
def run_io_in_batch(
    io_counts: list[int],
    method: IO_Schedule.METHOD,
    file_dir: str,
    max_workers: int = 1,
    # max_workers: int = os.cpu_count(),
):
    """
    使用线程池并行执行 `run_single_io` 函数。
    """
    method_result = {}
    tasks = []

    with ThreadPoolExecutor(max_workers=max_workers) as executor:
        # 提交所有任务到线程池
        for i, io_count in enumerate(io_counts):
            data_set_file = f"{file_dir}/../dataset/gen/{io_count}.txt"
            future = executor.submit(run_single_io, data_set_file, io_count, method)
            tasks.append(future)

        # 收集结果
        for future in as_completed(tasks):
            io_count, result = future.result()
            method_result[io_count] = result

    return method, method_result


# %%
def run_method_in_batch(methods: list[IO_Schedule.METHOD], io_counts: list[int]):
    results: dict[IO_Schedule.METHOD, dict[int, Result]] = {}
    tasks = []

    # with ThreadPoolExecutor(max_workers=len(methods)) as executor:
    with ThreadPoolExecutor(max_workers=1) as executor:
        # 提交所有任务到线程池
        for method in methods:
            future = executor.submit(run_io_in_batch, io_counts, method, file_dir)
            tasks.append(future)

        # 收集结果
        for future in as_completed(tasks):
            method, result_list = future.result()
            results[method] = result_list

    return results


# %%
def run_test():
    METHOD = IO_Schedule.METHOD
    methods = [
        # METHOD.BASE,
        METHOD.SCAN,
        # METHOD.Greedy1,
        METHOD.Greedy,
        # METHOD.LKH,
        # METHOD.LKH1,
        METHOD.LNS,
        METHOD.LKH_embeded,
        METHOD.Combined,
    ]
    # 考虑一般情况，io在前100%，随机分布，长度随机
    io_counts: list[int] = np.concatenate(
        [
            # np.arange(10, 1000, 5),
            np.arange(1000, 2000, 100),
            np.arange(2000, 5000, 500),
            np.arange(5000, 10001, 1000),
        ]
    ).tolist()
    # io_counts: list[int] = [10, 50, 100, 1000, 5000, 10000]
    results = run_method_in_batch(methods, io_counts)
    for method, method_result in results.items():
        print("\n----------------------------------------------------")
        print(
            f"{method.name} 算法总分为{sum(io_result.score for io_result in method_result.values())}"
        )
        print("----------------------------------------------------\n")

    visualize_results(results, io_counts)


# %%
import itertools


class LNSParamOptimizer:
    def __init__(self):
        self.param_ranges: dict[str, list[int]] = {
            LNS_Param.get_sym_name(LNS_Param.INITIAL_SOLUTIONS): list(range(5, 21, 5)),
            LNS_Param.get_sym_name(LNS_Param.LNS_ITERATIONS): list(
                range(100, 1001, 100)
            ),
            LNS_Param.get_sym_name(LNS_Param.SA_INITIAL_TEMP): list(
                range(500, 1501, 100)
            ),
            LNS_Param.get_sym_name(LNS_Param.SA_COOLING_RATE): [0.99, 0.995, 0.999],
        }
        self.optimization_history = []

    def generate_combinations(self):
        keys, values = zip(*self.param_ranges.items())
        for combination in itertools.product(*values):
            yield dict(zip(keys, combination))

    def optimize(self, evaluation_function):
        best_params = None
        best_score = float("-inf")

        for params in self.generate_combinations():
            score = evaluation_function(params)
            self.optimization_history.append((params, score))
            if score > best_score:
                best_score = score
                best_params = params

        return best_params, best_score

    def visualize_optimization(self):
        # 将优化历史按得分排序
        sorted_history = sorted(self.optimization_history, key=lambda x: x[1])

        # 提取参数和得分
        params = [str(h[0]) for h in sorted_history]
        scores = [h[1] for h in sorted_history]

        # 创建图表
        plt.figure(figsize=(12, 6))
        plt.plot(range(len(scores)), scores, marker="o")
        plt.title("LNS Parameter Optimization Progress")
        plt.xlabel("Iteration")
        plt.ylabel("Score")
        plt.xticks(rotation=45, ha="right")
        plt.tight_layout()

        # 显示最佳参数
        best_params, best_score = max(self.optimization_history, key=lambda x: x[1])
        plt.annotate(
            f"Best score: {best_score:.2f}\nParams: {best_params}",
            xy=(len(scores) - 1, best_score),
            xytext=(len(scores) - 1, best_score + 1),
            arrowprops=dict(facecolor="black", shrink=0.05),
        )

        plt.show()

    def visualize_param_impact(self):
        param_scores = {param: [] for param in self.param_ranges.keys()}

        for params, score in self.optimization_history:
            for param, value in params.items():
                param_scores[param].append((value, score))

        fig, axs = plt.subplots(2, 2, figsize=(15, 15))
        fig.suptitle("Parameter Impact on Optimization Score")

        for (param, scores), ax in zip(param_scores.items(), axs.ravel()):
            x = [s[0] for s in scores]
            y = [s[1] for s in scores]
            ax.scatter(x, y)
            ax.set_title(f"{param.name} Impact")
            ax.set_xlabel(param.name)
            ax.set_ylabel("Score")

        plt.tight_layout()
        plt.show()


# 示例使用
def evaluate_lns(params: dict[str, list[int]]):
    METHOD = IO_Schedule.METHOD
    methods = [
        METHOD.LNS,
        # METHOD.Combined,
    ]
    # 考虑一般情况，io在前100%，随机分布，长度随机
    io_counts: list[int] = np.concatenate(
        [
            np.arange(10, 1000, 20),
            # np.arange(1000, 2000, 100),
            # np.arange(2000, 5000, 500),
            # np.arange(5000, 10001, 1000),
        ]
    ).tolist()
    LNS_Param.INITIAL_SOLUTIONS.value = params[
        LNS_Param.get_sym_name(LNS_Param.INITIAL_SOLUTIONS)
    ]
    LNS_Param.LNS_ITERATIONS.value = params[
        LNS_Param.get_sym_name(LNS_Param.LNS_ITERATIONS)
    ]
    LNS_Param.SA_INITIAL_TEMP = params[
        LNS_Param.get_sym_name(LNS_Param.SA_INITIAL_TEMP)
    ]
    LNS_Param.SA_COOLING_RATE.value = params[
        LNS_Param.get_sym_name(LNS_Param.SA_COOLING_RATE)
    ]
    results = run_method_in_batch(methods, io_counts)
    method_result = results[METHOD.LNS]
    method_total_score = sum(io_result.score for io_result in method_result.values())
    return -method_total_score


def opt_LNS():
    # 使用示例
    optimizer = LNSParamOptimizer()
    best_params, best_score = optimizer.optimize(evaluate_lns)
    print(f"Best parameters: {best_params}")
    print(f"Best score: {best_score}")

    # 可视化优化过程
    optimizer.visualize_optimization()

    # 可视化参数影响
    optimizer.visualize_param_impact()


# %%
def test_scorer():
    scorer = ScoringSystem()
    result = Result()

    path, result_str, addr_dur, run_time, mem_use = ([], "", 1, 30 * 1000, 30 * 1000)
    result.path = path
    result.result_str = result_str
    result.addr_dur = addr_dur
    result.run_time = run_time
    result.mem_use = mem_use

    scorer.set_baseline_metrics(read_latency=100)
    scorer.set_sorted_metrics(read_latency=1000)
    scorer.set_request_size(10000)
    scorer.set_actual_space_used(mem_use)
    scorer.set_error_io_requests(0)  # 假设没有错误
    scorer.set_io_sorting_time(run_time)

    score, algorithm_score, time_bonus, time_penalty, space_penalty, error_penalty = (
        scorer.calculate_total_score()
    )
    result.score = score
    result.algorithm_score = algorithm_score
    result.time_bonus = time_bonus
    result.time_penalty = time_penalty
    result.space_penalty = space_penalty
    result.error_penalty = error_penalty
    return result

# %%
if __name__ == "__main__":
    # test_scorer()
    # judge()
    # run_test()
    opt_LNS()
    pass

# %%
