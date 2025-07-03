# 🕷️ cpider-http-server

A simple, multithreaded HTTP server written in C for serving static files and handling basic POST requests. Built purely for educational exploration of how HTTP servers work under the hood.

- ⚠️ Not production-ready. This project is meant for learning purposes only, designed to help understand systems-level programming, socket handling, and concurrent request processing in C.

## ✨ Usage
To start the cpider static file server, run the executable with the path to the directory you wish to serve.

    ./cpider ./public_html

This will start the server on the default port (typically 8080) with a default number of worker threads (e.g., matching your CPU core count), serving files from the ./public_html directory.

![image](https://github.com/user-attachments/assets/b948150c-43da-40c9-a3e8-2cab8c3224d4)


### Advanced Usage (Optional Arguments):
You can customize the server's behavior using the following optional command-line flags:

- ```-t <num_threads>``` : Specifies the maximum number of worker threads the server should spawn to handle incoming requests.
	- Recommended Value: It's generally advised to set this to 2 times your CPU Cores or or just the number of CPU Cores for I/O-bound tasks. Avoid excessively high numbers, as too many threads can lead to increased context switching overhead. If omitted, the server will default to an appropriate number (e.g., based on the system's CPU core count).
- ```-p <port_number>``` : Specifies the TCP port on which the server will listen for incoming HTTP connections.
	- Default: If omitted, the server will default to port 8080.
	- Note: Ports below 1024 usually require root/administrator privileges.

### Example with all options:

    ./cpider -t12 -p5000 ./my_website_files

This command will start cpider with ***12 worker threads***, listening on ***port 5000***, and serving static files from the ./my_website_files directory.


### 🏗️ Building:
🐧 Linux or WSL Only

This server runs only on Linux or WSL environments. It uses POSIX threads and Linux socket APIs.
⚙️ With Makefile

Clone the repo and run:

    make

Make sure you're inside a Linux shell or WSL terminal with gcc installed and the zlib library which should be pre-installed


### 🚀 Running

    ./cpider ./public


- Server scans the folder:

- Logs warnings for unsupported file types.

- Compressed the files that can be compressed 

- Fails if the folder is empty.

- Will attempt to serve index.html at root (/, /home, /index.html).

### 📨 POST Support

- Only enabled on /contact.

- Accepts form data (Content-Type: application/x-www-form-urlencoded).

- Data is parsed and appended to a local .txt file.

- Returns 201 Created on success.

## ✨ Features

### 🧵 Multithreaded Request Handling with Thread Pool

-    Handles multiple clients concurrently using a custom-built thread pool.

- Efficient under load, ready to process many incoming connections without blocking.

- Each client is processed independently, with logging tagged by client (port-based ID).

### ✅ Supports GET and POST

- **GET**: Serves static files with MIME-compliant headers.
  
- **POST**: Accepts form submissions at /contact. 
   - Parses application/x-www-form-urlencoded bodies and saves to a records.txt file, responding with 201 Created.

### 🧠 Manual HTTP Request Parsing

- Parses the request line and headers manually.

- Rejects malformed requests with 400 Bad Request.

### 🗂️ Static File Serving

- Serves all compatible file types from a user-specified directory.
  
- Automatically appends .html if the path lacks an extension (e.g., /about becomes /about.html).

- Serves fallback homepage if index.html is missing.

- Warns about unsupported file types at startup.

- Large files are read and sent in chunks using multiple write() calls with correct Content-Length.

### 🔄 Persistent Connections (Keep-Alive)

- Supports multiple requests per connection via Keep-Alive.

- Connection closes after timeout or reaching max request limit per socket.

- Each client’s request lifecycle is traced with its own ID (port-based).

### 🗜️ Gzip Compression (Accept-Encoding)
- Automatic Pre-compression: cpider scans the served directory and automatically creates Gzip-compressed versions of static files.

- Dedicated Storage: Compressed files are stored in a compressed/ folder within the served directory.
  
- Smart Serving:
	- If a client supports Gzip (Accept-Encoding: gzip) which is most browsers these days, cpider serves the pre-compressed .gz file.
	- Otherwise, the original uncompressed file is served.

- Benefits: Reduces bandwidth, speeds up loading times, and improves server performance by offloading on-the-fly compression.

### 📜 Color-Coded Logging

- Logs are colorized for clarity:

![image](https://github.com/user-attachments/assets/20ba9705-f02d-4027-b6ab-1961307b5551)

### 🧪 Designed for Learning, Not Deployment

This server is not secure, not optimized, and not robust enough for real-world deployment.

It exists solely as a learning experience to explore:

Networking with C sockets

1. HTTP parsing and protocol handling
2. File I/O
3. Multithreaded design using a thread pool
4. Lightweight logging and debugging patterns



### 📄 License

MIT

Let me know if you'd like a section for "Example Requests" or "Folder Structure" — happy to generate that too.
