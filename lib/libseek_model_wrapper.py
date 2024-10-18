# %%
import time
import ctypes
from ctypes import *
import os
from pathlib import Path
import time
import numpy as np

# 对libseek_model.so的包装

# %%
# 修正相对路径
file_dir = Path(__file__).parent
# 加载共享库
# ctypes.RTLD_GLOBAL 在加载 libproject_hw_dl.so 时, 可以访问之前加载的其他库中的符号
lib = ctypes.CDLL(file_dir / "libseek_model.so", mode=ctypes.RTLD_GLOBAL)

# %%
# 最大wrap
MAX_WRAP = 280
# 最大LPOS
MAX_LPOS = 730994
# 最大IO数量
MAX_IO_NUM = 10000
# 最小IO数量
MIN_IO_NUM = 10

# 定义C结构体


class HeadInfo(Structure):
    _fields_ = [("wrap", c_uint32), ("lpos", c_uint32), ("status", c_int)]

    def __init__(self, wrap: int = 0, lpos: int = 0, status: int = 0):
        super().__init__()
        self.wrap = wrap
        self.lpos = lpos
        self.status = status


class IOUint(Structure):
    _fields_ = [
        ("id", c_uint32),
        ("wrap", c_uint32),
        ("startLpos", c_uint32),
        ("endLpos", c_uint32),
    ]

    def __init__(
        self, id: int = 0, wrap: int = 0, startLpos: int = 0, endLpos: int = 0
    ):
        super().__init__()
        self.id = id
        self.wrap = wrap
        self.startLpos = startLpos
        self.endLpos = endLpos


class IOVector(Structure):
    _fields_ = [("len", c_uint32), ("ioArray", POINTER(IOUint))]

    def __init__(self, len: int = 0, ioArray: POINTER(IOUint) = None):
        super().__init__()
        self.len = len
        self.ioArray: list[IOUint] = ioArray or (IOUint * len)()

    def to_list(self) -> list[int]:
        return [self.ioArray[i].id for i in range(self.len)]


class InputParam(Structure):
    _fields_ = [("headInfo", HeadInfo), ("ioVec", IOVector)]

    def __init__(self, headInfo: HeadInfo = None, ioVec: IOVector = None):
        super().__init__()
        self.headInfo: HeadInfo = headInfo or HeadInfo()
        self.ioVec: IOVector = ioVec or IOVector()

    # 从样例文件初始化
    def from_case_file(self, case_file: str):
        result = parse_file(case_file, self.headInfo, self.ioVec)
        if result != 0:
            raise Exception("parse error")

    # 打印信息
    def print_info(self):
        print(
            f"HeadInfo: wrap={self.headInfo.wrap}, lpos={self.headInfo.lpos}, status={self.headInfo.status}"
        )
        print(f"IOVector length: {self.ioVec.len}")
        for i in range(self.ioVec.len):
            print(
                f"IOVector: {self.ioVec.ioArray[i].id} {self.ioVec.ioArray[i].wrap} {self.ioVec.ioArray[i].startLpos} {self.ioVec.ioArray[i].endLpos}"
            )


class OutputParam(Structure):
    _fields_ = [("len", c_uint32), ("sequence", POINTER(c_uint32))]

    # 不带参数则分配空间
    def __init__(self, len: int = 0, sequence: POINTER(c_uint32) = None):
        super().__init__()
        self.len = len
        self.sequence = sequence or (c_uint32 * len)()

    # 从list初始化
    def from_list(self, path_list: list[int]):
        if len(path_list) == self.len:
            for i, id in enumerate(path_list):
                self.sequence[i] = id
        else:
            raise Exception(f"length error: {self.len} != len({path_list})")

    def to_list(self) -> list[int]:
        return [self.sequence[i] for i in range(self.len)]


class TapeBeltSegWearInfo(Structure):
    _fields_ = [("segWear", c_uint16 * MAX_LPOS)]

    def __init__(self, segWear=None):
        super().__init__()
        if segWear is not None:
            self.segWear = segWear
        else:
            self.segWear = (c_uint16 * MAX_LPOS)()


class AccessTime(Structure):
    _fields_ = [("addressDuration", c_uint32), ("readDuration", c_uint32)]

    def __init__(self, addressDuration: int = 0, readDuration: int = 0):
        super().__init__()
        self.addressDuration = addressDuration
        self.readDuration = readDuration


# uint32_t SeekTimeCalculate(const HeadInfo *start, const HeadInfo *target);
lib.SeekTimeCalculate.argtypes = [POINTER(HeadInfo), POINTER(HeadInfo)]
lib.SeekTimeCalculate.restype = c_uint32


def seek_time_calculate(start: HeadInfo, target: HeadInfo) -> int:
    """计算寻址时间, 单位毫秒"""
    return lib.SeekTimeCalculate(byref(start), byref(target))


# uint32_t BeltWearTimes(const HeadInfo *start, const HeadInfo *target, TapeBeltSegWearInfo *segWearInfo);
lib.BeltWearTimes.argtypes = [
    POINTER(HeadInfo),
    POINTER(HeadInfo),
    POINTER(TapeBeltSegWearInfo),
]
lib.BeltWearTimes.restype = c_uint32


def belt_wear_times(
    start: HeadInfo, target: HeadInfo, seg_wear_info: TapeBeltSegWearInfo
) -> int:
    """计算带体磨损次数"""
    return lib.BeltWearTimes(byref(start), byref(target), byref(seg_wear_info))


# uint32_t MotorWearTimes(const HeadInfo *start, const HeadInfo *target);
lib.MotorWearTimes.argtypes = [POINTER(HeadInfo), POINTER(HeadInfo)]
lib.MotorWearTimes.restype = c_uint32


def motor_wear_times(start: HeadInfo, target: HeadInfo) -> int:
    """计算电机磨损次数"""
    return lib.MotorWearTimes(byref(start), byref(target))


# uint32_t ReadTimeCalculate(uint32_t lposRange);
lib.ReadTimeCalculate.argtypes = [c_uint32]
lib.ReadTimeCalculate.restype = c_uint32


def read_time_calculate(lpos_range: int) -> int:
    """计算读IO数据的时间, 单位毫秒"""
    return lib.ReadTimeCalculate(lpos_range)


# void TotalAccessTime(const InputParam *input, const OutputParam *output, AccessTime *accessTime);
lib.TotalAccessTime.argtypes = [
    POINTER(InputParam),
    POINTER(OutputParam),
    POINTER(AccessTime),
]
lib.TotalAccessTime.restype = None


def total_access_time(
    input_param: InputParam, output_param: OutputParam, access_time: AccessTime
) -> None:
    """计算批量IO的总访问时间(寻址时间 + 读IO时间)"""
    lib.TotalAccessTime(byref(input_param), byref(output_param), byref(access_time))


# uint32_t TotalTapeBeltWearTimes(const InputParam *input, const OutputParam *output, TapeBeltSegWearInfo *segWearInfo);
lib.TotalTapeBeltWearTimes.argtypes = [
    POINTER(InputParam),
    POINTER(OutputParam),
    POINTER(TapeBeltSegWearInfo),
]
lib.TotalTapeBeltWearTimes.restype = c_uint32


def total_tape_belt_wear_times(
    input_param: InputParam,
    output_param: OutputParam,
    seg_wear_info: TapeBeltSegWearInfo,
) -> int:
    """统计带体磨损次数"""
    return lib.TotalTapeBeltWearTimes(
        byref(input_param), byref(output_param), byref(seg_wear_info)
    )


# uint32_t TotalMotorWearTimes(const InputParam *input, const OutputParam *output);
lib.TotalMotorWearTimes.argtypes = [POINTER(InputParam), POINTER(OutputParam)]
lib.TotalMotorWearTimes.restype = c_uint32


def total_motor_wear_times(input_param: InputParam, output_param: OutputParam) -> int:
    """统计电机磨损次数"""
    return lib.TotalMotorWearTimes(byref(input_param), byref(output_param))


# 对其余函数的包装

lib_main = ctypes.CDLL(file_dir / "../build/libproject_hw_dl.so")


# C结构体
class Context(Structure):
    _fields_ = [("input", POINTER(InputParam))]  # 定义结构体字段

    def __init__(self, input_param: InputParam):
        super().__init__()

        # 将传入的 InputParam 实例的指针赋值给 input 字段
        self.input = pointer(input_param)


# int parseFile(const char *filename, HeadInfo *headInfo, IOVector *ioVector);
lib_main.parseFile.argtypes = [c_char_p, POINTER(HeadInfo), POINTER(IOVector)]
lib_main.parseFile.restype = c_uint32


def parse_file(filename: str, head_info: HeadInfo, io_vector: IOVector) -> int:
    """调用 C 库中的 parseFile 函数，并传入文件名、HeadInfo 和 IOVector"""
    filename_bytes = filename.encode("utf-8")  # 将文件名转换为字节字符串
    return lib_main.parseFile(
        c_char_p(filename_bytes), byref(head_info), byref(io_vector)
    )


# uint32_t getNodeDist(uint32_t idx_from, uint32_t idx_to, const Context *ctx);
lib_main.getNodeDist.argtypes = [c_uint32, c_uint32, POINTER(Context)]
lib_main.getNodeDist.restype = c_uint32


def get_node_dist(idx_from: int, idx_to: int, ctx: Context) -> int:
    """调用 C 库中的 getNodeDist 函数"""
    return lib_main.getNodeDist(c_uint32(idx_from), c_uint32(idx_to), byref(ctx))


# void getDistMatrix(const InputParam *input, int *matrix_2d);
lib_main.getDistMatrix.argtypes = [POINTER(InputParam), POINTER(c_int)]
lib_main.getDistMatrix.restype = None  # void 函数无返回值


def get_dist_matrix(input_param: InputParam) -> list:
    """调用 C 库中的 getDistMatrix 函数"""
    rows = input_param.ioVec.len + 2
    cols = rows
    # 创建一个 (rows * cols) 大小的 ctypes 一维数组来存放二维矩阵
    matrix_1d = (c_int * (rows * cols))()
    # 调用 C 函数，传递 input_param 和 matrix_1d 数组
    lib_main.getDistMatrix(byref(input_param), matrix_1d)
    # 将一维数组转换为二维列表
    matrix_2d = [[matrix_1d[i * cols + j] for j in range(cols)] for i in range(rows)]
    return matrix_2d


# double calculateCost(const HeadInfo *current, const HeadInfo *end);
lib_main.calculateCost.argtypes = [POINTER(HeadInfo), POINTER(HeadInfo)]
lib_main.calculateCost.restype = c_double


def calculate_cost(current: HeadInfo, end: HeadInfo) -> float:
    """调用 C 库中的 calculateCost 函数"""
    return lib_main.calculateCost(byref(current), byref(end))


# int32_t IOScheduleAlgorithm(const InputParam *input, OutputParam *output, int METHOD);
lib_main.IOScheduleAlgorithm.argtypes = [
    POINTER(InputParam),
    POINTER(OutputParam),
    c_int,
]
lib_main.IOScheduleAlgorithm.restype = c_int


def io_schedule_algorithm(
    input_param: InputParam, output_param: OutputParam, method: int
) -> int:
    """调用 C 库中的 IOScheduleAlgorithm 函数"""
    return lib_main.IOScheduleAlgorithm(byref(input_param), byref(output_param), method)

lib_main.getAlgorithmRunningDuration.restype = c_double


# double getAlgorithmRunningDuration();
def get_algorithm_running_duration() -> float:
    """调用 C 库中的 getAlgorithmRunningDuration 函数获取IOScheduleAlgorithmX运行时间"""
    return lib_main.getAlgorithmRunningDuration()


class LNS_Param:
    INITIAL_SOLUTIONS: ctypes.c_int = ctypes.c_int.in_dll(lib_main, "INITIAL_SOLUTIONS")
    LNS_ITERATIONS: ctypes.c_int = ctypes.c_int.in_dll(lib_main, "LNS_ITERATIONS")
    SA_INITIAL_TEMP: ctypes.c_int = ctypes.c_int.in_dll(lib_main, "SA_INITIAL_TEMP")
    SA_COOLING_RATE: ctypes.c_float = ctypes.c_float.in_dll(lib_main, "SA_COOLING_RATE")


# %%
import os


def execCmd(cmd):
    r = os.popen(cmd)
    text = r.read()
    r.close()
    return text


# %%
import numpy as np
import matplotlib.pyplot as plt
from typing import Optional, Union
from enum import Enum
import re
from textwrap import dedent


# IO调度问题对象
class IO_Schedule:
    file_dir = Path(__file__).parent

    def __init__(self, dataset_file: str) -> None:
        self.dataset_file = dataset_file
        self.input_param = InputParam()
        self.input_param.from_case_file(dataset_file)
        self.io_coordinates = self.__get_io_coordinates()
        self.dist_matrix = np.array(get_dist_matrix(self.input_param))
        self.output_param = OutputParam(self.input_param.ioVec.len)
        self.path: np.ndarray = None
        pass

    def __get_io_coordinates(self):
        """获取磁头和io的坐标(lpos, wrap)

        Returns:
            _type_: ndarray(n, 2)
        """
        input_param = self.input_param
        X = [
            input_param.ioVec.ioArray[i].startLpos for i in range(input_param.ioVec.len)
        ]
        Y = [input_param.ioVec.ioArray[i].wrap for i in range(input_param.ioVec.len)]
        X = np.concatenate([[input_param.headInfo.lpos], X])
        Y = np.concatenate([[input_param.headInfo.wrap], Y])

        io_coordinates = np.concatenate([X[:, np.newaxis], Y[:, np.newaxis]], axis=1)
        return io_coordinates

    def get_io_coordinates(self):
        """获取磁头和io的坐标(lpos, wrap)

        Returns:
            _type_: ndarray(n, 2)
        """
        return self.io_coordinates

    def get_dist_matrix(self):
        """获取距离矩阵

        Returns:
            _type_: ndarray(n, n)
        """
        return self.dist_matrix

    class METHOD(Enum):
        Greedy = 0
        LKH1 = 1
        LKH = 2
        BASE = 3
        SCAN = 4
        Greedy1 = 5
        LNS = 6
        LNS1 = 7
        LKH_embeded = 8
        Combined = 9

    def run(self, method: METHOD):
        """运行指定算法, 并更新self.path

        Args:
            method (METHOD): 方法enum

        Returns:
            _type_: path列表
        """
        io_schedule_algorithm(self.input_param, self.output_param, method.value)
        self.path = np.array(self.output_param.to_list())
        return self.path

    def execute_cmd(self, method: METHOD):
        """命令行运行指定算法, 并更新self.path

        Args:
            method (METHOD): 方法enum

        Returns:
            _type_: self.path, result_str, addr_dur(ms), algo_dur(ms), mem_use(KB)
        """
        # start = time.time()

        os.chdir(file_dir / "../build/")
        print(f"{method.name}:")
        result_str = execCmd(
            f"METHOD={method.value} ./project_hw -f {self.dataset_file}"
        )
        addr_dur_regex = r"\s*addressingDuration:\s*(\d+)\s*\(ms\)\s*"
        addr_dur = int(re.findall(addr_dur_regex, result_str)[0])
        print(f"{method.name} addressDuration: {addr_dur} ms")

        algo_dur_regex = r"\s*algorithmRunningDuration:\s*(\d+.?\d+)\s*\(ms\)\s*"
        algo_dur = float(re.findall(algo_dur_regex, result_str)[0])
        print(f"{method.name} algorithmRunningDuration: {algo_dur} ms")

        mem_use_regex = r"\s*memoryUse:\s*(\d+.?\d+)\s*\(KB\)\s*"
        mem_use = float(re.findall(mem_use_regex, result_str)[0])
        print(f"{method.name} memoryUse: {mem_use} KB")

        with open(self.dataset_file + ".result") as result_file:
            self.path = np.asarray(eval(result_file.read()))
        os.chdir(file_dir)

        # end = time.time()
        # run_time=round((end - start),2)
        # print(f"run time: {end - start:.2f}s")

        return self.path, result_str, addr_dur, algo_dur, mem_use

    def execute_ffi(self, method: METHOD):
        """使用ffi运行指定算法, 并更新self.path

        Args:
            method (METHOD): 方法enum

        Returns:
            _type_: path列表
        """
        print(f"{method.name}:")
        io_schedule_algorithm(self.input_param, self.output_param, method.value)
        self.path = np.array(self.output_param.to_list())
        addr_dur = self.address_duration()
        algo_dur = get_algorithm_running_duration()
        mem_use = 0.0
        print(f"addressDuration: {addr_dur} ms")
        print(f"algorithmRunningDuration: {algo_dur} ms")
        return self.path, "", addr_dur, algo_dur, mem_use

    def run_LKH(self, matrix: Union[np.ndarray, list[list[int]]], type="ATSP"):
        par_file_name = "par.tmp.o"
        prob_file_name = "prob.tmp.o"
        result_file_name = "result.tmp.o"
        matrix_str = "\n".join([" ".join(map(str, line)) for line in matrix])
        par_file_tmpl = dedent(
            f"""
            SPECIAL
            PROBLEM_FILE = {prob_file_name}
            MTSP_OBJECTIVE = MINSUM
            TRACE_LEVEL = 1
            MAX_CANDIDATES = 6
            MAX_TRIALS = 10000
            RUNS = 1
            OUTPUT_TOUR_FILE = {result_file_name}
            INITIAL_TOUR_ALGORITHM = GREEDY
            """
        )
        prob_file_tmpl = dedent(
            f"""
            NAME : Temp
            TYPE : {type}
            DIMENSION : {len(matrix)}
            VEHICLES : 1
            EDGE_WEIGHT_TYPE: EXPLICIT
            EDGE_WEIGHT_FORMAT: FULL_MATRIX
            EDGE_WEIGHT_SECTION
            {matrix_str}
            """
        )
        os.chdir(file_dir / "./LKH/")
        with open(par_file_name, "w") as par_file, open(
            prob_file_name, "w"
        ) as prob_file:
            par_file.write(par_file_tmpl)
            prob_file.write(prob_file_tmpl)
        result_str = execCmd(f"LKH {prob_file_name}")
        os.chdir(file_dir)

    def address_duration(self, path: Optional[Union[list[int], np.ndarray]] = None):
        """使用官方库获取寻址时间

        Args:
            path (Optional[list[int]], optional): 路径列表, 1-indexed

        Returns:
            _type_: 寻址时间, 单位ms
        """
        if path is None:
            path = self.path
        self.output_param.from_list(path)

        access_time = AccessTime()
        total_access_time(self.input_param, self.output_param, access_time)
        return access_time.addressDuration

    def plot_path(self, path: Optional[Union[list[int], np.ndarray]] = None):
        if path is None:
            path = self.path
        io_coordinates = self.io_coordinates
        # 倒置wrap轴
        plt.gca().invert_yaxis()
        # 添加索引标签
        for idx, point in enumerate(path):
            plt.text(
                io_coordinates[point, 0],
                io_coordinates[point, 1],
                str(idx),
                fontsize=10,
                ha="right",
            )
        for i in range(len(path) - 1):
            random_color = np.random.rand(
                3,
            )
            color = "red" if i == 0 else "blue"
            plt.annotate(
                "",
                xy=io_coordinates[path[i + 1]],
                xytext=io_coordinates[path[i]],
                arrowprops=dict(arrowstyle="->", color=color),
            )
        plt.plot(io_coordinates[path, 0], io_coordinates[path, 1], "o-")
        plt.show()


def get_io_coordinates(dataset_file: str):
    IO_Schedule(dataset_file).get_io_coordinates()


def address_duration(dataset_file: str, path: list[int]):
    return IO_Schedule(dataset_file).address_duration(path)


# %%

# e.g.
# address_duration(str(file_dir / "../dataset/case_5.txt"), [72, 60, 80, 37, 36, 44, 23, 66, 64, 10, 12, 33, 35, 70, 2, 68, 14, 30, 15, 79, 56, 5, 6, 38, 40, 55, 17, 8, 52, 58, 13, 41, 59, 82, 21, 51, 69, 71, 49, 62, 26, 39, 89, 54, 20, 73, 34, 27, 32, 83, 86, 67, 63, 90, 57, 75, 53, 88, 11, 25, 85, 87, 45, 31, 1, 61, 19, 46, 9, 76, 7, 22, 3, 77, 78, 81, 43, 74, 84, 47, 50, 48, 29, 24, 16, 18, 28, 65, 4, 42])

# %%
# input_param = InputParam()
# input_param.from_case_file(str(file_dir / "../dataset/case_5.txt"))
# dist_matrix = get_dist_matrix(input_param)

# %%
if __name__ == '__main__':
    test1 = IO_Schedule(f"{file_dir}/../dataset/case_4.txt")
    test1.execute_ffi(method=IO_Schedule.METHOD.Greedy)
    test1.address_duration()
    test1.plot_path()

    test = IO_Schedule(f"{file_dir}/../dataset/case_8.txt")
    test.execute_ffi(method=IO_Schedule.METHOD.SCAN)
    test.address_duration(path=None)
    test.plot_path(path=None)

# %%
