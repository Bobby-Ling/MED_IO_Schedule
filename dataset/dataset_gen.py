# %%
import sys
from pathlib import Path
import random

file_dir = Path(__file__).parent

# %%
def generate_tape_io_sequence(max_lpos = 730994, max_wrap = 280, io_count:int = random.randint(10, 10000), filename = ''):
    """可以指定io_count等参数, 否则随机

    Args:
        max_lpos (int, optional): Defaults to 730994.
        max_wrap (int, optional): Defaults to 280.
        io_count (int, optional): Defaults to random.randint(10, 10000).
        filename (str, optional): Defaults to '' 不为空时写入文件.

    Returns:
        _type_: sequence对象
    """
    # 随机生成磁头位置
    head_info = {
        "head": random.randint(0, max_lpos),
        "wrap": random.randint(0, max_wrap - 1),
        "lpos": random.randint(0, max_lpos),
        "status": random.randint(0, 1)  # 假设 status 是 0 或 1
    }

    io_vector = []
    for i in range(io_count):
        wrap = random.randint(0, max_wrap - 1)

        # 根据 wrap 的奇偶性生成 startLpos 和 endLpos
        if wrap % 2 == 0:  # wrap 为偶数
            start_lpos = random.randint(0, max_lpos - 1)
            end_lpos = random.randint(start_lpos + 1, max_lpos)  # 确保 start_lpos < end_lpos
        else:  # wrap 为奇数
            start_lpos = random.randint(0, max_lpos - 1)
            end_lpos = random.randint(0, start_lpos - 1)  # 确保 start_lpos > end_lpos

        io = {
            "io": i + 1,  # I/O 请求的 ID，从 1 开始
            "wrap": wrap,
            "startLpos": start_lpos,
            "endLpos": end_lpos
        }
        io_vector.append(io)

    # 输出结构
    sequence = {
        "head_info": head_info,
        "io_count": io_count,
        "io_vector": io_vector
    }

    if filename != '':
        # 写入到文件
        # 保存磁带 I/O 请求序列到文件
        with open(filename, 'w') as file:
            # 写入头信息，固定输出格式 ["head":"wrap","lpos","status"]
            file.write(f'["head":"wrap","lpos","status"]\n')
            file.write(f'[{sequence["head_info"]["wrap"]},{sequence["head_info"]["lpos"]},{sequence["head_info"]["status"]}]\n')

            # 写入 io count，固定输出格式 ["io count"]
            file.write(f'["io count"]\n')
            file.write(f'[{sequence["io_count"]}]\n')

            # 写入每个 I/O 请求，固定输出格式 ["io":"id","wrap","startLpos","endLpos"]
            file.write(f'["io":"id","wrap","startLpos","endLpos"]\n')
            for io in sequence["io_vector"]:
                file.write(f'[{io["io"]},{io["wrap"]},{io["startLpos"]},{io["endLpos"]}]\n')

        print(f'Data saved to {filename}')

    return sequence

# %%
if __name__ == '__main__':
    # 命令行运行时执行, import导入时不执行
    generate_tape_io_sequence(filename=file_dir / './case_7.txt')

# %%
