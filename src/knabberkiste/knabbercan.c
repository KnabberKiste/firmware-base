#include <knabberkiste/knabbercan.h>
#include <knabberkiste/hal/gpio.h>
#include <knabberkiste/hal/bxcan.h>
#include <knabberkiste/util/fifo.h>
#include <knabberkiste/util/error.h>
#include <knabberkiste/util/varbuf.h>

/* Constants */
#define KC_NUMBER_OF_TRANSACTION_IDS 256
#define KC_INTERNAL_EVENT_FIFO_SIZE 2
#define KC_RECV_FIFO_SIZE 32
#define KC_FRAME_COUNTER_MAX 7

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

fifo_declare_qualifier(KC_Received_Frame_t, kc_recv_fifo, KC_RECV_FIFO_SIZE, static);
fifo_declare_qualifier(KC_Received_EventFrame_t, kc_event_fifo, KC_INTERNAL_EVENT_FIFO_SIZE, static);

/* Internal functions */
static void kc_frame_transmit(
    KC_FrameType_t frame_type,
    KC_TransactionID_t tid,
    KC_Address_t receiver,
    size_t payload_size,
    void* payload
);
static void kc_check_if_addressing_required();
static bool kc_in_connected();
static bool kc_out_connected();

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
        case KC_EVENT_ADDRESSING_NEXT:
        case KC_EVENT_ADDRESSING_SUCCESS:
            if(kc_state == KC_STATE_ADDRESSING) {
                if(fifo_full(kc_event_fifo)) {
                    error_throw(ERR_OVERRUN, "Internal event fifo overrun.");
                }
                fifo_put(kc_event_fifo, event_frame);
            };
            break;

        case KC_EVENT_ADDRESSING_REQUIRED:
            if(kc_state == KC_STATE_READY) {
                kc_perform_addressing_sequence();
            }
            break;
    }
}

/* Internal function definitions */
static void kc_frame_transmit(
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
}

void kc_perform_addressing_sequence() {
    vcp_println("Addressing requested");

    kc_event_emit(KC_EVENT_ADDRESSING_REQUIRED, 0, 0);

    kc_state = KC_STATE_ADDRESSING;

    /* Assert CONN_IN */
    if(kc_in_connected()) {
        // Another node is connected to the IN port

        vcp_println("Waiting for DAISY_IN");
        
        // Wait for the DAISY signal to go high
        while(!(KC_DAISY_IN_PIN->input_data)) {
            kc_event_emit(KC_EVENT_ADDRESSING_REQUIRED, 0, 0);

            // Wait a bit, how long is not crucial
            for(int i = 0; i < 0xFFFF; i++) asm("NOP");
        }

        vcp_println("Waiting for ADDRESSING_NEXT");

        // Wait for ADDRESSING_NEXT
        while(1) {
            if(!fifo_empty(kc_event_fifo)) {
                KC_Received_EventFrame_t evt;
                fifo_get(kc_event_fifo, evt);
                if(evt.event_id == KC_EVENT_ADDRESSING_NEXT) {
                    // Assign the sender's address + 1 to this node
                    kc_node_address = evt.sender_address + 1;

                    // Emit ADDRESSING_SUCCESS
                    kc_event_emit(KC_EVENT_ADDRESSING_SUCCESS, 0, 0);
                    break;
                }
            }

            kc_process_incoming();
        }

        vcp_println("Waiting for ADDRESSING_FINISHED");

        
        if(kc_out_connected()) {
            // Wait for ADDRESSING_FINISHED
            while(1) {
                if(!fifo_empty(kc_event_fifo)) {
                    KC_Received_EventFrame_t evt = fifo_get_direct(kc_event_fifo);
                    if(evt.event_id == KC_EVENT_ADDRESSING_FINISHED) {
                        kc_bus_size = evt.sender_address;
                        break;
                    }
                }
    
                kc_process_incoming();
            }
        } else {
            kc_event_emit(KC_EVENT_ADDRESSING_FINISHED, 0, 0);
        }

        vcp_println("Addressing is finished!");
    } else {
        // This is the first node in the bus

        // Start the addressing procedure
        kc_node_address = 1;

        if(kc_out_connected()) {
            // This is not the only node in the bus
            kc_event_emit(KC_EVENT_ADDRESSING_START, 0, 0);

            vcp_println("Waiting for ADDRESSING_SUCCESS...");

            // Set the DAISY OUT pin
            KC_DAISY_OUT_PIN->output_data = 1;

            // Emit ADDRESSING_NEXT until receiving ADDRESSING_SUCCESS
            while(1) {
                kc_event_emit(KC_EVENT_ADDRESSING_NEXT, 0, 0);
                kc_process_incoming();

                // Wait a bit, how long is not crucial
                for(int i = 0; i < 0xFFFF; i++) asm("NOP");

                if(!fifo_empty(kc_event_fifo)) {
                    if(fifo_get_direct(kc_event_fifo).event_id == KC_EVENT_ADDRESSING_SUCCESS) {
                        // The next node is addressed successfully
                        break;
                    } else {
                        error_throw(ERR_RUNTIME_GENERIC, "Unexpected event received.");
                    }
                }
            }

            vcp_println("Waiting for ADDRESSING_FINISHED");

            // Wait for ADDRESSING_FINISHED
            while(1) {
                if(!fifo_empty(kc_event_fifo)) {
                    KC_Received_EventFrame_t evt = fifo_get_direct(kc_event_fifo);
                    if(evt.event_id == KC_EVENT_ADDRESSING_FINISHED) {
                        kc_bus_size = evt.sender_address;
                        break;
                    }
                }
    
                kc_process_incoming();
            }
        } else {
            // This is the only node on the bus
            vcp_println("Addressing skipped.");
        }

        vcp_println("Addressing finished.");
    }

    // Clear the DAISY pin
    KC_DAISY_OUT_PIN->output_data = 0;

    KC_INLED_GREEN_PIN->output_data ^= 1;

    kc_state = KC_STATE_READY;

    vcp_println("Addressing performed!");
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
    KC_DAISY_OUT_PIN->mode = GPIO_MODE_OUTPUT;

    KC_STBY_PIN->mode = GPIO_MODE_OUTPUT;

    /* Define pre-defined commands and events */
    kc_event_define(KC_EVENT_ADDRESSING_NEXT, kc_internal_event_handler);
    kc_event_define(KC_EVENT_ADDRESSING_SUCCESS, kc_internal_event_handler);
    kc_event_define(KC_EVENT_ADDRESSING_FINISHED, kc_internal_event_handler);
    kc_event_define(KC_EVENT_ADDRESSING_REQUIRED, kc_internal_event_handler);
    
    /* Initialize the CAN peripheral */
    can_init(1000000, CAN_TESTMODE_NONE);

    can_configure_filter_bank(
        CAN_FILTERBANK_0,
        CAN_FIFO_0,
        CAN_FILTERBANK_WIDTH_32BIT,
        CAN_FILTERBANK_MODE_MASK,
        0x00000000,
        0x00000000
    );

    kc_event_emit(KC_EVENT_ADDRESSING_REQUIRED, 0, 0);
    kc_perform_addressing_sequence();
    kc_event_emit(KC_EVENT_ONLINE, 0, 0);
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

    while(!fifo_empty(kc_recv_fifo)) {
        KC_Received_Frame_t frame;
        fifo_get(kc_recv_fifo, frame);

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
                break; // TODO IMPLEMENT THIS

            case KC_FRAMETYPE_RESPONSE:
                break; // TODO IMPLEMENT THIS

            case KC_FRAMETYPE_ERROR:
                break; // TODO IMPLEMENT THIS
        }

        // Deallocate the payload to avoid a memory leak
        varbuf_clear(frame.payload);
    }
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
            vcp_println("Connection change detected!");
            kc_perform_addressing_sequence();
        }
    }

    conn_in_previous_state = conn_in_current_state;
    conn_out_previous_state = conn_out_current_state;
    has_been_checked = true;
}

static bool kc_in_connected() {
    // LED pins must by Hi-Z for this
    KC_INLED_GREEN_PIN->mode = GPIO_MODE_ANALOG;

    bool result = KC_CONN_IN_PIN->input_data;

    // Reset LED pins
    KC_INLED_GREEN_PIN->mode = GPIO_MODE_OUTPUT;
    
    return result;
}
static bool kc_out_connected() {
    // LED pins must by Hi-Z for this
    KC_OUTLED_GREEN_PIN->mode = GPIO_MODE_ANALOG;

    bool result = KC_CONN_OUT_PIN->input_data;

    // Reset LED pins
    KC_OUTLED_GREEN_PIN->mode = GPIO_MODE_OUTPUT;
    
    return result;
}