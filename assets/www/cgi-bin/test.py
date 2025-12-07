#!/usr/bin/env python3
# test.py — простой CGI-тест: выводит env, query и body

import os
import sys
import html

def read_stdin():
    cl = os.environ.get('CONTENT_LENGTH')
    if not cl:
        return ""
    try:
        length = int(cl)
    except ValueError:
        return ""
    return sys.stdin.read(length)

def main():
    print("Content-Type: text/plain")

    #TEST for Content-Length
    # print("Content-Length: 200")
    print()  # разделитель заголовков и тела

    # Печатаем базовую информацию
    print("=== CGI debug ===")
    print("REQUEST_METHOD:", os.environ.get("REQUEST_METHOD", ""))
    print("QUERY_STRING :", os.environ.get("QUERY_STRING", ""))
    print("SCRIPT_NAME  :", os.environ.get("SCRIPT_NAME", ""))
    print("SCRIPT_FILENAME:", os.environ.get("SCRIPT_FILENAME", ""))
    print("REMOTE_ADDR  :", os.environ.get("REMOTE_ADDR", ""))
    print()

    # Печатаем все HTTP_* заголовки (если есть)
    print("--- HTTP_* headers ---")
    for k, v in sorted(os.environ.items()):
        if k.startswith("HTTP_"):
            print(f"{k} = {v}")
    print()

    # Тело (POST)
    if os.environ.get("REQUEST_METHOD", "").upper() == "POST":
        body = read_stdin()
        print("--- POST body (raw) ---")
        # Экранируем HTML на случай, если потом выведем в HTML; тут plain text, но на всякий случай
        print(body)
    else:
        print("--- No POST body (method is not POST) ---")

if __name__ == "__main__":
    main()
