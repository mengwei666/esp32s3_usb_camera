import requests

url = "http://192.168.4.1:81/stream"

try:
    response = requests.get(url, stream=True)  # 设置 stream=True 来接收分块数据
    if response.status_code == 200:
        # 读取响应的分块数据并处理
        for chunk in response.iter_content(chunk_size=1024):  # 指定每次读取的数据块大小
            # 在这里处理接收到的分块数据，可以保存到文件、处理图像等
            print("Received chunk of size:", len(chunk))
    else:
        print("Failed to fetch data:", response.status_code)
except Exception as e:
    print("An error occurred:", e)
