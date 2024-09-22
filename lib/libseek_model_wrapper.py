import ctypes
from ctypes import *

# 加载共享库
# ctypes.RTLD_GLOBAL 在加载 libproject_hw_dl.so 时, 可以访问之前加载的其他库中的符号
lib = ctypes.CDLL('./libseek_model.so', mode=ctypes.RTLD_GLOBAL)

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
    _fields_ = [
        ("wrap", c_uint32),
        ("lpos", c_uint32),
        ("status", c_int)
    ]

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
        ("endLpos", c_uint32)
    ]

    def __init__(self, id: int = 0, wrap: int = 0, startLpos: int = 0, endLpos: int = 0):
        super().__init__()
        self.id = id
        self.wrap = wrap
        self.startLpos = startLpos
        self.endLpos = endLpos

class IOVector(Structure):
    _fields_ = [
        ("len", c_uint32),
        ("ioArray", POINTER(IOUint))
    ]

    def __init__(self, len: int = 0, ioArray: POINTER(IOUint) = None):
        super().__init__()
        self.len = len
        self.ioArray:list[IOUint] = ioArray or (IOUint * len)()

class InputParam(Structure):
    _fields_ = [
        ("headInfo", HeadInfo),
        ("ioVec", IOVector)
    ]

    def __init__(self, headInfo: HeadInfo = None, ioVec: IOVector = None):
        super().__init__()
        self.headInfo:HeadInfo = headInfo or HeadInfo()
        self.ioVec:IOVector = ioVec or IOVector()

class OutputParam(Structure):
    _fields_ = [
        ("len", c_uint32),
        ("sequence", POINTER(c_uint32))
    ]

    # 不带参数则分配空间
    def __init__(self, len: int = 0, sequence: POINTER(c_uint32) = None):
        super().__init__()
        self.len = len
        self.sequence = sequence or (c_uint32 * len)()

class TapeBeltSegWearInfo(Structure):
    _fields_ = [
        ("segWear", c_uint16 * MAX_LPOS)
    ]

    def __init__(self, segWear=None):
        super().__init__()
        if segWear is not None:
            self.segWear = segWear
        else:
            self.segWear = (c_uint16 * MAX_LPOS)()

class AccessTime(Structure):
    _fields_ = [
        ("addressDuration", c_uint32),
        ("readDuration", c_uint32)
    ]

    def __init__(self, addressDuration: int = 0, readDuration: int = 0):
        super().__init__()
        self.addressDuration = addressDuration
        self.readDuration = readDuration


# 定义C函数的参数和返回类型

lib.SeekTimeCalculate.argtypes = [POINTER(HeadInfo), POINTER(HeadInfo)]
lib.SeekTimeCalculate.restype = c_uint32

lib.BeltWearTimes.argtypes = [POINTER(HeadInfo), POINTER(HeadInfo), POINTER(TapeBeltSegWearInfo)]
lib.BeltWearTimes.restype = c_uint32

lib.MotorWearTimes.argtypes = [POINTER(HeadInfo), POINTER(HeadInfo)]
lib.MotorWearTimes.restype = c_uint32

lib.ReadTimeCalculate.argtypes = [c_uint32]
lib.ReadTimeCalculate.restype = c_uint32

lib.TotalAccessTime.argtypes = [POINTER(InputParam), POINTER(OutputParam), POINTER(AccessTime)]
lib.TotalAccessTime.restype = None

lib.TotalTapeBeltWearTimes.argtypes = [POINTER(InputParam), POINTER(OutputParam), POINTER(TapeBeltSegWearInfo)]
lib.TotalTapeBeltWearTimes.restype = c_uint32

lib.TotalMotorWearTimes.argtypes = [POINTER(InputParam), POINTER(OutputParam)]
lib.TotalMotorWearTimes.restype = c_uint32

# 使用类型注解添加函数封装

import os
from typing import Union

def seek_time_calculate(start: HeadInfo, target: HeadInfo) -> int:
    """计算寻址时间, 单位毫秒"""
    return lib.SeekTimeCalculate(byref(start), byref(target))

def belt_wear_times(start: HeadInfo, target: HeadInfo, seg_wear_info: TapeBeltSegWearInfo) -> int:
    """计算带体磨损次数"""
    return lib.BeltWearTimes(byref(start), byref(target), byref(seg_wear_info))

def motor_wear_times(start: HeadInfo, target: HeadInfo) -> int:
    """计算电机磨损次数"""
    return lib.MotorWearTimes(byref(start), byref(target))

def read_time_calculate(lpos_range: int) -> int:
    """计算读IO数据的时间, 单位毫秒"""
    return lib.ReadTimeCalculate(lpos_range)

def total_access_time(input_param: InputParam, output_param: OutputParam, access_time: AccessTime) -> None:
    """计算批量IO的总访问时间(寻址时间 + 读IO时间)"""
    lib.TotalAccessTime(byref(input_param), byref(output_param), byref(access_time))

def total_tape_belt_wear_times(input_param: InputParam, output_param: OutputParam, seg_wear_info: TapeBeltSegWearInfo) -> int:
    """统计带体磨损次数"""
    return lib.TotalTapeBeltWearTimes(byref(input_param), byref(output_param), byref(seg_wear_info))

def total_motor_wear_times(input_param: InputParam, output_param: OutputParam) -> int:
    """统计电机磨损次数"""
    return lib.TotalMotorWearTimes(byref(input_param), byref(output_param))

lib_main = ctypes.CDLL('./libproject_hw_dl.so')

# 定义包装函数
def parse_file(filename: str, head_info: HeadInfo, io_vector: IOVector) -> int:
    """调用 C 库中的 parseFile 函数，并传入文件名、HeadInfo 和 IOVector"""
    filename_bytes = filename.encode('utf-8')  # 将文件名转换为字节字符串
    result = lib_main.parseFile(c_char_p(filename_bytes), byref(head_info), byref(io_vector))
    return result

# 定义Context结构体
class Context(Structure):
    _fields_ = [
        ("input", POINTER(InputParam))  # 定义结构体字段
    ]

    def __init__(self, input_param: InputParam):
        # 调用父类的初始化方法
        super().__init__()

        # 将传入的 InputParam 实例的指针赋值给 input 字段
        self.input = pointer(input_param)

# 设置getNodeDist函数参数类型和返回类型
lib_main.getNodeDist.argtypes = [c_uint32, c_uint32, POINTER(Context)]
lib_main.getNodeDist.restype = c_uint32

# 包装getNodeDist函数
def get_node_dist(idx_from: int, idx_to: int, ctx: Context) -> int:
    """调用 C 库中的 getNodeDist 函数"""
    result = lib_main.getNodeDist(c_uint32(idx_from), c_uint32(idx_to), byref(ctx))
    return result

# 设置getDistMatrix函数参数类型和返回类型
lib_main.getDistMatrix.argtypes = [POINTER(InputParam), POINTER(c_int)]
lib_main.getDistMatrix.restype = None  # void 函数无返回值

# 包装getDistMatrix函数
def get_dist_matrix(input_param: InputParam, matrix_2d: ctypes.Array) -> None:
    """调用 C 库中的 getDistMatrix 函数"""
    lib_main.getDistMatrix(byref(input_param), matrix_2d)


# os.chir("")

# 使用示例
def address_duration(dataset_file:str, path:list[int]):
    head_info = HeadInfo()
    io_vector = IOVector()

    result = parse_file(dataset_file, head_info, io_vector)
    input_param = InputParam(head_info, io_vector)
    if result == 0:
        print(f"HeadInfo: wrap={head_info.wrap}, lpos={head_info.lpos}, status={head_info.status}")
        print(f"IOVector length: {io_vector.len}")
        # for i in range(io_vector.len):
            # print(f"IOVector: {io_vector.ioArray[i].id} {io_vector.ioArray[i].wrap} {io_vector.ioArray[i].startLpos} {io_vector.ioArray[i].endLpos}")
    else:
        print("Error parsing file.")

    output_param = OutputParam(input_param.ioVec.len)
    for i, id in enumerate(path):
        output_param.sequence[i] = id

    access_time = AccessTime()
    total_access_time(input_param, output_param, access_time)
    print(f"addressDuration: {access_time.addressDuration}")

# e.g.
# judgaddress_duratione("./dataset/case_5.txt", [72, 60, 80, 37, 36, 44, 23, 66, 64, 10, 12, 33, 35, 70, 2, 68, 14, 30, 15, 79, 56, 5, 6, 38, 40, 55, 17, 8, 52, 58, 13, 41, 59, 82, 21, 51, 69, 71, 49, 62, 26, 39, 89, 54, 20, 73, 34, 27, 32, 83, 86, 67, 63, 90, 57, 75, 53, 88, 11, 25, 85, 87, 45, 31, 1, 61, 19, 46, 9, 76, 7, 22, 3, 77, 78, 81, 43, 74, 84, 47, 50, 48, 29, 24, 16, 18, 28, 65, 4, 42])