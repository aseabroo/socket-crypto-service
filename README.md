# Socket Crypto Service

A concurrent TCP client/server project in C that demonstrates socket programming, process-based concurrency, framed messages, input validation, and a reversible text transformation.

> **Important:** this project is an educational networking/system-programming demo. The cipher is intentionally simple and is **not suitable for real-world security or protecting sensitive data**.

## What it demonstrates

- TCP sockets with `socket()`, `bind()`, `listen()`, `accept()`, and `connect()`
- process-per-connection concurrency with `fork()`
- explicit message framing over a byte stream
- robust partial read/write handling
- client/server protocol design
- reversible text transformation over a constrained alphabet
- input validation
- deterministic round-trip tests
- secure random key generation from the operating system

## Why this project exists

This is an independent portfolio implementation of networking and systems concepts I previously practiced in coursework. It is not a republished assignment solution. The protocol, source layout, build system, tests, and documentation were rebuilt for this repository.

## Architecture

```text
plaintext + key
      |
      v
 crypto-client  ------ TCP ------>  crypto-server
      ^                              |
      |                              v
      +------ transformed text <-----+
```

The server supports two operations:

- `ENC` — transform plaintext into ciphertext
- `DEC` — reverse ciphertext back into plaintext

A single server handles both operation types.

## Protocol

Each request uses a small binary frame:

```text
+------------+----------------+----------------+-------------------+
| op (1 B)   | text len (4 B) | key len (4 B)  | text bytes + key  |
+------------+----------------+----------------+-------------------+
```

Lengths are transmitted in network byte order. The server validates frame sizes before allocating memory.

The response contains:

```text
+----------------+-------------------+
| data len (4 B) | response bytes    |
+----------------+-------------------+
```

## Alphabet and transformation

The demo alphabet is:

```text
A-Z and space
```

Characters map to integers `0..26`.

Encryption:

```text
cipher = (plain + key) mod 27
```

Decryption:

```text
plain = (cipher - key + 27) mod 27
```

This is intentionally simple so the networking behavior remains the focus.

## Build

```bash
make
```

Executables are written to `bin/`:

```text
bin/crypto-server
bin/crypto-client
bin/keygen
```

## Run

Start the server:

```bash
./bin/crypto-server 5151
```

Generate a key:

```bash
./bin/keygen 20
```

Encrypt:

```bash
./bin/crypto-client enc 127.0.0.1 5151 "HELLO WORLD" "XMCKL QWERTY"
```

Decrypt:

```bash
./bin/crypto-client dec 127.0.0.1 5151 "<CIPHERTEXT>" "XMCKL QWERTY"
```

## Test

```bash
make test
```

The integration test starts a local server, performs an encrypt/decrypt round trip, and confirms the recovered plaintext matches the original.

## Project Structure

```text
socket-crypto-service/
├── include/
│   ├── common.h
│   └── protocol.h
├── src/
│   ├── client.c
│   ├── common.c
│   ├── keygen.c
│   ├── protocol.c
│   └── server.c
├── tests/
│   └── integration.sh
├── .github/workflows/test.yml
├── .gitignore
├── Makefile
└── README.md
```

## Limitations

This project intentionally does not implement TLS, authentication, integrity protection, key exchange, or modern cryptography. Those are separate security concerns and should be handled by established libraries and protocols in production systems.
