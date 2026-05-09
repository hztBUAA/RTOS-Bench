#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""检查板卡上的文件 - 带Telnet协议协商"""
import socket
import time
import sys

# Telnet协议常量
IAC = 255  # Interpret As Command
DONT = 254
DO = 253
WONT = 252
WILL = 251
SB = 250
SE = 240

def negotiate_telnet(sock, data):
    """处理Telnet协议协商"""
    response = b""
    i = 0
    text = b""

    while i < len(data):
        if data[i] == IAC:
            if i + 2 < len(data):
                cmd = data[i+1]
                opt = data[i+2]

                # 响应协商
                if cmd == DO:
                    response += bytes([IAC, WONT, opt])
                elif cmd == WILL:
                    response += bytes([IAC, DONT, opt])

                i += 3
            else:
                break
        else:
            text += bytes([data[i]])
            i += 1

    if response:
        sock.sendall(response)

    return text


def check_board():
    board_ip = '192.168.31.204'
    board_port = 23
    username = 'root'
    password = 'root'

    try:
        print(f"[1/6] 连接到板卡 {board_ip}:{board_port}")
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(30)
        sock.connect((board_ip, board_port))

        # 等待login提示
        print("[2/6] 等待login提示...")
        buffer = b""
        start = time.time()
        while time.time() - start < 10:
            data = sock.recv(4096)
            text = negotiate_telnet(sock, data)
            buffer += text
            print(f"Text: {text}")
            if b"login:" in buffer:
                break

        # 发送用户名
        print(f"[3/6] 发送用户名: {username}")
        sock.sendall(username.encode() + b"\r\n")
        time.sleep(1)

        # 等待password提示
        print("[4/6] 等待password提示...")
        buffer = b""
        start = time.time()
        while time.time() - start < 10:
            data = sock.recv(4096)
            text = negotiate_telnet(sock, data)
            buffer += text
            print(f"Text: {text}")
            if b"password:" in buffer or b"Password:" in buffer:
                break

        # 发送密码
        print("[5/6] 发送密码")
        sock.sendall(password.encode() + b"\r\n")
        time.sleep(2)

        # 等待shell提示符
        print("[6/6] 等待shell提示符...")
        buffer = b""
        start = time.time()
        while time.time() - start < 10:
            try:
                sock.settimeout(1)
                data = sock.recv(4096)
                text = negotiate_telnet(sock, data)
                buffer += text
                print(f"Text: {text}")
                if b"#" in buffer:
                    break
            except socket.timeout:
                if b"#" in buffer:
                    break

        # 执行ls命令
        print("\n[TEST] 检查 /apps/ 目录...")
        sock.sendall(b"ls -la /apps/\r\n")
        time.sleep(2)

        # 读取输出
        output = b""
        start = time.time()
        while time.time() - start < 5:
            try:
                sock.settimeout(1)
                data = sock.recv(4096)
                if not data:
                    break
                text = negotiate_telnet(sock, data)
                output += text
                print(text.decode('utf-8', errors='ignore'), end='', flush=True)
            except socket.timeout:
                continue

        # 退出
        sock.sendall(b"exit\r\n")
        sock.close()

    except Exception as e:
        print(f"\n[ERROR] 失败: {e}")
        import traceback
        traceback.print_exc()
        return 1

if __name__ == "__main__":
    sys.exit(check_board())
