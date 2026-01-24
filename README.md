# webserv

## About

**Webserv** is an HTTP server inspired by NGINX, written in C++, and designed to run on Linux.

## Try it out:
```bash
git clone http://github.com/YellowFlash1040/webserv
cd webserv
make
./webserv
```

and visit `localhost:8080` in your browser.

## Documentation

Configuration file documentation, explaining how to write and customize your own configuration file can be found here:

```bash
docs/webserv.md
```

The most minimal configuration file looks like:

```nginx
http { server {} }
```
