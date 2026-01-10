#!/usr/bin/env python3
import warnings
warnings.filterwarnings("ignore", category=DeprecationWarning)

import cgitb
import os
import sys

cgitb.enable()

print("Content-Type: text/html")
print()  # End of headers

# Read the raw POST body
content_length = int(os.environ.get("CONTENT_LENGTH", 0))
post_data = sys.stdin.read(content_length) if content_length > 0 else ""

print("<html>")
print("<head><title>CGI POST Test</title></head>")
print("<body>")
print("<h1>POST Data Received</h1>")

if not post_data:
    print("<p>No POST data received.</p>")
else:
    print("<pre>")
    print(post_data)
    print("</pre>")

print("</body>")
print("</html>")
