#!/user/bin/env python

# Copyright (c) 2024，WuChao D-Robotics.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# 注意: 此程序在RDK板端端运行
# Attention: This program runs on RDK board.

import cv2
import numpy as np
from scipy.special import softmax
# from scipy.special import expit as sigmoid
from hobot_dnn import pyeasy_dnn as dnn  # BSP Python API

from time import time
import argparse
import logging 
import math
import serial
import datetime

# 日志模块配置
# logging configs
logging.basicConfig(
    level = logging.DEBUG,
    format = '[%(name)s] [%(asctime)s.%(msecs)03d] [%(levelname)s] %(message)s',
    datefmt='%H:%M:%S')
logger = logging.getLogger("RDK_YOLO")

# 串口发送控制变量
last_serial_time = 0
serial_cooldown = {}  # 每个类别的冷却时间

def send_serial_message(class_id, score, class_name):
    global last_serial_time, serial_cooldown
    
    if ser is None:
        logger.warning("串口未初始化，无法发送消息")
        return
    
    current_time = time()
    
    # 检查该类别的发送间隔（每个类别5秒内只发送一次）
    if class_id in serial_cooldown:
        if current_time - serial_cooldown[class_id] < 5.0:
            return
    
    try:
        # 将类别序号转为字符串发送
        text = str(class_id)
        
        data = text.encode('utf-8')
        
        # 发送数据
        ser.write(data)
        ser.write(b'\n')  # 发送换行
        serial_cooldown[class_id] = current_time
        last_serial_time = current_time
        logger.info(f"串口发送: {text} (类别序号) 对应药品: {medicine_names_chinese[class_id]} (置信度: {score:.2f})")
        
        # 确保数据发送完成
        ser.flush()
        
        # 可选：读取返回数据
        if ser.in_waiting:
            response = ser.read(ser.in_waiting).decode('utf-8', errors='ignore')
            if response:
                logger.info(f"串口接收: {response.strip()}")
                
    except Exception as e:
        logger.error(f"串口通信错误: {e}")

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--model-path', type=str, default='/home/sunrise/miniconda3/envs/lerobot/best_bayese_640x640_nv12.bin', 
                        help="""Path to BPU Quantized *.bin Model.
                                RDK X3(Module): Bernoulli2.
                                RDK Ultra: Bayes.
                                RDK X5(Module): Bayes-e.
                                RDK S100: Nash-e.
                                RDK S100P: Nash-m.""") 
    parser.add_argument('--test-img', type=str, default='../../../../resource/datasets/COCO2017/assets/bus.jpg', help='Path to Load Test Image.')
    parser.add_argument('--img-save-path', type=str, default='py_result.jpg', help='Path to Load Test Image.')
    parser.add_argument('--classes-num', type=int, default=8, help='Classes Num to Detect.')
    parser.add_argument('--nms-thres', type=float, default=0.7, help='IoU threshold.')
    parser.add_argument('--score-thres', type=float, default=0.25, help='confidence threshold.')
    parser.add_argument('--reg', type=int, default=16, help='DFL reg layer.')
    parser.add_argument('--camera-id', type=int, default=0, help='Camera device ID.')
    parser.add_argument('--serial-port', type=str, default='/dev/ttyS1', help='Serial port device path.')
    parser.add_argument('--baudrate', type=int, default=115200, help='Serial baud rate.')
    parser.add_argument('--serial-interval', type=float, default=1.0, help='Minimum interval between serial sends (seconds).')
    parser.add_argument('--display-threshold', type=float, default=0.5, help='Display threshold for bounding boxes (0-1).')
    opt = parser.parse_args()
    logger.info(opt)
    
    # 全局串口对象
    global ser
    # 重新初始化串口（使用命令行参数）
    try:
        ser = serial.Serial(opt.serial_port, opt.baudrate, timeout=0.5)
        logger.info(f"串口初始化成功: {opt.serial_port}@{opt.baudrate}")
    except Exception as e:
        logger.error(f"串口初始化失败: {e}")
        ser = None

    # 实例化
    model = YOLO11_Detect(opt)
    
    # 初始化摄像头
    cap = cv2.VideoCapture(opt.camera_id)
    if not cap.isOpened():
        logger.error("无法打开摄像头")
        return
        
    logger.info(f"开始实时摄像头检测，显示阈值: {opt.display_threshold}")
    logger.info("按 'q' 退出，按 's' 保存当前帧，按 't' 测试串口")
    
    # 设置摄像头参数（可选）
    cap.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
    cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)
    cap.set(cv2.CAP_PROP_FPS, 30)
    
    frame_count = 0
    fps_start_time = time()
    fps = 0
    
    # 串口发送控制变量
    detected_objects = {}  # 用于跟踪检测到的物体，避免重复发送
    
    while True:
        ret, frame = cap.read()
        if not ret:
            logger.error("无法读取摄像头帧")
            break
        
        # 计算FPS
        frame_count += 1
        if frame_count >= 30:
            fps = frame_count / (time() - fps_start_time)
            fps_start_time = time()
            frame_count = 0
        
        # 记录开始时间
        process_start_time = time()
        
        # 准备输入数据
        input_tensor = model.preprocess_yuv420sp(frame)
        
        # 推理
        outputs = model.c2numpy(model.forward(input_tensor))
        
        # 后处理
        results = model.postProcess(outputs)
        
        # 计算处理时间
        process_time = time() - process_start_time
        
        # 渲染结果
        display_frame = frame.copy()
        logger.info("\033[1;32m" + f"Frame FPS: {fps:.1f}, Process time: {process_time*1000:.1f}ms" + "\033[0m")
        
        current_time = time()
        current_detections = {}  # 当前帧检测到的物体
        
        if len(results) > 0:
            logger.info("\033[1;32m" + "检测结果: " + "\033[0m")
            for class_id, score, x1, y1, x2, y2 in results:
                # 只有当概率大于显示阈值时才显示
                if score >= opt.display_threshold:
                    # 日志输出使用类别序号，不显示中文
                    logger.info(f"({x1}, {y1}, {x2}, {y2}) -> Class {class_id}: {score:.2f}")
                    # 绘制检测框
                    draw_detection(display_frame, (x1, y1, x2, y2), score, class_id)
                    
                    # 创建检测对象的唯一标识符（基于位置和类别）
                    obj_key = f"{class_id}_{x1//30}_{y1//30}"  # 粗略位置分组
                    current_detections[obj_key] = (class_id, score)
                    
                    # 检查是否需要通过串口发送消息
                    if ser is not None and score >= opt.display_threshold:
                        # 如果是新检测到的物体
                        if obj_key not in detected_objects:
                            # 串口部分保持不变，仍然使用中文名称
                            send_serial_message(class_id, score, medicine_names_chinese[class_id])
                else:
                    # 概率低于显示阈值的检测结果，仅记录日志但不显示
                    # 日志输出使用类别序号，不显示中文
                    logger.debug(f"低概率检测(不显示): ({x1}, {y1}, {x2}, {y2}) -> Class {class_id}: {score:.2f}")
        else:
            logger.debug("未检测到目标")
        
        # 更新检测到的物体记录（只保留当前帧检测到的）
        detected_objects = current_detections
        
        # 显示FPS信息
        cv2.putText(display_frame, f"FPS: {fps:.1f}", (10, 30), 
                    cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)
        cv2.putText(display_frame, f"Process: {process_time*1000:.1f}ms", (10, 60), 
                    cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)
        
        # 显示检测类别数量
        cv2.putText(display_frame, f"Detections: {len(results)}", (10, 90), 
                    cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)
        
        # 显示串口状态
        serial_status = "Serial: ON" if ser else "Serial: OFF"
        cv2.putText(display_frame, serial_status, (10, 120), 
                    cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0) if ser else (0, 0, 255), 2)
        
        # 显示高置信度检测数量
        high_conf_detections = sum(1 for _, score, *_ in results if score >= opt.display_threshold)
        cv2.putText(display_frame, f"High Conf: {high_conf_detections}", (10, 150), 
                    cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0) if high_conf_detections > 0 else (0, 0, 255), 2)
        
        # 显示图像
        cv2.imshow('Medicine Detection', display_frame)
        
        # 键盘控制
        key = cv2.waitKey(1) & 0xFF
        if key == ord('q'):
            break
        elif key == ord('s'):
            # 保存当前帧
            save_path = f"capture_{int(time())}.jpg"
            cv2.imwrite(save_path, display_frame)
            logger.info(f"图像已保存到: {save_path}")
        elif key == ord(' '):
            # 空格键暂停/继续
            logger.info("暂停，按任意键继续...")
            cv2.waitKey(0)
        elif key == ord('t'):
            # 测试串口发送
            if ser:
                ser.write(b"Test message\n")
                logger.info("发送测试消息")
        elif key == ord('+') or key == ord('='):
            # 增加显示阈值
            opt.display_threshold = min(0.95, opt.display_threshold + 0.05)
            logger.info(f"显示阈值增加至: {opt.display_threshold:.2f}")
        elif key == ord('-') or key == ord('_'):
            # 减少显示阈值
            opt.display_threshold = max(0.05, opt.display_threshold - 0.05)
            logger.info(f"显示阈值减少至: {opt.display_threshold:.2f}")
    
    # 释放资源
    cap.release()
    cv2.destroyAllWindows()
    
    # 关闭串口
    if ser:
        ser.close()
        logger.info("串口已关闭")
    
    logger.info("程序结束")

class YOLO11_Detect():
    def __init__(self, opt):
        # 加载BPU的bin模型, 打印相关参数
        # Load the quantized *.bin model and print its parameters
        try:
            begin_time = time()
            self.quantize_model = dnn.load(opt.model_path)
            logger.info("\033[1;31m" + "Load D-Robotics Quantize model time = %.2f ms"%(1000*(time() - begin_time)) + "\033[0m")
            logger.info("\033[1;32m" + f"Model loaded successfully: {opt.model_path}" + "\033[0m")
        except Exception as e:
            logger.error("❌ Failed to load model file: %s"%(opt.model_path))
            logger.error("Please check if the model file exists and is compatible with your RDK device.")
            logger.error(e)
            exit(1)

        logger.info("\033[1;32m" + "-> input tensors" + "\033[0m")
        for i, quantize_input in enumerate(self.quantize_model[0].inputs):
            logger.info(f"input[{i}], name={quantize_input.name}, type={quantize_input.properties.dtype}, shape={quantize_input.properties.shape}")

        logger.info("\033[1;32m" + "-> output tensors" + "\033[0m")
        for i, quantize_input in enumerate(self.quantize_model[0].outputs):
            logger.info(f"output[{i}], name={quantize_input.name}, type={quantize_input.properties.dtype}, shape={quantize_input.properties.shape}")

        # ============= 修复：处理反量化系数为空的情况 =============
        # 检查scale_data是否有效，如果无效则使用1.0
        try:
            self.s_bboxes_scale = self.quantize_model[0].outputs[1].properties.scale_data
            if self.s_bboxes_scale is None or len(self.s_bboxes_scale) == 0:
                self.s_bboxes_scale = np.array([1.0], dtype=np.float32)
            else:
                self.s_bboxes_scale = np.array(self.s_bboxes_scale, dtype=np.float32)
            self.s_bboxes_scale = self.s_bboxes_scale[np.newaxis, :]
        except:
            self.s_bboxes_scale = np.array([[1.0]], dtype=np.float32)
            
        try:
            self.m_bboxes_scale = self.quantize_model[0].outputs[3].properties.scale_data
            if self.m_bboxes_scale is None or len(self.m_bboxes_scale) == 0:
                self.m_bboxes_scale = np.array([1.0], dtype=np.float32)
            else:
                self.m_bboxes_scale = np.array(self.m_bboxes_scale, dtype=np.float32)
            self.m_bboxes_scale = self.m_bboxes_scale[np.newaxis, :]
        except:
            self.m_bboxes_scale = np.array([[1.0]], dtype=np.float32)
            
        try:
            self.l_bboxes_scale = self.quantize_model[0].outputs[5].properties.scale_data
            if self.l_bboxes_scale is None or len(self.l_bboxes_scale) == 0:
                self.l_bboxes_scale = np.array([1.0], dtype=np.float32)
            else:
                self.l_bboxes_scale = np.array(self.l_bboxes_scale, dtype=np.float32)
            self.l_bboxes_scale = self.l_bboxes_scale[np.newaxis, :]
        except:
            self.l_bboxes_scale = np.array([[1.0]], dtype=np.float32)
            
        logger.info(f"self.s_bboxes_scale.shape={self.s_bboxes_scale.shape}, self.m_bboxes_scale.shape={self.m_bboxes_scale.shape}, self.l_bboxes_scale.shape={self.l_bboxes_scale.shape}")

        # DFL求期望的系数, 只需要生成一次
        # DFL calculates the expected coefficients, which only needs to be generated once.
        self.weights_static = np.array([i for i in range(opt.reg)]).astype(np.float32)[np.newaxis, np.newaxis, :]
        logger.info(f"{self.weights_static.shape = }")

        # anchors, 只需要生成一次
        self.s_anchor = np.stack([np.tile(np.linspace(0.5, 79.5, 80), reps=80), 
                            np.repeat(np.arange(0.5, 80.5, 1), 80)], axis=0).transpose(1,0)
        self.m_anchor = np.stack([np.tile(np.linspace(0.5, 39.5, 40), reps=40), 
                            np.repeat(np.arange(0.5, 40.5, 1), 40)], axis=0).transpose(1,0)
        self.l_anchor = np.stack([np.tile(np.linspace(0.5, 19.5, 20), reps=20), 
                            np.repeat(np.arange(0.5, 20.5, 1), 20)], axis=0).transpose(1,0)
        logger.info(f"{self.s_anchor.shape = }, {self.m_anchor.shape = }, {self.l_anchor.shape = }")

        # 输入图像大小, 一些阈值, 提前计算好
        self.input_image_size = 640
        self.SCORE_THRESHOLD = opt.score_thres
        self.NMS_THRESHOLD = opt.nms_thres
        self.CONF_THRES_RAW = -np.log(1/self.SCORE_THRESHOLD - 1)
        logger.info("SCORE_THRESHOLD  = %.2f, NMS_THRESHOLD = %.2f"%(self.SCORE_THRESHOLD, self.NMS_THRESHOLD))
        logger.info("CONF_THRES_RAW = %.2f"%self.CONF_THRES_RAW)

        self.input_H, self.input_W = self.quantize_model[0].inputs[0].properties.shape[2:4]
        logger.info(f"{self.input_H = }, {self.input_W = }")

        self.REG = opt.reg
        logger.info(f"{self.REG = }")

        self.CLASSES_NUM = opt.classes_num
        logger.info(f"{self.CLASSES_NUM = }")

    def preprocess_yuv420sp(self, img):
        RESIZE_TYPE = 0
        LETTERBOX_TYPE = 1
        PREPROCESS_TYPE = LETTERBOX_TYPE
        
        begin_time = time()
        self.img_h, self.img_w = img.shape[0:2]
        
        if PREPROCESS_TYPE == RESIZE_TYPE:
            # 利用resize的方式进行前处理, 准备nv12的输入数据
            input_tensor = cv2.resize(img, (self.input_W, self.input_H), interpolation=cv2.INTER_NEAREST)
            input_tensor = self.bgr2nv12(input_tensor)
            self.y_scale = 1.0 * self.input_H / self.img_h
            self.x_scale = 1.0 * self.input_W / self.img_w
            self.y_shift = 0
            self.x_shift = 0
            logger.debug("\033[1;31m" + f"pre process(resize) time = {1000*(time() - begin_time):.2f} ms" + "\033[0m")
        elif PREPROCESS_TYPE == LETTERBOX_TYPE:
            # 利用 letter box 的方式进行前处理, 准备nv12的输入数据
            self.x_scale = min(1.0 * self.input_H / self.img_h, 1.0 * self.input_W / self.img_w)
            self.y_scale = self.x_scale
            
            if self.x_scale <= 0 or self.y_scale <= 0:
                raise ValueError("Invalid scale factor.")
            
            new_w = int(self.img_w * self.x_scale)
            self.x_shift = (self.input_W - new_w) // 2
            x_other = self.input_W - new_w - self.x_shift
            
            new_h = int(self.img_h * self.y_scale)
            self.y_shift = (self.input_H - new_h) // 2
            y_other = self.input_H - new_h - self.y_shift
            
            input_tensor = cv2.resize(img, (new_w, new_h))
            input_tensor = cv2.copyMakeBorder(input_tensor, self.y_shift, y_other, self.x_shift, x_other, cv2.BORDER_CONSTANT, value=[127, 127, 127])
            input_tensor = self.bgr2nv12(input_tensor)
            logger.debug("\033[1;31m" + f"pre process(letter box) time = {1000*(time() - begin_time):.2f} ms" + "\033[0m")
        else:
            logger.error(f"illegal PREPROCESS_TYPE = {PREPROCESS_TYPE}")
            exit(-1)

        logger.debug("\033[1;31m" + f"pre process time = {1000*(time() - begin_time):.2f} ms" + "\033[0m")
        logger.debug(f"y_scale = {self.y_scale:.2f}, x_scale = {self.x_scale:.2f}")
        logger.debug(f"y_shift = {self.y_shift:.2f}, x_shift = {self.x_shift:.2f}")
        return input_tensor

    def bgr2nv12(self, bgr_img):
        begin_time = time()
        height, width = bgr_img.shape[0], bgr_img.shape[1]
        area = height * width
        yuv420p = cv2.cvtColor(bgr_img, cv2.COLOR_BGR2YUV_I420).reshape((area * 3 // 2,))
        y = yuv420p[:area]
        uv_planar = yuv420p[area:].reshape((2, area // 4))
        uv_packed = uv_planar.transpose((1, 0)).reshape((area // 2,))
        nv12 = np.zeros_like(yuv420p)
        nv12[:height * width] = y
        nv12[height * width:] = uv_packed
        logger.debug("\033[1;31m" + f"bgr8 to nv12 time = {1000*(time() - begin_time):.2f} ms" + "\033[0m")
        return nv12

    def forward(self, input_tensor):
        begin_time = time()
        quantize_outputs = self.quantize_model[0].forward(input_tensor)
        logger.debug("\033[1;31m" + f"forward time = {1000*(time() - begin_time):.2f} ms" + "\033[0m")
        return quantize_outputs

    def c2numpy(self, outputs):
        begin_time = time()
        outputs = [dnnTensor.buffer for dnnTensor in outputs]
        logger.debug("\033[1;31m" + f"c to numpy time = {1000*(time() - begin_time):.2f} ms" + "\033[0m")
        return outputs

    def postProcess(self, outputs):
        begin_time = time()
        
        # ============= 修复：处理可能为空的有效索引 =============
        # reshape
        s_clses = outputs[0].reshape(-1, self.CLASSES_NUM)
        s_bboxes = outputs[1].reshape(-1, self.REG * 4)
        m_clses = outputs[2].reshape(-1, self.CLASSES_NUM)
        m_bboxes = outputs[3].reshape(-1, self.REG * 4)
        l_clses = outputs[4].reshape(-1, self.CLASSES_NUM)
        l_bboxes = outputs[5].reshape(-1, self.REG * 4)

        # classify: 利用numpy向量化操作完成阈值筛选
        s_max_scores = np.max(s_clses, axis=1)
        s_valid_indices = np.flatnonzero(s_max_scores >= self.CONF_THRES_RAW)
        
        m_max_scores = np.max(m_clses, axis=1)
        m_valid_indices = np.flatnonzero(m_max_scores >= self.CONF_THRES_RAW)
        
        l_max_scores = np.max(l_clses, axis=1)
        l_valid_indices = np.flatnonzero(l_max_scores >= self.CONF_THRES_RAW)

        # 初始化结果列表
        dbboxes_list = []
        scores_list = []
        ids_list = []

        # 处理小尺度特征层
        if len(s_valid_indices) > 0:
            s_ids = np.argmax(s_clses[s_valid_indices, :], axis=1)
            s_scores = s_max_scores[s_valid_indices]
            
            # Sigmoid计算
            s_scores = 1 / (1 + np.exp(-s_scores))
            
            # Bounding Box分支
            s_bboxes_float32 = s_bboxes[s_valid_indices, :].astype(np.float32)
            # 应用反量化（如果scale有效）
            if self.s_bboxes_scale.shape[1] > 0:
                s_bboxes_float32 = s_bboxes_float32 * self.s_bboxes_scale
            
            # dist2bbox
            s_ltrb_indices = np.sum(softmax(s_bboxes_float32.reshape(-1, 4, self.REG), axis=2) * self.weights_static, axis=2)
            s_anchor_indices = self.s_anchor[s_valid_indices, :]
            s_x1y1 = s_anchor_indices - s_ltrb_indices[:, 0:2]
            s_x2y2 = s_anchor_indices + s_ltrb_indices[:, 2:4]
            s_dbboxes = np.hstack([s_x1y1, s_x2y2]) * 8
            
            dbboxes_list.append(s_dbboxes)
            scores_list.append(s_scores)
            ids_list.append(s_ids)

        # 处理中尺度特征层
        if len(m_valid_indices) > 0:
            m_ids = np.argmax(m_clses[m_valid_indices, :], axis=1)
            m_scores = m_max_scores[m_valid_indices]
            
            # Sigmoid计算
            m_scores = 1 / (1 + np.exp(-m_scores))
            
            # Bounding Box分支
            m_bboxes_float32 = m_bboxes[m_valid_indices, :].astype(np.float32)
            # 应用反量化（如果scale有效）
            if self.m_bboxes_scale.shape[1] > 0:
                m_bboxes_float32 = m_bboxes_float32 * self.m_bboxes_scale
            
            # dist2bbox
            m_ltrb_indices = np.sum(softmax(m_bboxes_float32.reshape(-1, 4, self.REG), axis=2) * self.weights_static, axis=2)
            m_anchor_indices = self.m_anchor[m_valid_indices, :]
            m_x1y1 = m_anchor_indices - m_ltrb_indices[:, 0:2]
            m_x2y2 = m_anchor_indices + m_ltrb_indices[:, 2:4]
            m_dbboxes = np.hstack([m_x1y1, m_x2y2]) * 16
            
            dbboxes_list.append(m_dbboxes)
            scores_list.append(m_scores)
            ids_list.append(m_ids)

        # 处理大尺度特征层
        if len(l_valid_indices) > 0:
            l_ids = np.argmax(l_clses[l_valid_indices, :], axis=1)
            l_scores = l_max_scores[l_valid_indices]
            
            # Sigmoid计算
            l_scores = 1 / (1 + np.exp(-l_scores))
            
            # Bounding Box分支
            l_bboxes_float32 = l_bboxes[l_valid_indices, :].astype(np.float32)
            # 应用反量化（如果scale有效）
            if self.l_bboxes_scale.shape[1] > 0:
                l_bboxes_float32 = l_bboxes_float32 * self.l_bboxes_scale
            
            # dist2bbox
            l_ltrb_indices = np.sum(softmax(l_bboxes_float32.reshape(-1, 4, self.REG), axis=2) * self.weights_static, axis=2)
            l_anchor_indices = self.l_anchor[l_valid_indices, :]
            l_x1y1 = l_anchor_indices - l_ltrb_indices[:, 0:2]
            l_x2y2 = l_anchor_indices + l_ltrb_indices[:, 2:4]
            l_dbboxes = np.hstack([l_x1y1, l_x2y2]) * 32
            
            dbboxes_list.append(l_dbboxes)
            scores_list.append(l_scores)
            ids_list.append(l_ids)

        # 如果没有检测到任何目标，直接返回空列表
        if len(dbboxes_list) == 0:
            logger.debug("未检测到任何目标")
            return []

        # 拼接所有特征层的结果
        dbboxes = np.concatenate(dbboxes_list, axis=0)
        scores = np.concatenate(scores_list, axis=0)
        ids = np.concatenate(ids_list, axis=0)

        # xyxy 2 xyhw (用于NMS)
        hw = (dbboxes[:, 2:4] - dbboxes[:, 0:2])
        xyhw2 = np.hstack([dbboxes[:, 0:2], hw])

        # 分类别nms
        results = []
        for i in range(self.CLASSES_NUM):
            id_indices = ids == i
            if np.sum(id_indices) == 0:
                continue
                
            # 使用cv2.dnn.NMSBoxes进行非极大值抑制
            bboxes_list = xyhw2[id_indices, :].tolist()
            scores_list = scores[id_indices].tolist()
            
            try:
                indices = cv2.dnn.NMSBoxes(bboxes_list, scores_list, self.SCORE_THRESHOLD, self.NMS_THRESHOLD)
                if len(indices) == 0:
                    continue
                
                # 兼容不同版本的OpenCV返回值
                if isinstance(indices, tuple):
                    indices = indices[0]
                indices = [int(i) for i in indices]
                
                dbboxes_filtered = dbboxes[id_indices, :]
                scores_filtered = scores[id_indices]
                
                for idx in indices:
                    x1, y1, x2, y2 = dbboxes_filtered[idx]
                    
                    # 坐标转换回原图
                    x1 = int((x1 - self.x_shift) / self.x_scale)
                    y1 = int((y1 - self.y_shift) / self.y_scale)
                    x2 = int((x2 - self.x_shift) / self.x_scale)
                    y2 = int((y2 - self.y_shift) / self.y_scale)

                    # 边界检查
                    x1 = max(0, min(x1, self.img_w))
                    x2 = max(0, min(x2, self.img_w))
                    y1 = max(0, min(y1, self.img_h))
                    y2 = max(0, min(y2, self.img_h))
                    
                    # 确保x1 <= x2, y1 <= y2
                    if x1 > x2:
                        x1, x2 = x2, x1
                    if y1 > y2:
                        y1, y2 = y2, y1
                    
                    # 过滤无效框
                    if x2 - x1 < 5 or y2 - y1 < 5:  # 过滤过小的检测框
                        continue
                        
                    results.append((i, scores_filtered[idx], x1, y1, x2, y2))
            except Exception as e:
                logger.warning(f"NMS error for class {i}: {e}")
                continue

        logger.debug("\033[1;31m" + f"Post Process time = {1000*(time() - begin_time):.2f} ms" + "\033[0m")
        logger.debug(f"检测到 {len(results)} 个目标")

        return results

# 8个药品类别（根据您的需求）
medicine_names = [
    "buluofen",                    # 0: 布洛芬
    "ganmaoqingrekeli",            # 1: 感冒清热颗粒
    "huoxiangzhengqijiaonang",     # 2: 藿香正气胶囊
    "kaixiongshuqiwan",            # 3: 开胸顺气丸
    "lianhuaqingwenkeli",          # 4: 莲花清瘟颗粒
    "niuhuangqingweiwan",          # 5: 牛黄清胃丸
    "shuanghuangliankouduye",      # 6: 双黄连口服液
    "banlangenkeli"                # 7: 板蓝根颗粒
]

# 中文名称（用于串口日志，保持不变）
medicine_names_chinese = [
    "布洛芬",                      # 0
    "感冒清热颗粒",                # 1
    "藿香正气胶囊",                # 2
    "开胸顺气丸",                  # 3
    "莲花清瘟颗粒",                # 4
    "牛黄清胃丸",                  # 5
    "双黄连口服液",                # 6
    "板蓝根颗粒"                   # 7
]

rdk_colors = [
    (56, 56, 255), (151, 157, 255), (31, 112, 255), (29, 178, 255),
    (49, 210, 207), (10, 249, 72), (23, 204, 146), (134, 219, 61),
    (52, 147, 26), (187, 212, 0), (168, 153, 44), (255, 194, 0),
    (147, 69, 52), (255, 115, 100), (236, 24, 0), (255, 56, 132),
    (133, 0, 82), (255, 56, 203), (200, 149, 255), (199, 55, 255)
]

def draw_detection(img, bbox, score, class_id) -> None:
    """
    Draws a detection bounding box and label on the image.

    Parameters:
        img (np.array): The input image.
        bbox (tuple[int, int, int, int]): A tuple containing the bounding box coordinates (x1, y1, x2, y2).
        score (float): The detection score of the object.
        class_id (int): The class ID of the detected object.
    """
    # 注意：调用此函数前已经确保score >= 0.5
    x1, y1, x2, y2 = bbox
    color = rdk_colors[class_id % 20]
    
    # 绘制边界框
    cv2.rectangle(img, (x1, y1), (x2, y2), color, 2)
    
    # 创建标签（显示类别序号，不显示中文）
    label = f"Class {class_id}: {score:.2f}"
    
    # 计算标签大小
    (label_width, label_height), baseline = cv2.getTextSize(label, cv2.FONT_HERSHEY_SIMPLEX, 0.5, 1)
    
    # 确保标签在图像内
    label_y = max(y1 - 5, label_height + 5)
    
    # 绘制标签背景
    cv2.rectangle(img, 
                  (x1, label_y - label_height - 5), 
                  (x1 + label_width + 5, label_y + 5), 
                  color, cv2.FILLED)
    
    # 绘制标签文本
    cv2.putText(img, label, (x1 + 2, label_y - 2), 
                cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 255), 1, cv2.LINE_AA)

if __name__ == "__main__":
    main()
