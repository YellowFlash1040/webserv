#!/usr/bin/env python3

import os
import sys
import html

# Print HTTP header
print("Content-Type: text/html\n")

print("<html><head><title>CGI Debug</title></head><body>")
print("<h1>CGI Debug Information</h1>")

# Command-line arguments
print("<h2>Command-line Arguments (sys.argv)</h2>")
if len(sys.argv) > 1:
    print("<ul>")
    for arg in sys.argv[1:]:
        print(f"<li>{html.escape(arg)}</li>")
    print("</ul>")
else:
    print("<p>No command-line arguments.</p>")

# Environment variables
print("<h2>Environment Variables</h2>")
print("<ul>")
for key in sorted(os.environ.keys()):
    val = os.environ[key]
    print(f"<li><b>{html.escape(key)}</b> = {html.escape(val)}</li>")
print("</ul>")

print("</body></html>")
