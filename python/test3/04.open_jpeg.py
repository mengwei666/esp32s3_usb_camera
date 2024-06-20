import cv2
import matplotlib.pyplot as plt


def read_usb_capture():
    # 选择摄像头的编号
    cap = cv2.VideoCapture(0)

    # 创建一个图像窗口
    fig, ax = plt.subplots()
    ax.axis('off')  # 关闭坐标轴

    while (cap.isOpened()):
        # 读取摄像头的画面
        ret, frame = cap.read()

        # 将BGR格式转换为RGB格式
        rgb_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)

        # 显示图像
        ax.imshow(rgb_frame)
        plt.pause(0.001)  # 暂停一小段时间，使图像窗口有时间更新

        # 按下'q'就退出
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    # 释放画面
    cap.release()
    plt.close()


if __name__ == '__main__':
    read_usb_capture()






