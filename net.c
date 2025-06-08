#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <err.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include "net.h"
#include "jbod.h"

/* the client socket descriptor for the connection to the server */
int cli_sd = -1;

/* attempts to read n bytes from fd; returns true on success and false on
 * failure */
static bool nread(int fd, int len, uint8_t *buf)
{
  int num_read = 0;
  while(num_read < len){
    int n = read(fd, &buf[num_read], len - num_read);
    if(n < 0){
      return false;
    }
    num_read += n;
  }
  return true;
}

/* attempts to write n bytes to fd; returns true on success and false on
 * failure */
static bool nwrite(int fd, int len, uint8_t *buf)
{
  // Checking the total amount of data written
  int num_write = 0; 
  while(num_write < len){
    int w = write(fd, &buf[num_write], len - num_write);

    // Proof checking if the system call works
    if(w < 0){
      return false;
    }
    num_write += w;
  }

  return true;
}

/* attempts to receive a packet from fd; returns true on success and false on
 * failure */
static bool recv_packet(int fd, uint32_t *op, uint16_t *ret, uint8_t *block){

  // Creating the temp_buf to copy into
  uint8_t temp_buf[HEADER_LEN];
  uint16_t len;

  nread(fd, HEADER_LEN, temp_buf);

  // Converting op to temp_buf line
  memcpy(&len, temp_buf, 2);
  memcpy(op, temp_buf + 2, 4);
  memcpy(ret, temp_buf + 6, 2);

  // Network to host conversion
  *op = ntohl(*op);
  len = ntohs(len);
  *ret = ntohs(*ret);
 
  if(len == HEADER_LEN){
    return true;
  }
  else if(len == 264){
    nread(fd, 256, block);
    return true;
  }
  else{
    return false;
  }

  return true;
}

/* attempts to send a packet to sd; returns true on success and false on
 * failure */
static bool send_packet(int sd, uint32_t op, uint8_t *block)
{
  // Creating a packet
  uint16_t len = HEADER_LEN;

  // Right shift
  uint32_t command = op >> 26;


  // Host to network conversion
  op = htonl(op);

  // Checking command value
  if(command == JBOD_WRITE_BLOCK){
    uint8_t packet[HEADER_LEN + JBOD_BLOCK_SIZE];
    len += JBOD_BLOCK_SIZE;
    len = htons(len);
    memcpy(packet, &len, 2);
    memcpy(packet + 2, &op, 4);
    memcpy(packet + 8, block, 256);
    nwrite(sd, 264, packet);
  }
  else{
    uint8_t packet[HEADER_LEN];
    len = htons(len);
    memcpy(packet, &len, 2);
    memcpy(packet + 2, &op, 4);
    nwrite(sd, 8, packet);
  }
 
  return true;
}

/* attempts to connect to server and set the global cli_sd variable to the
 * socket; returns true if successful and false if not. */
bool jbod_connect(const char *ip, uint16_t port)
{
  /*Steps*/
  struct sockaddr_in caddr;
  caddr.sin_family = AF_INET;
  caddr.sin_port = htons(JBOD_PORT);

  // Convert IP address to binary
  if(inet_aton(ip, &(caddr.sin_addr)) == 0)
  {
    return false;
  }

  // Create socket
  cli_sd = socket(PF_INET, SOCK_STREAM, 0);
  if (cli_sd == -1)
  {
    printf("Error on socket creation [%s]\n", strerror(errno));
    return false;
  }
  // Connect
  if (connect(cli_sd, (struct sockaddr *)&caddr, sizeof(caddr)) == -1)
  {
    return false;
  }

  
  return true;
}

/* disconnects from the server and resets cli_sd */
void jbod_disconnect(void)
{
  close(cli_sd);
  cli_sd = -1;
}

/* sends the JBOD operation to the server and receives and processes the
 * response. */
int jbod_client_operation(uint32_t op, uint8_t *block)
{
  uint16_t ret;
  uint32_t rop;
  if(send_packet(cli_sd, op, block))
  {
    recv_packet (cli_sd, &rop, &ret, block);  
  }
  return ret;
}