import cv2


def read_usb_capture():
    # 选择摄像头的编号
    cap = cv2.VideoCapture(0)
    while cap.isOpened():
        # 读取摄像头的画面
        ret, frame = cap.read()

        if not ret:
            break

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

