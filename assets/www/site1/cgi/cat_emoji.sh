#!/bin/bash

# HTTP header
echo "Content-Type: text/html"
echo ""

# Array of cat emojis
cats=("😺" "😸" "😹" "😻" "😼" "😽" "🙀" "😿" "🐱")

# Pick a random cat
RANDOM_INDEX=$(( RANDOM % ${#cats[@]} ))
cat_emoji=${cats[$RANDOM_INDEX]}

# HTML output
cat <<HTML
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<title>Random Cat Emoji</title>
<style>
body { font-family: Verdana, sans-serif; text-align: center; padding: 2rem; background: #fff3e0; color: #ff6f00; }
button { padding: 0.5rem 1rem; font-size: 1rem; margin-top: 1rem; cursor: pointer; }
</style>
</head>
<body>
<h1>Random Cat Emoji 🐾</h1>
<p>Here's your random cat: <strong>$cat_emoji</strong></p>
<form method="get">
    <button type="submit">Generate Another</button>
</form>
<br/>
<a href="/">Back to home</a>
</body>
</html>
HTML
