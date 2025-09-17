#include "protocol.h"
#include <string.h>

#define PROTOCOL_START_MARKER 0xAA

// Simple CRC16 implementation
uint16_t calculate_checksum(const uint8_t* data, uint16_t length) {
    uint16_t crc = 0xFFFF;
    
    for (uint16_t i = 0; i < length; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc = crc >> 1;
            }
        }
    }
    
    return crc;
}

bool validate_message(const protocol_message_t* msg) {
    if (msg == NULL) return false;
    
    // Check start marker
    if (msg->header.start_marker != PROTOCOL_START_MARKER) return false;
    
    // Check version
    if (msg->header.version != PROTOCOL_VERSION) return false;
    
    // Check payload length
    if (msg->header.payload_len > MAX_PAYLOAD_SIZE) return false;
    
    // Calculate and verify checksum
    uint16_t calc_checksum = calculate_checksum((const uint8_t*)&msg->header + 6, 
                                              sizeof(protocol_header_t) - 6 + msg->header.payload_len);
    
    return (calc_checksum == msg->header.checksum);
}

int encode_message(const protocol_message_t* msg, uint8_t* buffer, int buffer_size) {
    if (msg == NULL || buffer == NULL) return -1;
    
    int total_size = sizeof(protocol_header_t) + msg->header.payload_len;
    if (buffer_size < total_size) return -1;
    
    // Copy header and payload
    memcpy(buffer, &msg->header, sizeof(protocol_header_t));
    if (msg->header.payload_len > 0) {
        memcpy(buffer + sizeof(protocol_header_t), &msg->payload, msg->header.payload_len);
    }
    
    return total_size;
}

int decode_message(const uint8_t* buffer, int buffer_len, protocol_message_t* msg) {
    if (buffer == NULL || msg == NULL) return -1;
    if (buffer_len < sizeof(protocol_header_t)) return -1;
    
    // Copy header
    memcpy(&msg->header, buffer, sizeof(protocol_header_t));
    
    // Validate basic header
    if (msg->header.start_marker != PROTOCOL_START_MARKER) return -1;
    if (msg->header.payload_len > MAX_PAYLOAD_SIZE) return -1;
    
    int total_size = sizeof(protocol_header_t) + msg->header.payload_len;
    if (buffer_len < total_size) return -1;
    
    // Copy payload if present
    if (msg->header.payload_len > 0) {
        memcpy(&msg->payload, buffer + sizeof(protocol_header_t), msg->header.payload_len);
    }
    
    // Validate complete message
    if (!validate_message(msg)) return -1;
    
    return total_size;
}

void build_move_command(protocol_message_t* msg, int8_t direction, int8_t turn, 
                       uint8_t speed, uint8_t flags, uint8_t seq_num) {
    if (msg == NULL) return;
    
    // Build header
    msg->header.start_marker = PROTOCOL_START_MARKER;
    msg->header.version = PROTOCOL_VERSION;
    msg->header.msg_type = MSG_TYPE_MOVE_CMD;
    msg->header.seq_num = seq_num;
    msg->header.payload_len = sizeof(move_command_payload_t);
    
    // Build payload
    msg->payload.move_cmd.direction = direction;
    msg->payload.move_cmd.turn = turn;
    msg->payload.move_cmd.speed = speed;
    msg->payload.move_cmd.flags = flags;
    msg->payload.move_cmd.timestamp = 0; // Would be filled with actual timestamp
    
    // Calculate checksum
    msg->header.checksum = calculate_checksum((const uint8_t*)&msg->header + 6,
                                            sizeof(protocol_header_t) - 6 + msg->header.payload_len);
}

void build_status_response(protocol_message_t* msg, float angle, float velocity,
                         uint8_t state, uint8_t seq_num) {
    if (msg == NULL) return;
    
    // Build header
    msg->header.start_marker = PROTOCOL_START_MARKER;
    msg->header.version = PROTOCOL_VERSION;
    msg->header.msg_type = MSG_TYPE_STATUS_RESP;
    msg->header.seq_num = seq_num;
    msg->header.payload_len = sizeof(status_response_payload_t);
    
    // Build payload
    msg->payload.status_resp.angle = angle;
    msg->payload.status_resp.velocity = velocity;
    msg->payload.status_resp.robot_state = state;
    msg->payload.status_resp.gps_status = 0;
    msg->payload.status_resp.latitude = 0.0f;
    msg->payload.status_resp.longitude = 0.0f;
    msg->payload.status_resp.battery_level = 100;
    msg->payload.status_resp.error_flags = 0;
    
    // Calculate checksum
    msg->header.checksum = calculate_checksum((const uint8_t*)&msg->header + 6,
                                            sizeof(protocol_header_t) - 6 + msg->header.payload_len);
}

void build_error_message(protocol_message_t* msg, uint8_t error_code, uint8_t seq_num) {
    if (msg == NULL) return;
    
    // Build header
    msg->header.start_marker = PROTOCOL_START_MARKER;
    msg->header.version = PROTOCOL_VERSION;
    msg->header.msg_type = MSG_TYPE_ERROR;
    msg->header.seq_num = seq_num;
    msg->header.payload_len = 1;
    
    // Build payload
    msg->payload.raw_data[0] = error_code;
    
    // Calculate checksum
    msg->header.checksum = calculate_checksum((const uint8_t*)&msg->header + 6,
                                            sizeof(protocol_header_t) - 6 + msg->header.payload_len);
}