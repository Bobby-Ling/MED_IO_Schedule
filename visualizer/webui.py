import http.server
import socketserver
import webbrowser
import sys

tracing_file = sys.argv[1] if len(sys.argv) == 2 else "profiling.json"

def start_server(tracing_file:str):
    # 服务器端口
    PORT = 8000

    # 启动HTTP服务器的Handler类
    Handler = http.server.SimpleHTTPRequestHandler

    # 使用SocketServer启动服务器
    with socketserver.TCPServer(("", PORT), Handler) as httpd:
        print(f"Serving at port {PORT}")

        # 打开默认浏览器并加载tracing.html，同时传递参数
        url = f"http://localhost:{PORT}/tracing.html?tracing_url={tracing_file}"
        print(f"url: {url}")
        webbrowser.open(url)

        # 保持服务器运行
        httpd.serve_forever()

start_server(tracing_file)