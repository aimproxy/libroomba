#ifndef __COMMUM_H__
#define __COMMUM_H__

// the usual suspects
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

// socket includes
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

// Json
#include <json.h>

// wolfSSL
#include <wolfssl/options.h>
#include <wolfssl/ssl.h>

// wolfMQTT
#include <wolfmqtt/mqtt_client.h>

#define TLS_PORT                8883
#define DISCOVERY_PORT          5678
#define BUFSIZE                 1024
#define PRINT_BUFFER_SIZE       500
#define MQTT_CMD_TIMEOUT_MS     30000
#define MQTT_CON_TIMEOUT_MS     5000
#define INVALID_SOCKET_FD       -1
#define MQTT_MAX_PACKET_SZ      1024
#define MAX_PACKET_ID           ((1 << 16) - 1)

MqttClient mClient;
MqttNet mNetwork;
MqttObject mqttObj;

static int mSockFd = INVALID_SOCKET_FD;
static byte mSendBuf[MQTT_MAX_PACKET_SZ];
static byte mReadBuf[MQTT_MAX_PACKET_SZ];
static volatile word16 mPacketIdLast;

// Exposed Functions
void discovery();
void getRobotInfo(const char* host);
int getPassword(const char* host);
int initRoomba(const char* host, const char* username, const char* password);
int sendCommand(const char* topic, const char* publish_msg);

// Internal Functions
void error(const char *msg);
const char* buildCommand(const char* command);
int always_true_callback(int preverify, WOLFSSL_X509_STORE_CTX* store);
int mqtt_tls_cb(MqttClient* client);
int mqtt_message_cb(MqttClient *client, MqttMessage *msg, byte msg_new, byte msg_done);
int mqtt_message_cb(MqttClient *client, MqttMessage *msg, byte msg_new, byte msg_done);
void setup_timeout(struct timeval* tv, int timeout_ms);
int socket_get_error(int sockFd);
int mqtt_net_connect(void *context, const char* host, word16 port, int timeout_ms);
int mqtt_net_read(void *context, byte* buf, int buf_len, int timeout_ms);
int mqtt_net_write(void *context, const byte* buf, int buf_len, int timeout_ms);
int mqtt_net_disconnect(void *context);
word16 mqtt_get_packetid(void);

#endif
