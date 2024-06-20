import cv2
import matplotlib.pyplot as plt


def read_usb_capture():
    # 选择摄像头的编号
    cap = cv2.VideoCapture(0)
    while (cap.isOpened()):
        # 读取摄像头的画面
        ret, frame = cap.read()

        # 将BGR格式转换为RGB格式
        rgb_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)

        # 显示图像
        plt.imshow(rgb_frame)
        plt.axis('off')  # 关闭坐标轴
        plt.show()

        # 按下'q'就退出
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break
    # 释放画面
    cap.release()


if __name__ == '__main__':
    read_usb_capture()




