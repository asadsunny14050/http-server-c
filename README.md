🕷️ cpider-http-server

A simple, multithreaded HTTP server written in C for serving static files and handling basic POST requests. Built purely for educational exploration of how HTTP servers work under the hood.

    ⚠️ Not production-ready. This project is meant for learning purposes only, designed to help understand systems-level programming, socket handling, and concurrent request processing in C.

✨ Features

    🧵 Multithreaded Request Handling with Thread Pool

        Handles multiple clients concurrently using a custom-built thread pool.

        Efficient under load — ready to process many incoming connections without blocking.

        Each client is processed independently, with logging tagged by client (port-based ID).

    ✅ Supports GET and POST

        GET: Serves static files with MIME-compliant headers.

        POST: Accepts form submissions at /contact. Parses application/x-www-form-urlencoded bodies and saves to a .txt file, responding with 201 Created.

    🧠 Manual HTTP Request Parsing

        Parses the request line and headers manually.

        Rejects malformed requests with 400 Bad Request.

    🗂️ Static File Serving

        Serves all compatible file types from a user-specified directory.

        Automatically appends .html if the path lacks an extension (e.g., /about becomes /about.html).

        Serves fallback homepage if index.html is missing.

        Warns about unsupported file types at startup.

        Large files are read and sent in chunks using multiple send() calls with correct Content-Length.

    🔄 Persistent Connections (Keep-Alive)

        Supports multiple requests per connection via Keep-Alive.

        Connection closes after timeout or reaching max request limit per socket.

    📜 Color-Coded Logging

        Logs are colorized for clarity:

            Server setup and lifecycle events

            Request handling steps and debug info

            Warnings and errors

        Each client’s request lifecycle is traced with its own ID (port-based).

🏗️ Building
🐧 Linux or WSL Only

This server runs only on Linux or WSL environments. It uses POSIX threads and Linux socket APIs.
⚙️ With Makefile

Clone the repo and run:

make

    Make sure you're inside a Linux shell or WSL terminal with gcc and make installed.

🚀 Running

./cpider ./public

    Replace ./public with the directory containing your static files.

    Server scans the folder:

        Logs warnings for unsupported file types.

        Fails if the folder is empty.

    Will attempt to serve index.html at root (/, /home, /index.html).

📨 POST Support

    Only enabled on /contact.

    Accepts form data (Content-Type: application/x-www-form-urlencoded).

    Data is parsed and appended to a local .txt file.

    Returns 201 Created on success.

🧪 Designed for Learning, Not Deployment

This server is not secure, not optimized, and not robust enough for real-world deployment.

It exists solely as a learning experience to explore:

    Networking with C sockets

    HTTP parsing and protocol handling

    File I/O

    Multithreaded design using a thread pool

    Lightweight logging and debugging patterns

📄 License

MIT

Let me know if you'd like a section for "Example Requests" or "Folder Structure" — happy to generate that too.
