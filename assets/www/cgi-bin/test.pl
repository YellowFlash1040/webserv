#!/usr/bin/perl
use strict;
use warnings;

# Print required HTTP header
print "Content-Type: text/html\n\n";

# Start HTML
print "<html><head><title>Perl CGI Debug</title></head><body>\n";
print "<h1>Perl CGI Debug Info</h1>\n";

# Print command-line arguments
print "<h2>Command-line Arguments (\@ARGV)</h2>\n";
if (@ARGV) {
    print "<ul>\n";
    foreach my $arg (@ARGV) {
        print "<li>$arg</li>\n";
    }
    print "</ul>\n";
} else {
    print "<p>No command-line arguments.</p>\n";
}

# Print environment variables
print "<h2>Environment Variables</h2>\n";
print "<ul>\n";
foreach my $key (sort keys %ENV) {
    my $val = $ENV{$key} // '';
    $val =~ s/</&lt;/g;  # escape HTML
    $val =~ s/>/&gt;/g;
    print "<li><b>$key</b> = $val</li>\n";
}
print "</ul>\n";

# Print GET or POST data
print "<h2>Input Data</h2>\n";

my $input_data = '';
if ($ENV{'REQUEST_METHOD'} eq 'POST') {
    my $len = $ENV{'CONTENT_LENGTH'} || 0;
    read(STDIN, $input_data, $len) if $len;
} elsif ($ENV{'REQUEST_METHOD'} eq 'GET') {
    $input_data = $ENV{'QUERY_STRING'} // '';
}

$input_data =~ s/&/&amp;/g;
$input_data =~ s/</&lt;/g;
$input_data =~ s/>/&gt;/g;

print "<pre>$input_data</pre>\n";

# End HTML
print "</body></html>\n";
