/**
 * @file bxcan.h
 * @author Gabriel Heinzer
 * @brief HAL for bxCAN (basic and extendend controller area network) peripheral.
 */

#pragma once

/**
 * @brief Structure representing a single CAN frame.
 */
typedef struct {
    /// @brief Arbitration identifier of the CAN frame.
    uint32_t id;
    /// @brief Whether the CAN frame uses an extended indentifier or not.
    bool id_extended;
    /// @brief Remote-transmission-request bit.
    bool rtr;
    /// @brief Data-length-code, amount of data being transmitted.
    uint8_t dlc;
    /// @brief Data transmitted with the can frame.
    uint8_t data[8];
} CAN_Frame_t;

/**
 * @brief Initializes the CAN peripheral.
 */
void can_init();

/**
 * @brief Sends a single CAN frame.
 * 
 * @param frame The frame to send.
 */
void can_send_frame(CAN_Frame_t *frame);