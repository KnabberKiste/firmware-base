/**
 * @file knabbercan.h
 * @author Gabriel Heinzer
 * @brief knabberCAN protocol implementation.
 */

#pragma once

#include <stdlib.h>
#include <stdint.h>

/* Global variables */
extern const char* kcan_fwr_name;

/* Type definitions */
/**
 * @brief Type used for knabberCAN transaction IDs.
 */
typedef uint8_t KC_TransactionID_t;
/**
 * @brief Type used for knabberCAN addresses.
 */
typedef uint8_t KC_Address_t;

/**
 * @brief KnabberCAN state machine enumerator.
 */
typedef enum {
    /// @brief KnabberCAN is not currently initialized and therefore not working.
    KC_STATE_UNINITIALIZED = 0,
    /// @brief KnabberCAN is currently in the process of initializing.
    KC_STATE_INITIALIZING,
    /// @brief KnabberCAN is currently in the process of addressing the bus.
    KC_STATE_ADDRESSING,
    /// @brief KnabberCAN is ready for operation.
    KC_STATE_READY
} KC_State_t;

/**
 * @brief KnabberCAN frame type enumerator.
 */
typedef enum {
    /// @brief Indicates an event frame.
    KC_FRAMETYPE_EVENT = 0,
    /// @brief Indicates a command frame.
    KC_FRAMETYPE_COMMAND = 1,
    /// @brief Indicates a response frame.
    KC_FRAMETYPE_RESPONSE = 2,
    /// @brief Indicates an error frame.
    KC_FRAMETYPE_ERROR = 3
} KC_FrameType_t;

/**
 * @brief Struct representing an event frame which was received from the knabberCAN bus.
 */
typedef struct {
    /// @brief Address of the node which emitted the event.
    KC_Address_t sender_address;
    /// @brief Unique identifier of the event.
    KC_TransactionID_t event_id;
    /// @brief Size of the payload which is stored in @ref payload.
    size_t payload_size;
    /// @brief Pointer to a buffer storing the payload. This is allocated using @ref varbuf.h.
    void* payload;
} KC_Received_EventFrame_t;

/**
 * @brief Struct representing a command frame which was received from the knabberCAN bus.
 */
typedef struct {
    /// @brief Address of the node which sent the command.
    KC_Address_t sender_address;
    /// @brief Address of the node which received the command. This should either be the receiver address, or the broadcast address.
    KC_Address_t receiver_address;
    /// @brief Unique identifier of the command.
    KC_TransactionID_t command_id;
    /// @brief Size of the payload which is stored in @ref payload.
    size_t payload_size;
    /// @brief Pointer to a buffer storing the payload. This is allocated using @ref varbuf.h.
    uint8_t* payload;
} KC_Received_CommandFrame_t;

/**
 * @brief KnabberCAN response type.
 */
typedef struct {
    uint8_t* payload;
    size_t payload_size;
} KC_Response_t;

/**
 * @brief Struct representing an error frome received from the knabberCAN bus.
 * 
 */
typedef struct {
    /// @brief Address from which the error was sent.
    KC_Address_t sender_address;
    /// @brief Address which should receive the error. This should either be the receiver address, or the broadcast address.
    KC_Address_t receiver_address;
    /// @brief Error code received with the error.
    KC_TransactionID_t error_code;
    /// @brief Error message received with the error.
    const char* error_message;
} KC_Received_ErrorFrame_t;

/// @brief Type for a callback which may be assigned to an event.
typedef void (*KC_EventCallback_t)(KC_Received_EventFrame_t);
/// @brief Type for a callback which may be assigned to handle a command.
typedef KC_Response_t (*KC_CommandCallback_t)(KC_Received_CommandFrame_t);

/* Event definitions */
/**
 * @brief KnabberCAN `ADDRESSING_START` event ID.
 */
#define KC_EVENT_ADDRESSING_START 0x00
/**
 * @brief KnabberCAN `ADDRESSING_SUCCESS` event ID.
 */
#define KC_EVENT_ADDRESSING_SUCCESS 0x01
/**
 * @brief KnabberCAN `ADDRESSING_NEXT` event ID.
 */
#define KC_EVENT_ADDRESSING_NEXT 0x02
/**
 * @brief KnabberCAN `ADDRESSING_FINISHED` event ID.
 */
#define KC_EVENT_ADDRESSING_FINISHED 0x03
/**
 * @brief KnabberCAN `ADDRESSING_REQUIRED` event ID.
 */
#define KC_EVENT_ADDRESSING_REQUIRED 0x04
/**
 * @brief KnabberCAN `ONLINE` event ID.
 */
#define KC_EVENT_ONLINE 0x10

/* Command definitions */
/**
 * @brief KnabberCAN ``RESET`` command.
 */
#define KC_COMMAND_RESET 0x00
/**
 * @brief KnabberCAN ``SET INDICATORS ACTIVE`` command.
 */
#define KC_COMMAND_SET_INDICATORS_ACTIVE 0x10
/**
 * @brief KnabberCAN ``READ FWR NAME`` command.
 */
#define KC_COMMAND_READ_FWR_NAME 0x11

/* Special addresses */
/**
 * @brief KnabberCAN broadcast address.
 */
#define KC_ADDRESS_BROADCAST 0

/**
 * @brief Node address assigned to the knabberCAN node.
 */
extern KC_Address_t kc_node_address;

/**
 * @brief Current size of the knabberCAN bus.
 */
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
 * @param event_id Event ID to which the command corresponds.
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