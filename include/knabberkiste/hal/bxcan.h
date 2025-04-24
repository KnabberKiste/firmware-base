#pragma once

typedef struct {
    uint32_t id;
    bool id_extended;
    bool rtr;
    uint8_t dlc;
    uint8_t data[8];
} CAN_Frame_t;

void can_init();
void can_send_frame(CAN_Frame_t *frame);