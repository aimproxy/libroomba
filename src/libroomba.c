#include "libroomba.h"

void error(const char *msg) { perror(msg); exit(EXIT_FAILURE); }

/*
 * Discovery your Roomba through the network
 *
 * This function sends a probe to the broadcast until find a roomba
 */
void discovery()
{
  int sockfd, one = 1;
  struct timeval timo;
  struct sockaddr_in bcast, from;

  memset(&bcast, 0, sizeof (bcast));
  bcast.sin_family = AF_INET;
  bcast.sin_port = htons(DISCOVERY_PORT);
  bcast.sin_addr.s_addr = inet_addr("192.168.1.255");

  memset(&from, 0, sizeof (from));
  from.sin_family = AF_INET;
  from.sin_port = htons(DISCOVERY_PORT);
  from.sin_addr.s_addr = inet_addr("192.168.1.0");

  if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    error("socket");

  timo.tv_sec  = 3;
  timo.tv_usec = 0;
  if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timo, sizeof (timo)) == -1)
    error("setsockopt RCVTIMEO");

  if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &one, sizeof (one)) == -1)
    error("setsockopt BROADCAST");

  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &one, sizeof (one)) == -1)
    error("setsockopt REUSEPORT");

  for (;;) {
    const char probe[] = "irobotmcs";
    ssize_t n;
    int replies = 0;

    n = sendto(sockfd, probe, strlen(probe), 0, (struct sockaddr *) &bcast, sizeof (bcast));
    if (n != strlen(probe)) {
      if (n != -1)
        error("short write");
      error("sendto");
    }

    for (;;) {
      struct sockaddr_in peer;
      socklen_t peer_len = sizeof (peer);
      char reply[2048];

      n = recvfrom(sockfd, reply, sizeof (reply), 0, (struct sockaddr *) &peer, &peer_len);
      if (n == -1) {
        if (errno == EWOULDBLOCK)
          break;
        error("recvfrom");
      }

      if (peer.sin_addr.s_addr == from.sin_addr.s_addr)
        continue;

      printf("reply from %s:%d\n",
      inet_ntoa(peer.sin_addr), ntohs(peer.sin_port));
      ++replies;
    }
    if (!replies)
      printf("it's quiet\n");
  }
}

/*
 * Get Robot Info
 *
 * This function gives you the basic information about the robot,
 * such as firmaware version, hostname, the ip, and the blid.
 */
void getRobotInfo(const char* ip)
{
  struct json_object *jobj;
  struct sockaddr_in server_addr;
  int sockfd, perm = 1;
  socklen_t slen;
  char buf[BUFSIZE];

  /*
   * Create a datagram socket in the internet domain and use the
   * default protocol (UDP).
   */
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    error("socket() failed\n");

  // Fill the structure
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(DISCOVERY_PORT);

  if (inet_aton(ip , &server_addr.sin_addr) == 0)
    error("inet_aton");

  // Enabling Broadcast
  if(setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &perm, sizeof(int)) == -1)
    error("setsockopt() failed\n");

  // Fill the buffer
  strcpy(buf, "irobotmcs");
  slen = sizeof server_addr;

  // Send the message in buf to the server
  if (sendto(sockfd, buf, BUFSIZE, 0, (struct sockaddr *)&server_addr, slen) == -1)
    error("sendto()");

  // Receive the reply from the server
  if (recvfrom(sockfd, buf, BUFSIZE, 0, (struct sockaddr *)&server_addr, &slen) == -1)
    error("recvfrom()");

  // Treat the Json Object
	jobj = json_tokener_parse(buf);
  puts(json_object_to_json_string_ext(jobj, JSON_C_TO_STRING_SPACED | JSON_C_TO_STRING_PRETTY));

  // Deallocate the socket
  close(sockfd);
}

/*
 * wolfSSL verification callback
 *
 * This function is called when peer certificate verification
 * fails. Returning "1" from this function will
 * allow the SSL/TLS handshake to continue as if verification
 * succeeded.
 */
int always_true_callback(int preverify, WOLFSSL_X509_STORE_CTX* store)
{
    (void)preverify;
    return 1;
}

/*
 * Get Roomba password
 *
 * This method will only work correctly if you have triggered
 * wifi mode by holding the HOME button for several seconds
 * until the roomba beeps.
 */
int getPassword(const char* host)
{
  // Socket Structure
  int                sockfd;
  struct sockaddr_in servAddr;

  // declare wolfSSL objects
  WOLFSSL_CTX* ctx;
  WOLFSSL*     ssl;

  // Initialize wolfSSL
  wolfSSL_Init();

  /*
   * Create a socket that uses an internet IPv4 address,
   * Sets the socket to be stream based (TCP),
   * 0 means choose the default protocol.
   */
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    fprintf(stderr, "ERROR: failed to create the socket\n");
    return -1;
  }

  // Create and initialize WOLFSSL_CTX
  if ((ctx = wolfSSL_CTX_new(wolfTLSv1_2_client_method())) == NULL) {
    fprintf(stderr, "ERROR: failed to create WOLFSSL_CTX\n");
    return -1;
  }

  // Initialize the server address struct with zeros
  memset(&servAddr, 0, sizeof(servAddr));

  // Fill in the server address
  servAddr.sin_family = AF_INET;             // using IPv4
  servAddr.sin_port   = htons(DISCOVERY_PORT); // on DEFAULT_PORT

  // Get the server IPv4 address from the command line call
  if (inet_pton(AF_INET, host, &servAddr.sin_addr) != 1) {
      fprintf(stderr, "ERROR: invalid address\n");
      return -1;
  }

  // Connect to the server
  if (connect(sockfd, (struct sockaddr*) &servAddr, sizeof(servAddr)) == -1) {
    fprintf(stderr, "ERROR: failed to connect\n");
    return -1;
  }

  // No validate peer cert
  wolfSSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, always_true_callback);

  // Create a WOLFSSL object
  if ((ssl = wolfSSL_new(ctx)) == NULL) {
    fprintf(stderr, "ERROR: failed to create WOLFSSL object\n");
    return -1;
  }

  // Set cipher suite
  const char *CIPHER_LIST = "AES128-SHA256";
  wolfSSL_set_cipher_list(ssl, CIPHER_LIST);

  // Attach wolfSSL to the socket
  wolfSSL_set_fd(ssl, sockfd);

  // Connect to wolfSSL on the server side
  if (wolfSSL_connect(ssl) != SSL_SUCCESS) {
    fprintf(stderr, "ERROR: failed to connect to wolfSSL\n");
    return -1;
  }

  /*
   * Get a message for the server
   * [0]	240	 byte   0xf0  MQTT
   * [1]	5    byte   0x05  Message Length
   * [2]	239  byte   0xef
   * [3]	204	 byte   0xcc
   * [4]	59	 byte   0x3b
   * [5]	41	 byte   0x29
   * [6]	0	   byte   0x00 - Based on errors returned, this seems like its a response flag, where 0x00 is OK, and 0x03 is ERROR
   */
  byte hex_packet[] =  { 0xf0, 0x05, 0xef, 0xcc, 0x3b, 0x29, 0x00 };

  // Send the message to the server
  if (wolfSSL_write(ssl, hex_packet, sizeof(hex_packet)) != sizeof(hex_packet)) {
    fprintf(stderr, "ERROR: failed to write\n");
    return -1;
  }

  /*
   * NOTE data is 0xf0 (mqtt RESERVED) length (0x23 = 35),
   * 0xefcc3b2900 (magic packet), 0xXXXX... (30 bytes of
   * password). so 7 bytes, followed by 30 bytes of password
   * (total of 37)
   */
  byte buff[37];
  memset(buff, '\0', sizeof(buff));

  // Read the server data into our buff array
  while (1) {
    int ret = 0;
    if ((ret = wolfSSL_read(ssl, buff, sizeof(buff)-1)) == -1) {
      fprintf(stderr, "ERROR: failed to read\n");
      return -1;
    }

    /*
     *  [0]	240	byte 0xf0 MQTT
     *  [1]	35	byte 0x35 Len
     *  The message length includes the original 5 bytes we sent to it.
     */
    if (ret == 2) {
      continue;
    } else if (ret <= 7) {
      fprintf(stderr, "ERROR: Failed to retrieve password. Did you hold the home button until it beeped?\n");
      return -1;
    } else if (ret > 8) {
      byte psw[30];
      memset(&psw, '\0', sizeof(psw));
      memcpy(&psw, buff+5, sizeof(psw));

      // Get result in UTF-8
      for (int i = 0; i < sizeof(psw); i++) {
        printf("%c", psw[i]);
      }
      break;
    }
  }

  // Cleanup and return
  wolfSSL_free(ssl);      // Free the wolfSSL object
  wolfSSL_CTX_free(ctx);  // Free the wolfSSL context object
  wolfSSL_Cleanup();      // Cleanup the wolfSSL environment
  close(sockfd);          // Close the connection to the server
  return 0;               // Return reporting a success
}
