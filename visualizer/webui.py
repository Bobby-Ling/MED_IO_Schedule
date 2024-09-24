# %%
import http.server
import os
from pathlib import Path
import socketserver
import webbrowser

# %%
file_dir = Path(__file__).parent

# %%
def start_server(tracing_file:str = "profiling.json", port:int = 8000, open_in_browser:bool =False):
    def start_srv_internal(tracing_file:str = "profiling.json", port:int = 8000, open_in_browser:bool =False):
        # 服务器端口
        PORT = port

        # 启动HTTP服务器的Handler类
        Handler = http.server.SimpleHTTPRequestHandler

        # 使用SocketServer启动服务器
        with socketserver.TCPServer(("", PORT), Handler) as httpd:
            print(f"Serving at port {PORT}")

            # 打开默认浏览器并加载tracing.html，同时传递参数
            url = f"http://localhost:{PORT}/tracing.html"
            print(f"url: {url}?tracing_url={tracing_file}")

            if open_in_browser:
                webbrowser.open(url)

            # 保持服务器运行
            httpd.serve_forever()

    started = False
    PORT = port
    while not started:
        try:
            start_srv_internal(tracing_file, PORT, open_in_browser)
            started = True
        except:
            PORT += 1
            continue

# %%
if __name__ == '__main__':
    # 命令行运行时执行, import导入时不执行
    os.chdir(file_dir / '../build/')
    start_server('profiling.json')
# %%
