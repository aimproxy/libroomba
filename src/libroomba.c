#include "commum.h"

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
