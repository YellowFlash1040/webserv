#!/usr/bin/env python3
import warnings
warnings.filterwarnings("ignore", category=DeprecationWarning)

import cgi
import time

# Required HTTP headers
print("Content-Type: text/html\r\n\r\n")
# time.sleep(10)

# Retrieve form data
form = cgi.FieldStorage()
name = form.getvalue("name")
age = form.getvalue("age")

# sleep(10)  # Simulate a long processing time

# Generate the HTML content with the user's information
html_content = f"""
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>User Information</title>
    <style>
        @import url('https://fonts.googleapis.com/css2?family=Inter:ital,opsz,wght@0,14..32,100..900;1,14..32,100..900&display=swap');

        body {{
            font-family: 'Inter', sans-serif;
            background-color: #f0f0f0;
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
            color: #333;
        }}
        .container {{
            text-align: center;
        }}
    </style>
</head>
<body>
    <div class="container">
        <h1>Welcome, {name}!</h1>
        <p>It's cool to be {age}, you know :)</p>
    </div>
</body>
</html>
"""

print(html_content)
