#include <knabberkiste/hal/bxcan.h>
#include <knabberkiste/hal/vcp_debug.h>
#include <knabberkiste/hal/gpio.h>
#include <knabberkiste/hal/clock.h>
#include <knabberkiste/util/bit_manipulation.h>
#include <knabberkiste/util/fifo.h>
#include <knabberkiste/util/error.h>
#include <knabberkiste/io.h>
#include <string.h>
#include <math.h>

#define BXCAN_TX_QUEUE_SIZE 4

fifo_declare_qualifier(CAN_Frame_t, bxcan_tx_queue, BXCAN_TX_QUEUE_SIZE, static);

// Internal functions
CAN_ReceivedFrame_t can_read_frame_from_fifo(CAN_FIFO_t fifo);

// Interrupt handlers
void USB_HP_CAN_TX_IRQHandler() {
    // Transmit interrupt, this is fired by the following bits being set:
    //  - TSR -> RQCP0 (Request completed mailbox 0)
    //  - TSR -> RQCP1 (Request completed mailbox 1)
    //  - TSR -> RQCP2 (Request completed mailbox 2)
    // Of course, this is dependent on the enabled interrupts

    // Clear the RQCPx and TXOKx bits
    CLEAR_MASK(CAN->TSR, CAN_TSR_RQCP0 | CAN_TSR_RQCP1 | CAN_TSR_RQCP2);
    CLEAR_MASK(CAN->TSR, CAN_TSR_TXOK0 | CAN_TSR_TXOK1 | CAN_TSR_TXOK2);

    // Transmit the next message in the transmit queue
    if(!fifo_empty(bxcan_tx_queue)) {
        CAN_Frame_t nextFrame;
        fifo_get(bxcan_tx_queue, nextFrame);
        can_transmit_frame(&nextFrame);
    }
}

void USB_LP_CAN_RX0_IRQHandler() {
    // FIFO 0 receive interrupt, this is fired by the following bits being set:
    //  - RF0R -> FMP0  (FIFO 0 message pending)
    //  - RF0R -> FULL0 (FIFO 0 full)
    //  - RF0R -> FOVR0 (FIFO 0 overrun)
    // Of course, this is dependent on the enabled interrupts. See can_init
    // for more information.

    if(READ_MASK(CAN->RF0R, CAN_RF0R_FMP0)) {
        CAN_ReceivedFrame_t frame = can_read_frame_from_fifo(CAN_FIFO_0);

        can_recv_callback(frame);

        // Clear the FMP1 bit
        CLEAR_MASK(CAN->RF0R, CAN_RF0R_FMP0);
    } else if(READ_MASK(CAN->RF0R, CAN_RF0R_FOVR0)) {
        can_error_callback(CAN_ERR_FIFO0_OVERRUN);
    }
}

void CAN_RX1_IRQHandler() {
    // FIFO 1 receive interrupt, this is fired by the following bits being set:
    //  - RF1R -> FMP1  (FIFO 1 message pending)
    //  - RF1R -> FULL1 (FIFO 1 full)
    //  - RF1R -> FOVR1 (FIFO 1 overrun)
    // Of course, this is dependent on the enabled interrupts. See can_init
    // for more information.
    
    if(READ_MASK(CAN->RF1R, CAN_RF1R_FMP1)) {
        CAN_ReceivedFrame_t frame = can_read_frame_from_fifo(CAN_FIFO_1);

        can_recv_callback(frame);

        // Clear the FMP1 bit
        CLEAR_MASK(CAN->RF1R, CAN_RF1R_FMP1);
    } else if(READ_MASK(CAN->RF1R, CAN_RF1R_FOVR1)) {
        can_error_callback(CAN_ERR_FIFO1_OVERRUN);
    }
}

void CAN_SCE_IRQHandler() {
    // Status change/error interrupt, this is fired by the following bits being set:
    //  - ESR -> EWGF (Error warning flag interrupt, receive error counter >= 96 or transmit error counter >= 96)
    //  - ESR -> EPVF (Error passive flag interrupt, passive limit has been reached: receive/transmit error counter > 127)
    //  - ESR -> BOFF (Buss-off flag interrupt, transmit error counter > 255)
    //  - ESR -> LEC (Some kind of error has been detected)
    //  - MSR -> WKUI (Wake-up interrupt)
    //  - MSR -> SLAKI (Sleep acknowledge interrupt)
    // Of course, this is dependent on the enabled interrupts
    CAN_ErrorCode_t error_code = READ_MASK_OFFSET(CAN->ESR, 0b111, CAN_ESR_LEC_Pos);

    // SLAKI and WKUI are ignored
    if(error_code) {
        can_error_callback(error_code);
    }

    // Clear the interrupt flag
    SET_MASK(CAN->MSR, CAN_MSR_ERRI);
}

void can_init(uint32_t bitrate, CAN_TestMode_t testMode) {
    // Enable the clock for the bxCAN peripheral
    SET_MASK(RCC->APB1ENR, RCC_APB1ENR_CANEN);

    // Reset bxCAN peripheral
    SET_MASK(RCC->APB1RSTR, RCC_APB1RSTR_CANRST);
    CLEAR_MASK(RCC->APB1RSTR, RCC_APB1RSTR_CANRST);

    // Disable bxCAN sleep mode
    CLEAR_MASK(CAN->MCR, CAN_MCR_SLEEP);

    // Enable bxCAN initialization mode
    SET_MASK(CAN->MCR, CAN_MCR_INRQ); // Trigger initialization request
    while(!READ_MASK(CAN->MSR, CAN_MSR_INAK)); // Wait for initialization acknowledgement

    // Enable automatic bus-off recovery
    SET_MASK(CAN->MCR, CAN_MCR_ABOM);

    // Disable CAN debug freeze
    SET_BIT(CAN->MCR, 16);

    // Set transmit priority be request order
    SET_MASK(CAN->MCR, CAN_MCR_TXFP);

    // Enable receive FIFO locked mode
    SET_MASK(CAN->MCR, CAN_MCR_RFLM);

    // Enable interrutps
    SET_MASK(CAN->IER, CAN_IER_ERRIE); // Error interrupt enable
    SET_MASK(CAN->IER, CAN_IER_LECIE); // Last error code interrupt enable
    CLEAR_MASK(CAN->IER, CAN_IER_BOFIE); // Bus-off interrupt enable
    CLEAR_MASK(CAN->IER, CAN_IER_EPVIE); // Error passive interrupt enable
    CLEAR_MASK(CAN->IER, CAN_IER_EWGIE); // Error warning interrupt enable
    SET_MASK(CAN->IER, CAN_IER_FOVIE1); // FIFO 1 overrun interrupt enable
    SET_MASK(CAN->IER, CAN_IER_FMPIE1); // FIFO 1 message pending interrupt enable
    CLEAR_MASK(CAN->IER, CAN_IER_FFIE1); // FIFO 1 full interrupt enable
    SET_MASK(CAN->IER, CAN_IER_FOVIE0); // FIFO 0 overrun interrupt enable
    SET_MASK(CAN->IER, CAN_IER_FMPIE0); // FIFO 0 message pending interrupt enable
    CLEAR_MASK(CAN->IER, CAN_IER_FFIE0); // FIFO 0 full interrupt enable
    SET_MASK(CAN->IER, CAN_IER_TMEIE); // Transmit mailbox empty interrupt enable

    // Enable interrupts
    NVIC_EnableIRQ(CAN_TX_IRQn);
    NVIC_EnableIRQ(CAN_RX0_IRQn);
    NVIC_EnableIRQ(CAN_RX1_IRQn);
    NVIC_EnableIRQ(CAN_SCE_IRQn);

    // Configure test mode
    SET_MASK(CAN->BTR, testMode);

    // Set a sample point of 87.5%
    WRITE_MASK_OFFSET(CAN->BTR, 0b1111, 5, CAN_BTR_TS1_Pos);
    WRITE_MASK_OFFSET(CAN->BTR, 0b111, 0, CAN_BTR_TS2_Pos);

    // Set synchronization jump
    WRITE_MASK_OFFSET(CAN->BTR, 0b11, 0, CAN_BTR_SJW_Pos);
    
    // Configure bitrate
    uint8_t ts1_val = READ_MASK_OFFSET(CAN->BTR, 0b1111, CAN_BTR_TS1_Pos);
    uint8_t ts2_val = READ_MASK_OFFSET(CAN->BTR, 0b111, CAN_BTR_TS2_Pos);
    float time_quantum_frequency = bitrate * (3 + ts1_val + ts2_val);

    float brp_val_float = (clock_getAPB1Frequency() / time_quantum_frequency) - 1;
    uint32_t brp_val = round(brp_val_float);
    WRITE_MASK_OFFSET(CAN->BTR, 0b111111111, brp_val, CAN_BTR_BRP_Pos);

    // Enable bxCAN
    CLEAR_MASK(CAN->MCR, CAN_MCR_INRQ);
    while(READ_MASK(CAN->MSR, CAN_MSR_INAK)); // Wait for initialization acknowledgement

    // Enable IRQs, needed for CAN
    __enable_irq();
}

void can_configure_filter_bank(
    CAN_FilterBank_t filter,
    CAN_FIFO_t fifo,
    CAN_FilterBankWidth_t width,
    CAN_FilterBankMode_t mode,
    uint32_t FxR1_val,
    uint32_t FxR2_val
) {
    // Enter filter initialization mode
    SET_MASK(CAN->FMR, CAN_FMR_FINIT);

    // Set the filter's mode
    WRITE_BIT(CAN->FM1R, filter, mode);

    // Set the filter's width
    WRITE_BIT(CAN->FS1R, filter, width);

    // Assign the filter to it's FIFO
    WRITE_BIT(CAN->FFA1R, filter, fifo);

    // Activate the filter
    SET_BIT(CAN->FA1R, filter);

    // Write the filter value
    CAN->sFilterRegister[filter].FR1 = FxR1_val;
    CAN->sFilterRegister[filter].FR2 = FxR2_val;

    // Leave filter initialization mode
    CLEAR_MASK(CAN->FMR, CAN_FMR_FINIT); 
}

void can_transmit_frame(CAN_Frame_t* frame) {
    // Check if a mailbox is free
    bool mailbox_free = READ_MASK(CAN->TSR, CAN_TSR_TME);

    if(mailbox_free) {
        __disable_irq();
        uint8_t mailbox_id = READ_MASK_OFFSET(CAN->TSR, 0b11, CAN_TSR_CODE_Pos);
        CAN_TxMailBox_TypeDef* mailbox = &(CAN->sTxMailBox[mailbox_id]);

        // Set the identifier register
        WRITE_BIT(mailbox->TIR, CAN_TI0R_RTR_Pos, frame->rtr);
        WRITE_BIT(mailbox->TIR, CAN_TI0R_IDE_Pos, frame->id_extended);

        if(frame->id_extended) {
            // Configure the extended ID
            WRITE_MASK_OFFSET(mailbox->TIR, 0x3FFFF, frame->id, CAN_TI0R_EXID_Pos);
        } else {
            // Configure the standard ID
            WRITE_MASK_OFFSET(mailbox->TIR, 0x7FF, frame->id, CAN_TI0R_STID_Pos);
        }

        // Set the data length control register
        WRITE_MASK_OFFSET(mailbox->TDTR, 0xF, frame->dlc, CAN_TDT0R_DLC_Pos);

        // Set the data low register
        mailbox->TDLR = ((uint32_t*)(frame->data))[0];
        mailbox->TDHR = ((uint32_t*)(frame->data))[1];

        // Set the transmit request bit
        SET_MASK(mailbox->TIR, CAN_TI0R_TXRQ);
        __enable_irq();
    } else {
        // Append the frame to the transmit queue
        fifo_put(bxcan_tx_queue, *frame);
    }
}

CAN_ReceivedFrame_t can_read_frame_from_fifo(CAN_FIFO_t fifo) {
    CAN_FIFOMailBox_TypeDef* mailbox = &(CAN->sFIFOMailBox[fifo]);

    // Read the CAN frame
    CAN_ReceivedFrame_t recv_frame;

    // Read the receiving information
    recv_frame.fifo = fifo;
    recv_frame.filter_match = READ_MASK_OFFSET(mailbox->RDTR, 0xFF, CAN_RDT0R_FMI_Pos);

    // Read the CAN frame itself
    recv_frame.frame.dlc = READ_MASK_OFFSET(mailbox->RDTR, 0b1111, CAN_RDT0R_DLC_Pos);

    recv_frame.frame.rtr = READ_MASK_OFFSET(mailbox->RIR, 0b1, CAN_RI0R_RTR_Pos);
    recv_frame.frame.id_extended = READ_MASK_OFFSET(mailbox->RIR, 0b1, CAN_RI0R_IDE_Pos);
    if(recv_frame.frame.id_extended) {
        recv_frame.frame.id = READ_MASK_OFFSET(mailbox->RIR, 0x3FFFF, CAN_RI0R_EXID_Pos);
    } else {
        recv_frame.frame.id = READ_MASK_OFFSET(mailbox->RIR, 0x7FF, CAN_RI0R_STID_Pos);
    }

    ((uint32_t*)(recv_frame.frame.data))[0] = mailbox->RDLR;
    ((uint32_t*)(recv_frame.frame.data))[1] = mailbox->RDHR;

    // Release the mailbox
    if(fifo == CAN_FIFO_0) {
        SET_MASK(CAN->RF0R, CAN_RF0R_RFOM0);
    } else {
        SET_MASK(CAN->RF1R, CAN_RF1R_RFOM1);
    }

    return recv_frame;
}