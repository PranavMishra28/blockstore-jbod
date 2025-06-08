# Technical Documentation

## JBOD Client-Server Implementation Details

### Network Protocol Specification

#### Packet Structure
- **Header (8 bytes)**:
  - Length field (2 bytes): Total packet length including header and data
  - Operation code (4 bytes): The command to be executed (network byte order)
  - Return code (2 bytes): The result of the operation (network byte order)
- **Data block (optional, 256 bytes)**:
  - Only present for read responses and write requests

#### Byte Order
- All multi-byte fields are transmitted in network byte order (big-endian)
- Client must convert to/from host byte order using `htonl`/`ntohl` and `htons`/`ntohs`

### Core Functions Implementation Details

#### Network Communication Layer (`net.c/h`)

##### `bool nread(int fd, int len, uint8_t *buf)`
- Purpose: Read exactly `len` bytes from file descriptor `fd` into buffer `buf`
- Implementation:
  - Uses a loop to handle partial reads
  - Returns `true` when all bytes are read, `false` on error
  - Handles interrupted system calls (EINTR)

##### `bool nwrite(int fd, int len, uint8_t *buf)`
- Purpose: Write exactly `len` bytes from buffer `buf` to file descriptor `fd`
- Implementation:
  - Uses a loop to handle partial writes
  - Returns `true` when all bytes are written, `false` on error
  - Handles interrupted system calls (EINTR)

##### `bool send_packet(int sd, uint32_t op, uint8_t *block)`
- Purpose: Construct and send a packet to the server
- Packet Construction:
  - Determine packet length based on operation type
  - Convert values to network byte order
  - Assemble header and data (if applicable) into packet
- Implementation:
  - For write operations: include data block in packet
  - For other operations: header only
  - Uses `nwrite` to ensure complete transmission

##### `bool recv_packet(int fd, uint32_t *op, uint16_t *ret, uint8_t *block)`
- Purpose: Receive and unpack a packet from the server
- Implementation:
  - First reads header (8 bytes) using `nread`
  - Extracts length, operation and return code
  - Converts from network to host byte order
  - If packet length indicates data block present, reads additional 256 bytes

#### JBOD Client Operations (`mdadm.c/h`)

##### `bool jbod_connect(const char *ip, uint16_t port)`
- Purpose: Establish a connection to the JBOD server
- Implementation:
  - Creates a socket using `socket()`
  - Connects to server using `connect()`
  - Sets global `cli_sd` to socket descriptor on success

##### `void jbod_disconnect(void)`
- Purpose: Close the connection to the JBOD server
- Implementation:
  - Closes the socket using `close()`
  - Resets `cli_sd` to -1

##### `int jbod_client_operation(uint32_t op, uint8_t *block)`
- Purpose: Send operation to server and process response
- Implementation:
  - Sends operation using `send_packet()`
  - Receives response using `recv_packet()`
  - Returns the response code

### Global Variables

- `cli_sd`: Integer holding the client socket descriptor
  - Initialized to -1
  - Set to valid socket descriptor when connected
  - Reset to -1 when disconnected

### Error Handling

- Network failures: Functions return `false` if network operations fail
- Invalid operations: Server returns error codes to indicate invalid operations
- Connection errors: `jbod_connect()` returns `false` if connection fails

### Performance Considerations

- The protocol includes minimal overhead (8-byte header)
- Fixed-size data blocks (256 bytes) optimize for common use cases
- Error handling designed to be robust against network interruptions

### Future Enhancements

1. **Security**: Add authentication and encryption to protect data in transit
2. **Multiple Clients**: Enhance server to handle multiple concurrent clients
3. **Asynchronous I/O**: Implement non-blocking operations for improved performance
4. **Extended Commands**: Add additional JBOD management operations
5. **Improved Caching**: Develop more sophisticated caching algorithms
