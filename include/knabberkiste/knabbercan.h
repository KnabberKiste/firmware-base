/**
 * @file knabbercan.h
 * @author Gabriel Heinzer
 * @brief knabberCAN protocol implementation.
 */

#pragma once

#include <stdlib.h>
#include <stdint.h>

/* Type definitions */
typedef uint8_t KC_TransactionID_t;
typedef uint8_t KC_Address_t;

typedef enum {
    KC_STATE_UNINITIALIZED = 0,
    KC_STATE_INITIALIZING,
    KC_STATE_ADDRESSING,
    KC_STATE_READY
} KC_State_t;

typedef enum {
    KC_FRAMETYPE_EVENT = 0,
    KC_FRAMETYPE_COMMAND = 1,
    KC_FRAMETYPE_RESPONSE = 2,
    KC_FRAMETYPE_ERROR = 3
} KC_FrameType_t;

typedef struct {
    KC_Address_t sender_address;
    KC_TransactionID_t event_id;
    size_t payload_size;
    void* payload;
} KC_Received_EventFrame_t;

typedef struct {
    KC_Address_t sender_address;
    KC_Address_t receiver_address;
    KC_TransactionID_t command_id;
    size_t payload_size;
    void* payload;
} KC_Received_CommandFrame_t;

typedef struct {
    size_t payload_size;
    void* payload;
} KC_Response_t;

typedef struct {
    KC_Address_t sender_address;
    KC_Address_t receiver_address;
    KC_TransactionID_t error_code;
    const char* error_message;
} KC_Received_ErrorFrame_t;

typedef void (*KC_EventCallback_t)(KC_Received_EventFrame_t);
typedef KC_Response_t (*KC_CommandCallback_t)(KC_Received_CommandFrame_t);

/* Event definitions */
#define KC_EVENT_ADDRESSING_START 0x00
#define KC_EVENT_ADDRESSING_SUCCESS 0x01
#define KC_EVENT_ADDRESSING_NEXT 0x02
#define KC_EVENT_ADDRESSING_FINISHED 0x03
#define KC_EVENT_ADDRESSING_REQUIRED 0x04
#define KC_EVENT_ONLINE 0x10

/* Special addresses */
#define KC_ADDRESS_BROADCAST 0

/**
 * @brief Node address assigned to the knabberCAN node.
 */
extern KC_Address_t kc_node_address;

extern KC_Address_t kc_bus_size;

/**
 * @brief Initializes the knabberCAN hardware resources and performs addressing. Blocks
 * until addressing is finished.
 */
void kc_init();

/**
 * @brief Emits an event with the given parameters.
 * 
 * @param event_id Event ID of the event to emit.
 * @param payload Pointer to the payload sent with the event.
 * @param payload_size Size of the payload to be sent with the event.
 */
void kc_event_emit(KC_TransactionID_t event_id, void* payload, size_t payload_size);

/**
 * @brief Defines a command with the given command ID and attaches the specified
 * callback to the command.
 * 
 * @param command_id Command ID to which the command corresponds.
 * @param callback Callback which will be called when the command is received.
 * 
 * @throws ERR_RUNTIME_GENERAL The specified command ID has already been defined.
 */
void kc_command_define(KC_TransactionID_t command_id, KC_CommandCallback_t callback);


/**
 * @brief Defines an event with the given event ID and attaches the specified
 * callback to the event.
 * 
 * @param command_id Event ID to which the command corresponds.
 * @param callback Callback which will be called when the event is received.
 * 
 * @throws ERR_RUNTIME_GENERAL The specified event ID has already been defined.
 */
void kc_event_define(KC_TransactionID_t event_id, KC_EventCallback_t callback);

/**
 * @brief Gets the current state of the knabber CAN initialization.
 * 
 * @return The state the library is currently in.
 */
KC_State_t kc_get_state();

/**
 * @brief Processes all incoming frames. Call this regularly to avoid an
 * overrun of frames.
 * 
 * This calls all callbacks for commands and events.
 */
void kc_process_incoming();