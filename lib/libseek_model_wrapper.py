import ctypes
from ctypes import *

# 加载共享库
lib = ctypes.CDLL('./libseek_model.so')

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

class IOUint(Structure):
    _fields_ = [
        ("id", c_uint32),
        ("wrap", c_uint32),
        ("startLpos", c_uint32),
        ("endLpos", c_uint32)
    ]

class IOVector(Structure):
    _fields_ = [
        ("len", c_uint32),
        ("ioArray", POINTER(IOUint))
    ]

class InputParam(Structure):
    _fields_ = [
        ("headInfo", HeadInfo),
        ("ioVec", IOVector)
    ]

class OutputParam(Structure):
    _fields_ = [
        ("len", c_uint32),
        ("sequence", POINTER(c_uint32))
    ]

class TapeBeltSegWearInfo(Structure):
    _fields_ = [
        ("segWear", c_uint16 * MAX_LPOS)
    ]

class AccessTime(Structure):
    _fields_ = [
        ("addressDuration", c_uint32),
        ("readDuration", c_uint32)
    ]

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
