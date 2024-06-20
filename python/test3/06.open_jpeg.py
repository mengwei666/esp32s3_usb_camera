import cv2

def read_usb_capture():
    # 选择摄像头的编号，并设置为MJPEG格式
    cap = cv2.VideoCapture(0)
    cap.set(cv2.CAP_PROP_FOURCC, cv2.VideoWriter_fourcc(*'MJPG'))

    while cap.isOpened():
        # 读取摄像头的画面
        ret, frame = cap.read()

        if not ret:
            break

        # 获取MJPEG数据
        ret, jpeg_data = cv2.imencode('.jpg', frame)
        if ret:
            # 打印MJPEG数据（这里仅打印长度作为示例，实际数据量较大，不建议全部打印）
            print(f'MJPEG data length: {len(jpeg_data)}')
            print("---------------------------------------------------------------")
            print(jpeg_data.tobytes().hex())
            print("---------------------------------------------------------------")

        # 显示图像
        cv2.imshow('USB Capture', frame)

        # 按下'q'就退出
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    # 释放画面
    cap.release()
    cv2.destroyAllWindows()

if __name__ == '__main__':
    read_usb_capture()

