#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Protocol version
#define PROTOCOL_VERSION 0x01

// Message types
#define MSG_TYPE_MOVE_CMD       0x01
#define MSG_TYPE_STATUS_REQ     0x02
#define MSG_TYPE_STATUS_RESP    0x03
#define MSG_TYPE_CONFIG_SET     0x04
#define MSG_TYPE_CONFIG_GET     0x05
#define MSG_TYPE_ERROR          0xFF

// Command flags
#define CMD_FLAG_BALANCE        0x01
#define CMD_FLAG_STANDUP        0x02
#define CMD_FLAG_EMERGENCY      0x04

// Maximum payload size
#define MAX_PAYLOAD_SIZE        64

// Protocol header (fixed 8 bytes)
typedef struct __attribute__((packed)) {
    uint8_t start_marker;       // Always 0xAA
    uint8_t version;           // Protocol version
    uint8_t msg_type;          // Message type
    uint8_t seq_num;           // Sequence number
    uint16_t payload_len;      // Payload length
    uint16_t checksum;         // CRC16 checksum
} protocol_header_t;

// Move command payload
typedef struct __attribute__((packed)) {
    int8_t direction;          // -1, 0, 1
    int8_t turn;              // -100 to 100
    uint8_t speed;            // 0 to 100
    uint8_t flags;            // Command flags
    uint32_t timestamp;       // Command timestamp
} move_command_payload_t;

// Status response payload
typedef struct __attribute__((packed)) {
    float angle;              // Current angle
    float velocity;           // Current velocity
    uint8_t robot_state;      // Current robot state
    uint8_t gps_status;       // GPS status flags
    float latitude;           // GPS latitude (if available)
    float longitude;          // GPS longitude (if available)
    uint8_t battery_level;    // Battery percentage
    uint8_t error_flags;      // Error status flags
} status_response_payload_t;

// Configuration payload
typedef struct __attribute__((packed)) {
    uint8_t config_id;        // Configuration parameter ID
    float value;              // Configuration value
} config_payload_t;

// Complete protocol message
typedef struct __attribute__((packed)) {
    protocol_header_t header;
    union {
        move_command_payload_t move_cmd;
        status_response_payload_t status_resp;
        config_payload_t config;
        uint8_t raw_data[MAX_PAYLOAD_SIZE];
    } payload;
} protocol_message_t;

// Protocol functions
uint16_t calculate_checksum(const uint8_t* data, uint16_t length);
bool validate_message(const protocol_message_t* msg);
int encode_message(const protocol_message_t* msg, uint8_t* buffer, int buffer_size);
int decode_message(const uint8_t* buffer, int buffer_len, protocol_message_t* msg);

// Message builders
void build_move_command(protocol_message_t* msg, int8_t direction, int8_t turn, 
                       uint8_t speed, uint8_t flags, uint8_t seq_num);
void build_status_response(protocol_message_t* msg, float angle, float velocity,
                         uint8_t state, uint8_t seq_num);
void build_error_message(protocol_message_t* msg, uint8_t error_code, uint8_t seq_num);

#ifdef __cplusplus
}
#endif

#endif // PROTOCOL_H