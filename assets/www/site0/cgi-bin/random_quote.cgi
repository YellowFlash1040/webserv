#!/usr/bin/perl

use strict;
use warnings;

# Required CGI header
print "Content-Type: text/html\n\n";

# List of quotes
my @quotes = (
    "Talk is cheap. Show me the code. Linus Torvalds",
    "Programs must be written for people to read. Harold Abelson",
    "First, solve the problem. Then, write the code. John Johnson",
    "Simplicity is the soul of efficiency. Austin Freeman",
    "Any sufficiently advanced technology is indistinguishable from magic. Arthur C. Clarke",
);

# Pick a random quote
my $quote = $quotes[ int(rand(@quotes)) ];

# Output HTML
print <<"HTML";
<!DOCTYPE html>
<html>
<head>
    <title>Random Quote</title>
</head>
<body>
    <h1>Random Quote</h1>
    <p>$quote</p>
</body>
</html>
HTML
