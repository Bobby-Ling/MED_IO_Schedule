import json

def parse_case(dataset_file:str, result_file:str, sequence:list[int] =[]):
    # 读取文件内容
    with open(dataset_file, "r") as file:
        lines = file.readlines()
    if sequence == []:
        try:
            with open(result_file, "r") as file:
                sequence = eval(file.read())
        except:
            print("error")
            exit(-1)

    # 去掉每行的空白字符和换行符
    lines = [line.strip() for line in lines]

    # 解析 head 信息
    head_fields = lines[0][1:-1].split(":")  # 第1行，解析 "head":"wrap","lpos","status"
    head_values = [int(x) for x in lines[1][1:-1].split(",")]  # 第2行，解析 [8,1000,0]
    head_info = {
        "wrap": head_values[0],
        "lpos": head_values[1],
        "status": "HEAD_STATIC" if head_values[2] == 0 else "HEAD_RW"
    }

    # 解析 IO count
    io_count = int(lines[3][1:-1])  # 第4行，解析 IO 数量

    # 解析 IO 列表
    io_list = []
    for i in range(5, 5 + io_count):  # 从第6行开始解析IO数据
        io_values = [int(x) for x in lines[i][1:-1].split(",")]
        io_list.append({
            "id": io_values[0],
            "wrap": io_values[1],
            "startLpos": io_values[2],
            "endLpos": io_values[3]
        })

    # 构建 IOVector 和 InputParam
    io_vector = {
        "len": io_count,
        "ioArray": io_list
    }

    input_param = {
        "headInfo": head_info,
        "ioVec": io_vector
    }

    # 构建 OutputParam (根据 IO 数量和顺序)
    output_param = {
        "len": io_count,
        "sequence": sequence
    }

    # 最终的 JSON 结构
    result = {
        "InputParam": input_param,
        "OutputParam": output_param
    }

    # print(json.dumps(result, indent=4))
    return json.dumps(result, indent=4)
