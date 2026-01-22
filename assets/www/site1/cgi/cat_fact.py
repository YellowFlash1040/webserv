#!/usr/bin/env python3

import random

# List of fun cat facts
CAT_FACTS = [
    "Cats sleep 70% of their lives.",
    "A group of cats is called a clowder.",
    "Cats have five toes on their front paws, but only four on the back.",
    "The oldest cat ever recorded lived 38 years!",
    "Cats can rotate their ears 180 degrees.",
    "A cat’s whiskers are roughly as wide as its body.",
    "Cats can make over 100 different sounds.",
    "Cats can run up to 30 miles per hour."
]

# Pick a random fact
fact = random.choice(CAT_FACTS)

# Output HTTP header
print("Content-Type: text/html")
print()  # blank line required

# Output HTML page
print(f"""
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Random Cat Fact</title>
    <style>
        body {{ font-family: Verdana, sans-serif; background-color: #f0f8ff; text-align: center; padding: 2rem; }}
        .fact {{ font-size: 1.5rem; margin-top: 2rem; color: #004080; }}
        button {{ margin-top: 2rem; padding: 0.5rem 1rem; font-size: 1rem; }}
    </style>
</head>
<body>
    <h1>🐱 Random Cat Fact 🐱</h1>
    <div class="fact">{fact}</div>
    <form method="get">
        <button type="submit">Get another fact</button>
    </form>

    <br/>
    <a href="/">Back to home</a>
</body>
</html>
""")