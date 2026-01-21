#!/usr/bin/env python3

import os
import sys

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

    print("Content-Type: text/plain\n")

    print("=== CGI Debug Info ===\n")
    
    print("Command-line Arguments (@ARGV)")
    if len(sys.argv) > 1:
        print(" ".join(sys.argv[1:]))
    else:
        print("No command-line arguments.")
    print()

    print("Environment Variables")
    for k, v in sorted(os.environ.items()):
        print(f"{k} = {v}")
    print()

    if os.environ.get("REQUEST_METHOD", "").upper() == "POST":
        body = read_stdin()
        print("--- POST body (raw) ---")
        print(body)
    else:
        print("--- No POST body (method is not POST) ---")

if __name__ == "__main__":
    main()
