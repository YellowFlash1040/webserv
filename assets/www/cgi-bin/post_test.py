#!/usr/bin/env python3
import os
import sys

length = int(os.environ.get("CONTENT_LENGTH", "0"))
body = sys.stdin.read(length)

print("Content-Type: text/plain\r")
print("\r")
print("POST BODY:")
print(body)

print("\nENV:")
for k in ["REQUEST_METHOD", "CONTENT_LENGTH", "CONTENT_TYPE", "QUERY_STRING"]:
    print(f"{k}={os.environ.get(k)}")
