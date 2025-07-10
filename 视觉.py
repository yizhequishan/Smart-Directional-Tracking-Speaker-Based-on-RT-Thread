import sensor, time, image, lcd
from pyb import UART

# 初始化串口
uart = UART(2, 115200)  # 根据实际情况修改串口号和波特率

# 初始化sensor
sensor.reset()
sensor.set_pixformat(sensor.RGB565)  # 保持彩色捕获，用于显示
sensor.set_framesize(sensor.HQVGA)   # 设置分辨率
sensor.set_contrast(3)
sensor.set_gainceiling(16)
sensor.set_windowing((320, 240))

# 跳过几帧，让感光元件稳定下来
sensor.skip_frames(time=2000)

# 初始化LCD
lcd.init()

# 加载Haar算子
face_cascade = image.HaarCascade("frontalface", stages=25)
print(face_cascade)

# 图像中心
IMAGE_CENTER_X = 230 // 2
IMAGE_CENTER_Y = 180 // 2

# 平滑窗口大小
SMOOTH_WINDOW = 3
offset_history = []  # 存储历史偏移值用于平滑

clock = time.clock()

while True:
    clock.tick()
    # 捕获彩色图像
    color_img = sensor.snapshot()
    color_img = color_img.replace(hmirror=True, vflip=True)
    display_img = color_img.copy()  # 创建显示副本

    # 创建灰度图用于处理
    gray_img = color_img.to_grayscale()
    gray_img = gray_img.histeq()  # 直方图均衡化增强对比度

    # 在灰度图上检测人脸
    objects = gray_img.find_features(face_cascade, threshold=0.5, scale=1.35)

    if objects:
        # 取第一个人脸
        x, y, w, h = objects[0]
        # 计算人脸中心
        face_center_x = x + w // 2
        face_center_y = y + h // 2
        # 计算偏移量
        offset_x = face_center_x - IMAGE_CENTER_X
        offset_y = face_center_y - IMAGE_CENTER_Y

        # 平滑偏移量
        offset_history.append((offset_x, offset_y))
        if len(offset_history) > SMOOTH_WINDOW:
            offset_history.pop(0)  # 移除最早的记录
        avg_offset_x = sum([o[0] for o in offset_history]) // len(offset_history)
        avg_offset_y = sum([o[1] for o in offset_history]) // len(offset_history)

        # 在彩色图像上绘制人脸矩形和中心
        display_img.draw_rectangle(objects[0], color=(0, 255, 0))
        display_img.draw_cross(face_center_x, face_center_y, size=10, color=(255, 0, 0))
        display_img.draw_string(10, 20, f"Offset: ({avg_offset_x}, {avg_offset_y})", color=(255, 0, 0))

        # 通过串口发送平滑后的偏移值
        uart.write(f"{avg_offset_x},{avg_offset_y}\n")

    else:
        display_img.draw_string(0, 20, "No face detected", color=(255, 0, 0))

    # 在彩色图像上绘制FPS
    display_img.draw_string(0, 0, "FPS:%.2f" % (clock.fps()), color=(0, 255, 0))

    # 显示彩色图像到LCD
    lcd.display(display_img)
