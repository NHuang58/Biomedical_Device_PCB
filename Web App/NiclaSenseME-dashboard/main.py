import webview
import threading
import http.server
import socketserver
import os

PORT = 8000
DIRECTORY = os.path.dirname(os.path.abspath(__file__))

class Handler(http.server.SimpleHTTPRequestHandler):
    def translate_path(self, path):
        # Serve files from this directory
        path = os.path.join(DIRECTORY, path.lstrip("/"))
        return path

def start_server():
    with socketserver.TCPServer(("", PORT), Handler) as httpd:
        print(f"Serving at port {PORT}")
        httpd.serve_forever()

if __name__ == "__main__":
    server_thread = threading.Thread(target=start_server, daemon=True)
    server_thread.start()

    webview.create_window(
        "Arduino BLE Dashboard",
        f"http://localhost:{PORT}/index.html",
        width=1280,
        height=720
    )
    webview.start()
