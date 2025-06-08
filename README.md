# JBOD Client-Server Storage System

## Overview
This project implements a client-server system for a JBOD (Just a Bunch of Disks) storage configuration. The client communicates with the server to perform operations on the storage system, such as reading from and writing to data blocks across multiple disks.

## Architecture

### Components
- **Client**: Connects to the server, sends operation requests, and processes responses
- **Server**: Manages the JBOD storage system and executes operations requested by clients
- **JBOD**: Storage system consisting of multiple disks organized linearly

### Network Protocol
The client and server communicate using a custom binary protocol with the following packet structure:

1. **Header** (8 bytes):
   - Length field (2 bytes): Total packet length in bytes
   - Operation code (4 bytes): Command to execute on the JBOD
   - Return code (2 bytes): Result of operation (in server responses)

2. **Data block** (optional, 256 bytes):
   - Present only for read/write operations

## Core Functions

### Connection Management
- `jbod_connect(const char *ip, uint16_t port)`: Establishes connection to the server
- `jbod_disconnect(void)`: Closes the active server connection

### JBOD Operations
- `jbod_client_operation(uint32_t op, uint8_t *block)`: Sends commands to server and processes responses

### Network Utilities
- `nread(int fd, int len, uint8_t *buf)`: Reliably reads `len` bytes from file descriptor
- `nwrite(int fd, int len, uint8_t *buf)`: Reliably writes `len` bytes to file descriptor
- `recv_packet(int fd, uint32_t *op, uint16_t *ret, uint8_t *block)`: Receives and unpacks a packet
- `send_packet(int sd, uint32_t op, uint8_t *block)`: Constructs and sends a packet

## Packet Flow

1. **Client connects** to server via `jbod_connect()`
2. **Client sends operation** using `jbod_client_operation()`
   - Operation is packaged via `send_packet()`
   - Packet is sent to server
3. **Server processes** the operation on the JBOD storage
4. **Server responds** with result code (and data if applicable)
5. **Client receives response** via `recv_packet()`
6. **Client disconnects** using `jbod_disconnect()` when finished

## Cache System
The system includes an optional cache implementation that improves performance by storing recently accessed blocks, reducing the need for network communication.

## Building and Running

### Prerequisites
- C compiler (gcc recommended)
- POSIX-compliant operating system
- Make build system

### Compilation
```bash
make all
```

### Running the Server
```bash
./jbod_server
```

### Running Tests
```bash
./tester
```

## Project Structure
```
.
├── cache.c/h      # Cache implementation
├── jbod.h         # JBOD interface definitions
├── mdadm.c/h      # JBOD client operations
├── net.c/h        # Network communication layer
├── tester.c/h     # Test suite
├── util.c/h       # Utility functions
└── traces/        # Test trace files
```

## Academic Context
This project was developed as part of CMPSC 311 (Assignments 1-5), focusing on systems programming concepts, including:

- Client-server architecture
- Network programming and socket communications
- Storage systems
- Caching mechanisms
- Reliable data transfer protocols

## Authors
- [Your Name]

## License
See the LICENSE file for details.
