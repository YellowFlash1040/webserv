#!/usr/bin/env python3

# test_redirect.py — CGI-скрипт, который всегда возвращает редирект

import os
import sys

def main():
    # Можно указать статус вручную:
    # print("Status: 301 Moved Permanently")
    # Или оставить серверу решить (тогда твой сервер должен поставить 302)

    print("Location: https://example.com")  # <-- ключевой заголовок
    print("Content-Type: text/plain")
    print()  # разделитель заголовков и тела

    # Браузер почти всегда игнорирует body при редиректе, но для теста полезно оставить:
    print("You should be redirected to https://example.com")

if __name__ == "__main__":
    main()
