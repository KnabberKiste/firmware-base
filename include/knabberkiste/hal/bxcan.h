/**
 * @file bxcan.h
 * @author Gabriel Heinzer
 * @brief HAL for bxCAN (basic and extendend controller area network) peripheral.
 * 
 * @section bxcan_initialization Initialization
 * Before accessing any bxCAN functionality, you must first initialize the peripheral
 * using @ref can_init(). This function takes the bitrate as an argument, which must
 * match the bitrate of the other CAN nodes. You can also configure a test mode if
 * required, but in most cases, @ref CAN_TESTMODE_NONE will probably be best.
 * 
 * After initializing the peripheral itself, you should also configure some identifier
 * filtering. Only messages which pass through at least one filter are acknowledged, which
 * means that messages which aren't acknowledged by at least one CAN node cause an error
 * on the sending node.
 * 
 * For the values you need to pass to @ref can_configure_filter_bank, please refer to the
 * reference manual RM0316 from STMicroelectronics.
 * 
 * @section bxcan_frame_transmission Frame transmission
 * Use @ref can_transmit_frame() to schedule a frame for tranmission. Messages are buffered
 * internally in a queue when they can't be transmitted immediately. Then, they are
 * immediately (and automatically) transmitted when the next transmit mailbox becomes
 * available.
 * 
 * @section bxcan_frame_reception Frame reception
 * Received frames are automatically converted in a @ref CAN_Frame_t structure and then passed
 * to @ref can_recv_callback(). This function should be defined by your application. It is called
 * for every frame in an interrupt context. Don't perform any lengthy processing in this callback,
 * as this blocks the whole microcontroller (because it's executed inside the ISR).
 * 
 * @section bxcan_error_handling Error handling
 * When an error occurs during transmission, @ref can_error_callback() is called. Note that this
 * may not be called for every frame which fails to get transmitted, but only once if an error
 * condition stays active.
 * 
 * The bxCAN HAL recovers automatically from errors, i.e. CAN frames will be transmitted once
 * the error condition is gone.
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <knabberkiste/io.h>

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
 * @brief Enumerator for the available CAN test mode flags. You may
 * OR these to enable multiple test modes simulateously.
 */
typedef enum {
    /// @brief Disables all test modes
    CAN_TESTMODE_NONE = 0,
    /// @brief Enables loopback mode.  Can be used in conjunction with @ref CAN_TESTMODE_SILENT.
    CAN_TESTMODE_LOOPBACK = CAN_BTR_LBKM,
    /// @brief Enables silent mode. Can be used in conjunction with @ref CAN_TESTMODE_LOOPBACK.
    CAN_TESTMODE_SILENT = CAN_BTR_SILM
} CAN_TestMode_t;

/**
 * @brief Enumerator for bxCAN filter numbers.
 */
typedef enum {
    /// @brief bxCAN filter bank 0
    CAN_FILTERBANK_0 = 0,
    /// @brief bxCAN filter bank 1
    CAN_FILTERBANK_1,
    /// @brief bxCAN filter bank 2
    CAN_FILTERBANK_2,
    /// @brief bxCAN filter bank 3
    CAN_FILTERBANK_3,
    /// @brief bxCAN filter bank 4
    CAN_FILTERBANK_4,
    /// @brief bxCAN filter bank 5
    CAN_FILTERBANK_5,
    /// @brief bxCAN filter bank 6
    CAN_FILTERBANK_6,
    /// @brief bxCAN filter bank 8
    CAN_FILTERBANK_8,
    /// @brief bxCAN filter bank 9
    CAN_FILTERBANK_9,
    /// @brief bxCAN filter bank 10
    CAN_FILTERBANK_10,
    /// @brief bxCAN filter bank 11
    CAN_FILTERBANK_11,
    /// @brief bxCAN filter bank 12
    CAN_FILTERBANK_12,
    /// @brief bxCAN filter bank 13
    CAN_FILTERBANK_13
} CAN_FilterBank_t;

/**
 * @brief Enumerator for bxCAN filter bank width.
 */
typedef enum {
    /// @brief Two 16-bit filters for the STDID[10:0], RTR, IDE, and EXTID[17:15] bits
    CAN_FILTERBANK_WIDTH_16BIT = 0,
    /// @brief One 32-bit filter for the STDID[10:0], EXTID[17:0], IDE, and RTR bits
    CAN_FILTERBANK_WIDTH_32BIT = 1
} CAN_FilterBankWidth_t;

/**
 * @brief Enumerator for bxCAN filter bank width.
 */
typedef enum {
    /**
     * @brief In mask mode the identifier registers are associated with mask registers specifying which
     * bits of the identifier are handled as “must match” or as “don’t care”.
     */
    CAN_FILTERBANK_MODE_MASK = 0,
    /**
     * @brief In identifier list mode, the mask registers are used as identifier registers. Thus, instead of
     * defining an identifier and a mask, two identifiers are specified, doubling the number of single
     * identifiers. All bits of the incoming identifier must match the bits specified in the filter
     * registers.
     */
    CAN_FILTERBANK_MODE_LIST = 1
} CAN_FilterBankMode_t;

/**
 * @brief Enumerator for bxCAN FIFO numbers.
 */
typedef enum{
    CAN_FIFO_0 = 0,
    CAN_FIFO_1 = 1
} CAN_FIFO_t;

/**
 * @brief Structure representing a single received CAN frame. Note that
 * this has some additional data members compared to CAN_Frame_t.
 */
typedef struct {
    /// @brief CAN frame which was received.
    CAN_Frame_t frame;
    /// @brief FIFO in which the CAN frame was stored.
    CAN_FIFO_t fifo;
    /// @brief Identifier filter which matched the CAN frame.
    CAN_FilterBank_t filter_match;
} CAN_ReceivedFrame_t;

/**
 * @brief Enumerator for all possible CAN errors codes.
 */
typedef enum {
    /// @brief No error has been detected.
    CAN_ERR_NONE = 0,
    /// @brief Bit stuffing error.
    CAN_ERR_STUFF_ERROR = 0b001,
    /// @brief Frame form error.
    CAN_ERR_FORM_ERROR = 0b010,
    /// @brief Frame was not acknowledged.
    CAN_ERR_ACKNOWLEDGEMENT_ERROR = 0b011,
    /// @brief Recessive bit could not be sent.
    CAN_ERR_BIT_RECESSIVE_ERROR = 0b100,
    /// @brief Dominant bit could not be sent.
    CAN_ERR_BIT_DOMINANT_ERROR = 0b101,
    /// @brief CRC invalid.
    CAN_ERR_CRC_ERROR = 0b110,
    /// @brief Software-set error.
    CAN_ERR_SOFTWARE_ERROR = 0b111,
    /// @brief FIFO0 has been overrun. Messages have been lost.
    CAN_ERR_FIFO0_OVERRUN,
    /// @brief FIFO1 has been overrun. Messages have been lost.
    CAN_ERR_FIFO1_OVERRUN
} CAN_ErrorCode_t;

/**
 * @brief Initializes the CAN peripheral.
 * 
 * @warning You must first initalize the CAN TX and CAN RX pins with their alternate
 * function mapping, otherwise this initialization procedure will never return.
 * 
 * @param bitrate Bitrate, in bits per second, to initialize the peripheral with.
 * @param testMode Test mode flags to enable.
 */
void can_init(
    uint32_t bitrate,
    CAN_TestMode_t testMode
);

/**
 * @brief Configures the specified filter bank.
 * 
 * @param filter The filter bank to configure.
 * @param fifo FIFO to which the filter bank will be assigned to.
 * @param width Width of the filter bank. Refer to the reference manual for more information.
 * @param mode Mode of the filter bank. Refer to the reference manual for more information.
 * @param FxR1_val Value of the first filter register. Refer to the reference manual for more information.
 * @param FxR2_val Value of the first filter register. Refer to the reference manual for more information.
 */
void can_configure_filter_bank(
    CAN_FilterBank_t filter,
    CAN_FIFO_t fifo,
    CAN_FilterBankWidth_t width,
    CAN_FilterBankMode_t mode,
    uint32_t FxR1_val,
    uint32_t FxR2_val
);

/**
 * @brief Transmit a single CAN frame. The CAN frame may be buffered internally
 * if the no transmit mailbox is available.
 * 
 * @param frame The frame to send.
 */
void can_transmit_frame(CAN_Frame_t* frame);

/**
 * @brief Callback function called when a frame has been received. This function is 
 * bound weakly internally, i.e. can, or should, be implemented by the application.
 * 
 * @param frame Frame which was received.
 */
void can_recv_callback(CAN_ReceivedFrame_t frame) __attribute__((weak));

/**
 * @brief Callback function called when an error occurred in the CAN peripheral.
 * This function is bound weakly internally, i.e. can, or should, be implemented
 * by the application.
 * 
 * @param error_code Error code which caused the interrupt.
 */
void can_error_callback(CAN_ErrorCode_t error_code) __attribute__((weak));