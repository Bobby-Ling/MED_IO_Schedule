# %%
import sys
from pathlib import Path
import random

file_dir = Path(__file__).parent


# %%
def generate_tape_io_sequence(
    max_lpos=730994,
    max_wrap=280,
    io_count=None,
    io_length=None,
    io_area=1.0,
    io_distribution=None,
    io_edge=None,
    filename="",
):
    """可以指定io_count等参数, 否则随机

    Args:
        max_lpos (int, optional): Defaults to 730994.
        max_wrap (int, optional): Defaults to 280.
        io_count (int, optional): io_count (int, optional): Number of I/O requests. If None, randomly chosen from [10, 50, 100, 1000, 2000, 5000, 10000].
        io_length (int, optional): Fixed length for each I/O. If None, random length between 30 and 1500.
        io_area (float, optional): Portion of the tape to use (0.0-1.0). Defaults to 1.0.
        io_distribution (str, optional): Distribution of I/Os. Can be None (random) or 'GAUSS'.
        io_edge (str, optional): Edge case for I/O direction. Can be None, 'FORWARD', or 'REVERSE'.
        filename (str, optional): Defaults to '' 不为空时写入文件.

    Returns:
        _type_: sequence对象
    """
    # Set io_count if not specified
    if io_count is None:
        io_count = random.choice([10, 50, 100, 1000, 2000, 5000, 10000])

    effective_max_lpos = int(max_lpos * io_area)

    # 随机生成磁头位置
    head_info = {
        "head": random.randint(0, max_lpos),
        "wrap": random.randint(0, max_wrap - 1),
        "lpos": random.randint(0, max_lpos),
        "status": random.randint(0, 1)  # 假设 status 是 0 或 1
    }

    io_vector = []
    for i in range(io_count):
        if io_edge == 'FORWARD':
            wrap = random.randint(0, max_wrap // 2 - 1) * 2  # Even wrap
        elif io_edge == 'REVERSE':
            wrap = random.randint(0, max_wrap // 2 - 1) * 2 + 1  # Odd wrap
        else:
            wrap = random.randint(0, max_wrap - 1)

        if io_length is None:
            length = random.randint(30, 1500)
        else:
            length = io_length

        if io_distribution == 'GAUSS':
            mean = effective_max_lpos / 2
            std_dev = effective_max_lpos / 6
            start_lpos = int(random.gauss(mean, std_dev))
            start_lpos = max(0, min(start_lpos, effective_max_lpos - length))
        else:
            start_lpos = random.randint(0, effective_max_lpos - length)

        # Determine direction based on wrap
        if wrap % 2 == 0:  # Even wrap: BOT->EOT
            end_lpos = start_lpos + length
        else:  # Odd wrap: EOT->BOT
            end_lpos = start_lpos
            start_lpos += length

        io = {
            "io": i + 1,
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
    generate_tape_io_sequence(io_area=1.0,io_count=10000,io_length=1000,filename=file_dir / './case_8.txt')

# %%
