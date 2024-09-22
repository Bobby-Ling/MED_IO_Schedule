from dataset_parser import parse_case
import json

def sln_json_to_profiling_json(sln_json: str):
    MAX_WRAP:int = 280
    MAX_LPOS:int = 730994
    SHOW_WRAP_WITH_NO_IO:bool = False

    sln = json.loads(sln_json)

    profiling_data = []

    # 处理磁头信息

    head = sln['InputParam']['headInfo']

    profiling_data += [
        {
            "name": "Head",
            "ph": "B",
            "pid": "ioArray",
            "tid": f"wrap{head['wrap']}",
            "ts": head['lpos']
        },
        {
            "name": "Head",
            "ph": "E",
            "pid": "ioArray",
            "tid": f"wrap{head['wrap']}",
            "ts": head['lpos']+10000
        }
    ]

    # 处理io排布信息
    io_vec_len = sln['InputParam']['ioVec']['len']
    io_vec = sln['InputParam']['ioVec']['ioArray']

    used_wraps:list[int] = [ wrap['wrap'] for wrap in io_vec ]
    for io in io_vec:
        # 一个wrap可以有多个io
        profiling_data += [
            {
                "name": f"id: {io['id']}",
                "ph": "B",
                "pid": "ioArray",
                "tid": f"wrap{io['wrap']}",
                "ts": min(io['startLpos'], io['endLpos'])
            },
            {
                "name": f"id: {io['id']}",
                "ph": "E",
                "pid": "ioArray",
                "tid": f"wrap{io['wrap']}",
                "ts": max(io['startLpos'], io['endLpos'])
            }
        ]
    if SHOW_WRAP_WITH_NO_IO:
        for wrap in range(0, MAX_WRAP+1):
            if wrap not in used_wraps:
                profiling_data += [
                    {
                        "name": f"id: {io['id']}",
                        "ph": "B",
                        "pid": "ioArray",
                        "tid": f"wrap{wrap}",
                        "ts": 0
                    },
                    {
                        "name": f"id: {io['id']}",
                        "ph": "E",
                        "pid": "ioArray",
                        "tid": f"wrap{wrap}",
                        "ts": 0
                    }
                ]
    profiling_data = sorted(profiling_data, key=lambda x: int(x['tid'].replace("wrap", "")))

    # 处理io调度信息(算法结果)

    scheduling = sln['OutputParam']['sequence']

    from_id = "Head"
    to_id = scheduling[0]
    from_io = head
    to_io = io_vec[to_id-1]
    from_end = from_io['lpos']
    to_start = to_io['startLpos']
    # 处理磁头到第一个io
    profiling_data += [
        {
            "name": "connect",
            "ph": "s",
            "id": f"{from_id}->{to_id}",
            "pid": "ioArray",
            # "tid": f"wrap{from_io['wrap']}",
            "tid": f"wrap{from_io['wrap'] if from_end < to_start else to_io['wrap']}",
            # "ts": from_end,
            "ts": min(from_end, to_start)
        },
        {
            "name": "connect",
            "ph": "f",
            "bp": "e",
            "id": f"{from_id}->{to_id}",
            "pid": "ioArray",
            # "tid": f"wrap{to_io['wrap']}",
            "tid": f"wrap{from_io['wrap'] if from_end > to_start else to_io['wrap']}",
            # "ts": to_start
            "ts": max(from_end, to_start)
        }
    ]
    # 这里假定id-1==io_vec的index
    for from_id, to_id in zip(scheduling[:-1], scheduling[1:]):
        from_io = io_vec[from_id-1]
        to_io = io_vec[to_id-1]
        from_end = from_io['endLpos']
        to_start = to_io['startLpos']
        profiling_data += [
            {
                "name": "connect",
                "ph": "s",
                "id": f"{from_id}->{to_id}",
                "pid": "ioArray",
                # "tid": f"wrap{from_io['wrap']}",
                "tid": f"wrap{from_io['wrap'] if from_end < to_start else to_io['wrap']}",
                # "ts": from_end,
                "ts": min(from_end, to_start)
            },
            {
                "name": "connect",
                "ph": "f",
                "bp": "e",
                "id": f"{from_id}->{to_id}",
                "pid": "ioArray",
                # "tid": f"wrap{to_io['wrap']}",
                "tid": f"wrap{from_io['wrap'] if from_end > to_start else to_io['wrap']}",
                # "ts": to_start
                "ts": max(from_end, to_start)
            }
        ]

    profiling_json = json.dumps(profiling_data, indent="    ")

    return profiling_json





