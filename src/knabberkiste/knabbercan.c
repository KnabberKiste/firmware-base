#include <knabberkiste/knabbercan.h>
#include <knabberkiste/hal/gpio.h>
#include <knabberkiste/hal/bxcan.h>
#include <knabberkiste/util/fifo.h>
#include <knabberkiste/util/error.h>
#include <knabberkiste/util/varbuf.h>
#include <string.h>

/* Constants */
#define KC_NUMBER_OF_TRANSACTION_IDS 256
#define KC_INTERNAL_EVENT_FIFO_SIZE 2
#define KC_RECV_FIFO_SIZE 32
#define KC_FRAME_COUNTER_MAX 7
#define KC_LED_FLASH_TICKS 1

/* Identifier bit-field struct */
typedef union __attribute__((__packed__)) {
    uint32_t value;
    struct __attribute__((packed)) {
        KC_Address_t receiver_address : 7;
        KC_Address_t sender_address : 7;
        KC_TransactionID_t transaction_id : 8;
        uint8_t counter : 3;
        bool last : 1;
        bool first : 1;
        KC_FrameType_t frame_type : 2;
    } components;
} KC_Identifier_t;

typedef struct {
    KC_FrameType_t frame_type;
    KC_Address_t sender_address;
    KC_Address_t receiver_address;
    KC_TransactionID_t transaction_id;
    size_t payload_size;
    uint8_t* payload;
    uint8_t previous_counter_value;
} KC_Received_Frame_t;

/* GPIO pin definitions */
#define KC_RXD_PIN PA11
#define KC_TXD_PIN PA12
#define KC_RXD_AF GPIO_AF9
#define KC_TXD_AF GPIO_AF9

#define KC_OUTLED_GREEN_PIN PA0
#define KC_INLED_GREEN_PIN PA1
#define KC_OUTLED_YELLOW_PIN PA2
#define KC_INLED_YELLOW_PIN PA3

#define KC_CONN_IN_PIN PA8
#define KC_CONN_OUT_PIN PA7

#define KC_DAISY_IN_PIN PA6
#define KC_DAISY_OUT_PIN PA4

#define KC_STBY_PIN PA5

/* Internal variables */
KC_Address_t kc_node_address = 0;
KC_Address_t kc_bus_size = 0;
static KC_CommandCallback_t volatile kc_command_callbacks[KC_NUMBER_OF_TRANSACTION_IDS] = { 0 };
static KC_EventCallback_t volatile kc_event_callbacks[KC_NUMBER_OF_TRANSACTION_IDS] = { 0 };
static volatile KC_State_t kc_state = KC_STATE_UNINITIALIZED;
static KC_Received_Frame_t* kc_incomplete_frames = 0;
static bool send_flag = false;
static bool indicators_active = true;
const char* kcan_fwr_name = "<unknown>";

fifo_declare_qualifier(KC_Received_Frame_t, kc_recv_fifo, KC_RECV_FIFO_SIZE, static);

/* Internal functions */
static void kc_check_if_addressing_required();
static bool kc_in_connected();
static bool kc_out_connected();
static void kc_request_addressing();
static void kc_address_next();
static void kc_address_end();

/* CAN bus callbacks*/
void can_recv_callback(CAN_ReceivedFrame_t frame) {
    // Validate the identifier
    if(frame.frame.rtr || !frame.frame.id_extended) {
        error_throw(ERR_KC_INVALID_FRAME, "Identifier invalid.");
    }

    KC_Identifier_t identifier;
    identifier.value = frame.frame.id;

    // Search for an incomplete frame from that address
    for(size_t i = 0; i < varbuf_length(kc_incomplete_frames); i++) {
        KC_Received_Frame_t* incomplete_frame = &(kc_incomplete_frames[i]);
        if(
            identifier.components.receiver_address == incomplete_frame->receiver_address &&
            identifier.components.frame_type == incomplete_frame->frame_type &&
            identifier.components.sender_address == incomplete_frame->sender_address &&
            identifier.components.transaction_id == incomplete_frame->transaction_id
        ) {
            // An incomplete frame exists

            // Check if this can be the case
            if(identifier.components.first) error_throw(ERR_KC_INVALID_FRAME, "Previous frame incomplete.");

            // Check the frame counter
            uint8_t expected_frame_counter = (incomplete_frame->previous_counter_value + 1) % (KC_FRAME_COUNTER_MAX - 1);
            if(expected_frame_counter != identifier.components.counter) {
                error_throw(ERR_KC_INVALID_FRAME, "Frame counter error.");
            }
            incomplete_frame->previous_counter_value = expected_frame_counter;

            // Append the data to the incomplete frame
            varbuf_push_chunk(incomplete_frame->payload, &(frame.frame.data), frame.frame.dlc);
            incomplete_frame->payload_size += frame.frame.dlc;

            // Pop the frame if it's complete
            if(identifier.components.last) {
                KC_Received_Frame_t complete_frame;
                varbuf_pop_index(kc_incomplete_frames, i, &complete_frame);
                if(fifo_full(kc_recv_fifo)) {
                    error_throw(ERR_OVERRUN, "knabberCAN receive FIFO overrun.");
                }
                fifo_put(kc_recv_fifo, complete_frame);
            }

            return;
        }
    }
    // No incomplete frame was found

    // Check if this can be the case
    if(!identifier.components.first) {
        error_throw(ERR_KC_INVALID_FRAME, "First packed seems to have been dropped.");
    }

    KC_Received_Frame_t kc_frame = { 0 };
    kc_frame.frame_type = identifier.components.frame_type;
    kc_frame.sender_address = identifier.components.sender_address;
    kc_frame.receiver_address = identifier.components.receiver_address;
    kc_frame.transaction_id = identifier.components.transaction_id;
    kc_frame.previous_counter_value = identifier.components.counter;
    kc_frame.payload = 0;
    
    varbuf_push_chunk(kc_frame.payload, &(frame.frame.data), frame.frame.dlc);
    kc_frame.payload_size = frame.frame.dlc;
    
    if(identifier.components.last) {
        if(fifo_full(kc_recv_fifo)) {
            error_throw(ERR_OVERRUN, "knabberCAN receive FIFO overrun.");
        }
        fifo_put(kc_recv_fifo, kc_frame);
    } else {
        varbuf_push(kc_incomplete_frames, kc_frame);
    }
}

void can_error_callback(CAN_ErrorCode_t error_code) {
    // Errors are ignored by knabberCAN
}

/* Event and command handlers */
static void kc_internal_event_handler(KC_Received_EventFrame_t event_frame) {
    switch(event_frame.event_id) {
        case KC_EVENT_ADDRESSING_FINISHED:
            // Set the bus size
            kc_bus_size = event_frame.sender_address;
            kc_address_end();
            break;

        case KC_EVENT_ADDRESSING_NEXT:
            if(KC_DAISY_IN_PIN->input_data == 0) {
                // This is the node currently being addressed.
                kc_node_address = event_frame.sender_address + 1;

                vcp_println("Node address received!");
                
                // Address the next node
                kc_event_emit(KC_EVENT_ADDRESSING_SUCCESS, 0, 0);
                kc_address_next();
            }
            break;

        case KC_EVENT_ADDRESSING_SUCCESS:
            break;

        case KC_EVENT_ADDRESSING_START:
            vcp_println("Addressing procedure started.");
            kc_state = KC_STATE_ADDRESSING;
            KC_DAISY_IN_PIN->pull_mode = GPIO_PULLUP;
            break;

        case KC_EVENT_ADDRESSING_REQUIRED:
            // Ignore requests while already addressing
            if(kc_state == KC_STATE_ADDRESSING) { break; }

            // Indicate readyness for addressing
            kc_state = KC_STATE_ADDRESSING;
            KC_DAISY_IN_PIN->pull_mode = GPIO_PULLUP;

            if(kc_in_connected() == false) {
                vcp_println("Initiating addressing procedure...");
                kc_event_emit(KC_EVENT_ADDRESSING_START, 0, 0);

                // This is the first node on the bus, and it must start the
                // addressing procedure
                kc_node_address = 1;
                kc_address_next();
            }
            break;
    }
}
static KC_Response_t kc_internal_command_handler(KC_Received_CommandFrame_t command_frame) {
    KC_Response_t response = { .payload = 0 };

    switch(command_frame.command_id) {
        case KC_COMMAND_RESET:
            NVIC_SystemReset();
            break;

        case KC_COMMAND_SET_INDICATORS_ACTIVE:
            indicators_active = *((bool*)command_frame.payload);
            break;

        case KC_COMMAND_READ_FWR_NAME:
            varbuf_push_chunk(
                response.payload,
                kcan_fwr_name,
                strlen(kcan_fwr_name)
            );
            response.payload_size = strlen(kcan_fwr_name);
            break;
    }

    return response;
}

/* Internal function definitions */
static void kc_request_addressing() {
    KC_Received_EventFrame_t ef = { .event_id = KC_EVENT_ADDRESSING_REQUIRED, .payload = 0, .payload_size = 0, .sender_address = 0 };
    kc_event_emit(KC_EVENT_ADDRESSING_REQUIRED, 0, 0);
    kc_internal_event_handler(ef);
}

void kc_frame_transmit(
    KC_FrameType_t frame_type,
    KC_TransactionID_t tid,
    KC_Address_t receiver,
    size_t payload_size,
    void* payload
) {
    KC_Identifier_t id;
    id.components.counter = 0;
    id.components.receiver_address = receiver;
    id.components.sender_address = kc_node_address;
    id.components.transaction_id = tid;
    id.components.frame_type = frame_type;

    for(size_t payload_offset = 0; (payload_offset < payload_size) || payload_offset == 0; payload_offset += 8) {
        size_t bytes_remaining = (payload_size - payload_offset);
        id.components.first = (payload_offset == 0);
        id.components.last = bytes_remaining <= 8;

        uint8_t dlc = 8;
        if(id.components.last) {
            dlc = bytes_remaining;
        }

        CAN_Frame_t frame = { 0 };
        frame.dlc = dlc;
        frame.id = id.value;
        frame.id_extended = true;
        memcpy(frame.data, payload + payload_offset, dlc);

        can_transmit_frame(&frame);

        id.components.counter = (id.components.counter + 1) % (KC_FRAME_COUNTER_MAX + 1);
    }
    send_flag = true;
}

static void kc_address_end() {
    // Set the DAISY signal to be Hi-Z again
    KC_DAISY_OUT_PIN->mode = GPIO_MODE_INPUT;
    KC_DAISY_OUT_PIN->pull_mode = GPIO_NOPULL;

    // Pull down the DAISY signal for the previous node
    KC_DAISY_IN_PIN->pull_mode = GPIO_PULLDOWN;

    // Inform the user
    char info_string[64] = { 0 };
    snprintf(info_string, 64, "Addressing finished [ Node address = %d, Bus size = %d ]", kc_node_address, kc_bus_size);
    vcp_println(info_string);

    if(kc_node_address) {
        // Emit the ONLINE event if addressed successfully
        kc_event_emit(KC_EVENT_ONLINE, 0, 0);

        // Configure the CAN filter bank to match node-specific frames
        // Broadcast frames are accepted by filterbank 1
        can_configure_filter_bank(
            CAN_FILTERBANK_1,
            CAN_FIFO_0,
            CAN_FILTERBANK_WIDTH_32BIT,
            CAN_FILTERBANK_MODE_MASK,
            0b00000000000000000000000000000100 | kc_node_address << 3,
            0b00000000000000000000001111111110
        );

        // Addressing has been finished
        kc_state = KC_STATE_READY;
    } else {
        // Addressing hasn't been successful, request another addressing procedure
        kc_request_addressing();
    }
}

static void kc_address_next() {
    if(kc_out_connected()) {
        vcp_println("Addressing next node...");

        // There's at least one more node in the chain
        // Wait for the next node to be ready for addressing
        KC_DAISY_OUT_PIN->mode = GPIO_MODE_INPUT;
        KC_DAISY_OUT_PIN->pull_mode = GPIO_NOPULL;
        while(!KC_DAISY_OUT_PIN->input_data);

        // Pull down the DAISY signal for the next node
        KC_DAISY_OUT_PIN->mode = GPIO_MODE_OUTPUT;
        KC_DAISY_OUT_PIN->output_data = 0;

        kc_event_emit(KC_EVENT_ADDRESSING_NEXT, 0, 0);
    } else {
        // This is the last node in the chain
        // Notify the other nodes that the addressing has been finished
        kc_event_emit(KC_EVENT_ADDRESSING_FINISHED, 0, 0);

        kc_bus_size = kc_node_address;
        kc_address_end();
    }
}

/* Public function definitions */
void kc_init() {
    kc_state = KC_STATE_INITIALIZING;

    /* Configure the GPIO pins */
    KC_RXD_PIN->mode = GPIO_MODE_ALTERNATE;
    KC_TXD_PIN->mode = GPIO_MODE_ALTERNATE;
    KC_RXD_PIN->alternate = KC_RXD_AF;
    KC_TXD_PIN->alternate = KC_TXD_AF;

    KC_OUTLED_GREEN_PIN->mode = GPIO_MODE_OUTPUT;
    KC_INLED_GREEN_PIN->mode = GPIO_MODE_OUTPUT;
    KC_OUTLED_YELLOW_PIN->mode = GPIO_MODE_OUTPUT;
    KC_INLED_YELLOW_PIN->mode = GPIO_MODE_OUTPUT;

    KC_CONN_IN_PIN->mode = GPIO_MODE_INPUT;
    KC_CONN_OUT_PIN->mode = GPIO_MODE_INPUT;

    KC_DAISY_IN_PIN->mode = GPIO_MODE_INPUT;
    KC_DAISY_OUT_PIN->mode = GPIO_MODE_INPUT;
    KC_DAISY_IN_PIN->pull_mode = GPIO_PULLDOWN;
    KC_DAISY_OUT_PIN->pull_mode = GPIO_NOPULL;

    KC_STBY_PIN->mode = GPIO_MODE_OUTPUT;

    /* Define pre-defined commands and events */
    kc_event_define(KC_EVENT_ADDRESSING_START, kc_internal_event_handler);
    kc_event_define(KC_EVENT_ADDRESSING_NEXT, kc_internal_event_handler);
    kc_event_define(KC_EVENT_ADDRESSING_SUCCESS, kc_internal_event_handler);
    kc_event_define(KC_EVENT_ADDRESSING_FINISHED, kc_internal_event_handler);
    kc_event_define(KC_EVENT_ADDRESSING_REQUIRED, kc_internal_event_handler);
    kc_command_define(KC_COMMAND_RESET, kc_internal_command_handler);
    kc_command_define(KC_COMMAND_SET_INDICATORS_ACTIVE, kc_internal_command_handler);
    kc_command_define(KC_COMMAND_READ_FWR_NAME, kc_internal_command_handler);
    
    /* Initialize the CAN peripheral */
    can_init(1000000, CAN_TESTMODE_NONE);

    // Configure the filter bank to match broadcast frames
    can_configure_filter_bank(
        CAN_FILTERBANK_0,
        CAN_FIFO_0,
        CAN_FILTERBANK_WIDTH_32BIT,
        CAN_FILTERBANK_MODE_MASK,
        0b00000000000000000000000000000100,
        0b00000000000000000000001111111110
    );

    kc_request_addressing();
}

void kc_command_define(KC_TransactionID_t command_id, KC_CommandCallback_t callback) {
    if(kc_command_callbacks[command_id] == 0) {
        kc_command_callbacks[command_id] = callback;
    } else {
        error_throw(ERR_RUNTIME_GENERIC, "Command is already defined.");
    }
}

void kc_event_define(KC_TransactionID_t event_id, KC_EventCallback_t callback) {
    if(kc_event_callbacks[event_id] == 0) {
        kc_event_callbacks[event_id] = callback;
    } else {
        error_throw(ERR_RUNTIME_GENERIC, "Event is already defined.");
    }
}

KC_State_t kc_get_state() { return kc_state; }

void kc_event_emit(KC_TransactionID_t event_id, void* payload, size_t payload_size) {
    kc_frame_transmit(
        KC_FRAMETYPE_EVENT,
        event_id,
        KC_ADDRESS_BROADCAST,
        payload_size,
        payload
    );
}

void kc_process_incoming() {
    kc_check_if_addressing_required();

    bool received = false;

    while(!fifo_empty(kc_recv_fifo)) {
        KC_Received_Frame_t frame;
        fifo_get(kc_recv_fifo, frame);
        received = true;

        switch(frame.frame_type) {
            case KC_FRAMETYPE_EVENT:
                if(kc_event_callbacks[frame.transaction_id] != 0) {
                    KC_Received_EventFrame_t event_frame;
                    event_frame.event_id = frame.transaction_id;
                    event_frame.payload = frame.payload;
                    event_frame.payload_size = frame.payload_size;
                    event_frame.sender_address = frame.sender_address;

                    kc_event_callbacks[frame.transaction_id](event_frame);
                }
                break;

            case KC_FRAMETYPE_COMMAND:
                if(kc_command_callbacks[frame.transaction_id] != 0) {
                    KC_Received_CommandFrame_t command_frame;
                    command_frame.command_id = frame.transaction_id;
                    command_frame.payload = frame.payload;
                    command_frame.payload_size = frame.payload_size;
                    command_frame.receiver_address = frame.receiver_address;
                    command_frame.sender_address = frame.sender_address;

                    KC_Response_t response = kc_command_callbacks[frame.transaction_id](command_frame);
                    kc_frame_transmit(
                        KC_FRAMETYPE_RESPONSE,
                        command_frame.command_id,
                        command_frame.sender_address,
                        response.payload_size,
                        response.payload
                    );

                    // Invalidate response payload buffer
                    varbuf_clear(response.payload);
                }
                break; // TODO IMPLEMENT THIS

            case KC_FRAMETYPE_RESPONSE:
                break; // TODO IMPLEMENT THIS

            case KC_FRAMETYPE_ERROR:
                break; // TODO IMPLEMENT THIS
        }

        // Deallocate the payload to avoid a memory leak
        varbuf_clear(frame.payload);
    }

    // Set the LED state
    static TickType_t last_send_tick = 0;
    static TickType_t last_recv_tick = 0;
    TickType_t current_tick = xTaskGetTickCount();
    if(send_flag) {
        last_send_tick = current_tick;
        send_flag = false;
    }
    if(received) {
        last_recv_tick = current_tick;
        received = false;
    }

    bool send_led_on = (current_tick - last_send_tick < KC_LED_FLASH_TICKS) && indicators_active;
    bool recv_led_on = (current_tick - last_recv_tick < KC_LED_FLASH_TICKS) && indicators_active;
    KC_INLED_GREEN_PIN->output_data = send_led_on;
    KC_OUTLED_GREEN_PIN->output_data = send_led_on;
    KC_INLED_YELLOW_PIN->output_data = recv_led_on;
    KC_OUTLED_YELLOW_PIN->output_data = recv_led_on;
}

static void kc_check_if_addressing_required() {
    static bool has_been_checked = false;
    static bool conn_in_previous_state = 0;
    static bool conn_out_previous_state = 0;

    bool conn_in_current_state = kc_in_connected();
    bool conn_out_current_state = kc_out_connected();

    if(has_been_checked && kc_state == KC_STATE_READY) {
        // Check the CONN_IN pin for changes
        if(
            conn_in_current_state != conn_in_previous_state ||
            conn_out_previous_state != conn_out_current_state
        ) {
            kc_request_addressing();
        }
    }

    conn_in_previous_state = conn_in_current_state;
    conn_out_previous_state = conn_out_current_state;
    has_been_checked = true;
}

static bool kc_in_connected() {
    // LED pins must by Hi-Z for this
    KC_INLED_GREEN_PIN->mode = GPIO_MODE_ANALOG;

    for(uint16_t i = 0; i < UINT8_MAX; i++) __asm("NOP");
    bool result = KC_CONN_IN_PIN->input_data;

    // Reset LED pins
    KC_INLED_GREEN_PIN->mode = GPIO_MODE_OUTPUT;
    
    return result;
}
static bool kc_out_connected() {
    // LED pins must by Hi-Z for this
    KC_OUTLED_GREEN_PIN->mode = GPIO_MODE_ANALOG;

    for(uint16_t i = 0; i < UINT8_MAX; i++) __asm("NOP");
    bool result = KC_CONN_OUT_PIN->input_data;

    // Reset LED pins
    KC_OUTLED_GREEN_PIN->mode = GPIO_MODE_OUTPUT;
    
    return result;
}