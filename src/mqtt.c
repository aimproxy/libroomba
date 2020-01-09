#include "libroomba.h"

/*
 * Local Functions
 */
int mqtt_tls_cb(MqttClient* client) {
  int rc = WOLFSSL_SUCCESS;

  client->tls.ctx = wolfSSL_CTX_new(wolfTLSv1_2_client_method());
  if (!client->tls.ctx) {
    return WOLFSSL_FAILURE;
  }

  const char *CIPHER_LIST = "AES128-SHA256";
  wolfSSL_CTX_set_cipher_list(client->tls.ctx, CIPHER_LIST);

  wolfSSL_CTX_set_verify(client->tls.ctx, WOLFSSL_VERIFY_PEER,
                         always_true_callback);

  return rc;
}

int mqtt_message_cb(MqttClient *client, MqttMessage *msg,
    byte msg_new, byte msg_done)
{
  byte buf[PRINT_BUFFER_SIZE+1];
  word32 len;

  (void)client;

  if (msg_new) {
    /* Determine min size to dump */
    len = msg->topic_name_len;
    if (len > PRINT_BUFFER_SIZE) {
      len = PRINT_BUFFER_SIZE;
    }
    XMEMCPY(buf, msg->topic_name, len);
    buf[len] = '\0'; /* Make sure its null terminated */

    /* Print incoming message */
    PRINTF("MQTT Message: Topic %s, Qos %d, Len %u",
          buf, msg->qos, msg->total_len);
    }

    /* Print message payload */
    len = msg->buffer_len;
    if (len > PRINT_BUFFER_SIZE) {
        len = PRINT_BUFFER_SIZE;
    }

    XMEMCPY(buf, msg->buffer, len);
    buf[len] = '\0'; /* Make sure its null terminated */
    PRINTF("Payload (%d - %d): %s",
        msg->buffer_pos, msg->buffer_pos + len, buf);

    if (msg_done) {
        PRINTF("MQTT Message: Done");
    }

    /* Return negative to terminate publish processing */
    return MQTT_CODE_SUCCESS;
}

void setup_timeout(struct timeval* tv, int timeout_ms)
{
  tv->tv_sec = timeout_ms / 1000;
  tv->tv_usec = (timeout_ms % 1000) * 1000;

  /* Make sure there is a minimum value specified */
  if (tv->tv_sec < 0 || (tv->tv_sec == 0 && tv->tv_usec <= 0)) {
    tv->tv_sec = 0;
    tv->tv_usec = 100;
  }
}

int socket_get_error(int sockFd)
{
  int so_error = 0;
  socklen_t len = sizeof(so_error);
  getsockopt(sockFd, SOL_SOCKET, SO_ERROR, &so_error, &len);

  return so_error;
}

int mqtt_net_connect(void *context, const char* host, word16 port,
    int timeout_ms)
{
  int rc;
  int sockFd, *pSockFd = (int*)context;
  struct sockaddr_in addr;
  struct addrinfo *result = NULL;
  struct addrinfo hints;

  if (pSockFd == NULL) {
    return MQTT_CODE_ERROR_BAD_ARG;
  }

  (void)timeout_ms;

  /* get address */
  XMEMSET(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;

  XMEMSET(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;

  rc = getaddrinfo(host, NULL, &hints, &result);
  if (rc >= 0 && result != NULL) {
    struct addrinfo* res = result;

    /* prefer ip4 addresses */
    while (res) {
      if (res->ai_family == AF_INET) {
        result = res;
        break;
      }
      res = res->ai_next;
    }

    if (result->ai_family == AF_INET) {
      addr.sin_port = htons(port);
      addr.sin_family = AF_INET;
      addr.sin_addr = ((struct sockaddr_in*)(result->ai_addr))->sin_addr;
    }
    else {
      rc = -1;
    }
    freeaddrinfo(result);
  }

  if (rc < 0) {
    return MQTT_CODE_ERROR_NETWORK;
  }

  sockFd = socket(addr.sin_family, SOCK_STREAM, 0);
  if (sockFd < 0) {
    return MQTT_CODE_ERROR_NETWORK;
  }

  /* Start connect */
  rc = connect(sockFd, (struct sockaddr*)&addr, sizeof(addr));
  if (rc < 0) {
    PRINTF("NetConnect: Error %d (Sock Err %d)",
          rc, socket_get_error(*pSockFd));

    close(sockFd);
    return MQTT_CODE_ERROR_NETWORK;
  }

  /* save socket number to context */
  *pSockFd = sockFd;

  return MQTT_CODE_SUCCESS;
}

int mqtt_net_read(void *context, byte* buf, int buf_len, int timeout_ms)
{
  int rc;
  int *pSockFd = (int*)context;
  int bytes = 0;
  struct timeval tv;

  if (pSockFd == NULL) {
    return MQTT_CODE_ERROR_BAD_ARG;
  }

  /* Setup timeout */
  setup_timeout(&tv, timeout_ms);
  setsockopt(*pSockFd, SOL_SOCKET, SO_SNDTIMEO, (char *)&tv, sizeof(tv));

  /* Loop until buf_len has been read, error or timeout */
  while (bytes < buf_len) {
    rc = (int)recv(*pSockFd, &buf[bytes], buf_len - bytes, 0);
    if (rc < 0) {
      rc = socket_get_error(*pSockFd);
      PRINTF("NetRead: Error %d", rc);

      if (rc == 0) break; /* timeout */

      return MQTT_CODE_ERROR_NETWORK;
    }

    bytes += rc; /* Data */
  }

  if (bytes == 0) {
    return MQTT_CODE_ERROR_TIMEOUT;
  }

  return bytes;
}

int mqtt_net_write(void *context, const byte* buf, int buf_len,
    int timeout_ms)
{
  int rc;
  int *pSockFd = (int*)context;
  struct timeval tv;

  if (pSockFd == NULL) {
    return MQTT_CODE_ERROR_BAD_ARG;
  }

  /* Setup timeout */
  setup_timeout(&tv, timeout_ms);
  setsockopt(*pSockFd, SOL_SOCKET, SO_SNDTIMEO, (char *)&tv, sizeof(tv));

  rc = (int)send(*pSockFd, buf, buf_len, 0);
  if (rc < 0) {
    PRINTF("NetWrite: Error %d (Sock Err %d)",
          rc, socket_get_error(*pSockFd));

    return MQTT_CODE_ERROR_NETWORK;
  }

  return rc;
}

int mqtt_net_disconnect(void *context)
{
  int *pSockFd = (int*)context;

  if (pSockFd == NULL) {
    return MQTT_CODE_ERROR_BAD_ARG;
  }

  close(*pSockFd);
  *pSockFd = INVALID_SOCKET_FD;

  return MQTT_CODE_SUCCESS;
}

word16 mqtt_get_packetid(void)
{
  /* Check rollover */
  if (mPacketIdLast >= MAX_PACKET_ID) {
    mPacketIdLast = 0;
  }

  return ++mPacketIdLast;
}
