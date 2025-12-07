#!/usr/bin/env python3
# test_utf8.py — simple CGI debug script

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
    # Output headers: must include Content-Type and blank line
    print("Content-Type: text/plain; charset=utf-8")
    print()  # separates headers from body

    # Basic CGI debug info
    print("=== CGI debug ===")
    print("REQUEST_METHOD  :", os.environ.get("REQUEST_METHOD", ""))
    print("QUERY_STRING    :", os.environ.get("QUERY_STRING", ""))
    print("SCRIPT_NAME     :", os.environ.get("SCRIPT_NAME", ""))
    print("SCRIPT_FILENAME :", os.environ.get("SCRIPT_FILENAME", ""))
    print("REMOTE_ADDR     :", os.environ.get("REMOTE_ADDR", ""))
    print()

    # Print all HTTP headers
    print("--- HTTP_* headers ---")
    for k, v in sorted(os.environ.items()):
        if k.startswith("HTTP_"):
            print(f"{k} = {v}")
    print()

    # Handle POST body
    if os.environ.get("REQUEST_METHOD", "").upper() == "POST":
        body = read_stdin()
        print("--- POST body (raw) ---")
        print(body)
    else:
        print("--- No POST body (method is not POST) ---")

if __name__ == "__main__":
    main()