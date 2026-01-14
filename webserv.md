# Documentation

## Table of Contents
- [http](#http)
- [server](#server)
- [server_name](#server_name)
- [listen](#listen)
- [error_page](#error_page)
- [client_max_body_size](#client_max_body_size)
- [location](#location)
- [limit_except](#limit_except)
- [return](#return)
- [root](#root)
- [alias](#alias)
- [autoindex](#autoindex)
- [index](#index)
- [upload_store](#upload_store)
- [cgi_pass](#cgi_pass)

### http

Syntax: **http** { ... }  
Default: —  
Context: global  
Multiple allowed: no  
Cascade policy: —

Description:  
Provides the configuration file context in which the HTTP server directives are specified.

Example:

```nginx
http {
    # Other directives
}
```

### server

Syntax: **server** { ... }  
Default: —  
Context: http  
Multiple allowed: yes  
Cascade policy: —

Description:  
Sets configuration for a virtual server.

Example:

```nginx
server {
    # Other directives
}
```

### server_name

Syntax: **server_name** _name_ ...;  
Default: server_name "";  
Context: server  
Multiple allowed: no  
Cascade policy: —

Description:  
Sets names of a virtual server.

Example:

```nginx
server_name example.com www.example.com;
```

### listen

Syntax: **listen** _address_:_port_  
**listen** _address_  
**listen** _port_  
Default: listen 8080;  
Context: server  
Multiple allowed: yes  
Cascade policy: —

Description:  
Sets the *`address`* and *`port`* for an IP socket on which the server will accept requests.  
Both *`address`* and *`port`*, or only *`address`* or only *`port`* can be specified.  
If only address is given, the port 8080 is used.  
If the directive is not present then 0.0.0.0:8080 is used.  
If multiple servers listen on the same IP socket then the server with a matching `server_name` is chosen.  
If none of the `server_name` matches the `host` header - first server specified in the configuration file for this socket is chosen.

Example:

```nginx
listen 127.0.0.1:8080;
listen 127.0.0.1;
listen 8080;
```

### error_page

Syntax: **error_page** _code_ ... _uri_;  
Default: —  
Context: http, server, location  
Multiple allowed: yes  
Cascade policy: merge

Description:  
Defines the URI that will be shown for the specified errors.

Example:

```nginx
error_page 404             /404.html;
error_page 500 502 503 504 /50x.html;
```

### client_max_body_size

Syntax: **client_max_body_size** _size_;  
Default: client_max_body_size 1m;  
Context: http, server, location  
Multiple allowed: no  
Cascade policy: override

Description:  
Sets the maximum allowed size of the client request body.  
If the size in a request exceeds the configured value, the 413 (Request Entity Too Large) error is returned to the client.  
Please be aware that browsers cannot correctly display this error.  
Setting size to 0 disables checking of client request body size.

Example:

```nginx
client_max_body_size 10g;
```

### location

Syntax: **location** _uri_ { ... }  
Default: location / {}  
Context: server  
Multiple allowed: yes  
Cascade policy: —

Description:  
Sets configuration depending on a request URI.

The matching is performed against a normalized URI, after decoding the text encoded in the “%XX” form,  
resolving references to relative path components “.” and “..”,  
and possible compression of two or more adjacent slashes into a single slash.

The location with the longest matching prefix is selected.

Example:

```nginx
location /images/ {
    # Other directives
}
```

### limit_except

Syntax: **limit_except** _method_ ...  
Default: limit_except GET POST;  
Context: location  
Multiple allowed: no  
Cascade policy: —

Description:  
Limits allowed HTTP methods inside a location. The _`method`_ parameter can be one of the following: `GET`, `POST`, `DELETE`.

Example:

```nginx
limit_except GET;
```

### return

Syntax: **return** _code_ _URL_;  
**return** _code_;  
Default: —  
Context: server, location  
Multiple allowed: no  
Cascade policy: non-cascading — the first defined `return` takes effect

Description:  
Stops processing and returns the specified code and URL to a client.  
The code can be only in range [300, 399].

If both the `server` and a `location` block define a `return` directive,  
the first one encountered in the configuration file takes precedence  
(for example, a `return` in the `server` block overrides one in a nested `location`).

Example:

```nginx
return 307;
return 301 https://profile.intra.42.fr;
```

### root

Syntax: **root** _path_;  
Default: root /var/www;  
Context: http, server, location  
Multiple allowed: no  
Cascade policy: override  
Conflicting directives: alias

Description:  
Sets the root directory for requests. For example, in the configuration example below  
the `/data/w3/i/top.gif` file will be sent in response to the `/i/top.gif` request.  
A path to the file is constructed by merely adding a URI to the value of the `root` directive.  
If a URI has to be modified, the `alias` directive should be used.

Example:

```nginx
location /i/ {
    root /data/w3;
}
```

### alias

Syntax: **alias** _path_;  
Default: —  
Context: location  
Multiple allowed: no  
Cascade policy: override  
Conflicting directives: root

Description:  
Defines a replacement for the specified location. In the configuration example below  
on request of `/i/top.gif`, the file `/data/w3/images/top.gif` will be sent.

Example:

```nginx
location /i/ {
    alias /data/w3/images/;
}
```

### autoindex

Syntax: **autoindex** _on_ | _off_;  
Default: autoindex off;  
Context: http, server, location  
Multiple allowed: no  
Cascade policy: override

Description:  
Enables or disables the directory listing output.

Example:

```nginx
autoindex on;
```

### index

Syntax: **index** _file_ ...;  
Default: index index.html;  
Context: http, server, location  
Multiple allowed: no  
Cascade policy: override

Description:  
Defines files that will be used as an index. Files are checked in the specified order.

Example:

```nginx
index index.html;
index index.html index.txt;
```

### upload_store

Syntax: **upload_store** _uri_;  
Default: —  
Context: server, location  
Multiple allowed: no  
Cascade policy: override

Description:  
Defines a folder where uploaded files will be saved to.

Example:

```nginx
upload_store /var/tmp;
```

### cgi_pass

Syntax: **cgi_pass** _extension_ _executor_;  
Default: —  
Context: server, location  
Multiple allowed: yes  
Cascade policy: merge

Description:  
Defines a mapping between  
a file extension  
and the CGI executable that should handle requests for files with that extension.

When a request is processed within a `server` or `location` block containing a `cgi_pass` directive,  
and targets a resource whose URI ends with the specified extension,  
the request is being passed to the corresponding executor through the CGI mechanism.  
The program’s output is then sent back to the client as the response.

This directive can be specified multiple times within a context block to handle different CGI types.

So in short: this directive allows automatic detection and delegation of CGI requests based on file extensions,  
enabling different interpreters or handlers for different script types.

Example:

```nginx
cgi_pass .py /usr/bin/python3;
cgi_pass .php /usr/bin/php-cgi;
```
