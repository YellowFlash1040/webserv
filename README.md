# webserv

An HTTP server inspired by NGINX, written in C++, and designed to run on Linux.

This project is part of the [Codam](https://www.codam.nl/en/) curriculum, [42 School](https://42.fr/en/homepage/) campus.

## Features

- HTTP/1.1 compliant server basics
- GET, POST, and DELETE request handling
- Static file serving
- Config file parsing (NGINX-style syntax)
- Multiple server blocks / virtual hosts
- Non-blocking I/O with `epoll`
- Error pages support
- File upload handling
- Multiple clients support

## Prerequisites

Before building the project, ensure you have:

- GNU Make
- GNU Compiler Collection (GCC / G++)
- POSIX-compliant system (Linux recommended)

On Debian/Ubuntu-based systems, you can install required tools with:

```
sudo apt update && sudo apt install build-essential
```

## Installation

1.  Clone the repository:

```
git clone https://github.com/YellowFlash1040/webserv
cd webserv
```

2.  Build the project:

```
make
```

This will create an executable named `webserv`.

## Usage

Run the server with a configuration file:

```
./webserv config.conf
```

If no config file is provided, a default configuration file with path `./webserv.conf` will be used

## Configuration File

Configuration file documentation, explaining how to write and customize your own configuration file can be found here:

```bash
docs/webserv.md
```

The most minimal configuration file looks like:

```nginx
http { server {} }
```

## Testing:

Launch the server:

```bash
./webserv
```

and visit `localhost:8080` in your browser.

## Project Goals

This project aims to deepen understanding of:

- Network programming (sockets, TCP/IP)
- HTTP protocol internals
- Event-driven architecture
- File descriptor management
- Memory and resource handling in C++

## Notes

- Only standard C++ is used (no external frameworks)
- The server is designed for learning purposes, not production use
