/*
 * Source Modified by Zubeen Tolani < ZeekHuge - zeekhuge@gmail.com >
 * Based on the examples distributed by TI
 *
 * Copyright (C) 2015 Texas Instruments Incorporated - http://www.ti.com/
 *
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *	* Redistributions of source code must retain the above copyright
 *	  notice, this list of conditions and the following disclaimer.
 *
 *	* Redistributions in binary form must reproduce the above copyright
 *	  notice, this list of conditions and the following disclaimer in the
 *	  documentation and/or other materials provided with the
 *	  distribution.
 *
 *	* Neither the name of Texas Instruments Incorporated nor the names of
 *	  its contributors may be used to endorse or promote products derived
 *	  from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
				  
#include <stdint.h>
#include <pru_cfg.h>
#include <pru_ctrl.h>
#include <pru_intc.h>
#include <rsc_types.h>
#include <pru_rpmsg.h>
#include "resource_table_0.h"

/* Host-1 Interrupt sets bit 31 in register R31 */
#define HOST_INT			((uint32_t) 1 << 30)

/* The PRU-ICSS system events used for RPMsg are defined in the Linux device tree
 * PRU0 uses system event 16 (To ARM) and 17 (From ARM)
 * PRU1 uses system event 18 (To ARM) and 19 (From ARM)
 */
#define TO_ARM_HOST			16
#define FROM_ARM_HOST			17

/*
 * Using the name 'rpmsg-pru' will probe the rpmsg_pru driver found
 * at linux-x.y.z/drivers/rpmsg/rpmsg_pru.c
 */
#define CHAN_NAME			"rpmsg-pru"
#define CHAN_DESC			"Channel 30"
#define CHAN_PORT			30

/*
 * Used to make sure the Linux drivers are ready for RPMsg communication
 * Found at linux-x.y.z/include/uapi/linux/virtio_config.h
 */
#define VIRTIO_CONFIG_S_DRIVER_OK	4


#define	INS_PER_US 200           // 5ns per instruction
#define INS_PER_DELAY_LOOP 2	 // two instructions per delay loop
#define DELAY_CYCLES_US (INS_PER_US / INS_PER_DELAY_LOOP)

#define CARRIER_PERIOD = (5264)
#define CARRIER_PULSE_WIDTH (2632)  // 38kHz = 26.32us * 200 cycles/us = 5264 cycles / signal period. signal is on for half of period -> /2



volatile register unsigned int __R30;
volatile register unsigned int __R31;
#define PRU0_GPIO (1<<2)

uint8_t payload[RPMSG_BUF_SIZE];


void mark(unsigned pulses){
	//for now this just makes one pulse
	while(pulses > 0){
		__R30 |= PRU0_GPIO;
		__delay_cycles(CARRIER_PULSE_WIDTH);
		__R30 ^= PRU0_GPIO;
		__delay_cycles(CARRIER_PULSE_WIDTH);
		pulses--;
	}
	
}

void space(unsigned pulses){
	__R30 &= ~PRU0_GPIO;
	while(pulses > 0){
		__delay_cycles(CARRIER_PULSE_WIDTH);
		__delay_cycles(CARRIER_PULSE_WIDTH);
		pulses--;
	}
}


void putBits(unsigned data, unsigned char nbits){
	unsigned char i;
	for (i = 0; i < nbits; i++) {
		//mark always 600us
        if (data & 0x8000) {//always 16 bits
        	//1 is space for 1600
          mark(23);  space(61);
        } else {
        	//0 is space for 460
          mark(23 ); space(17);
        };
        data <<= 1;
     }
}

unsigned parseCode(uint8_t * payload){
	unsigned code = 0;
	unsigned i;
	for(i = 1; i < 6; i++){
		code <<= 1;
		if(payload[i] == '1')code++;
	}
	return code;
}

void main(void) {
	//Initialize RPMsg
	struct pru_rpmsg_transport transport;
	uint16_t src, dst, len;
	volatile uint8_t *status;
	/* Allow OCP master port access by the PRU so the PRU can read external memories */
	CT_CFG.SYSCFG_bit.STANDBY_INIT = 0;
	/* Clear the status of the PRU-ICSS system event that the ARM will use to 'kick' us */
	CT_INTC.SICR_bit.STS_CLR_IDX = FROM_ARM_HOST;
	/* Make sure the Linux drivers are ready for RPMsg communication */
	status = &resourceTable.rpmsg_vdev.status;
	while (!(*status & VIRTIO_CONFIG_S_DRIVER_OK));
	/* Initialize the RPMsg transport structure */
	pru_rpmsg_init(&transport, &resourceTable.rpmsg_vring0, &resourceTable.rpmsg_vring1, TO_ARM_HOST, FROM_ARM_HOST);
	/* Create the RPMsg channel between the PRU and ARM user space using the transport structure. */
	while (pru_rpmsg_channel(RPMSG_NS_CREATE, &transport, CHAN_NAME, CHAN_DESC, CHAN_PORT) != PRU_RPMSG_SUCCESS);
	
	
	//wait for start signal from ARM
	waitForMessage:
	while(!(__R31 & HOST_INT));	
	/* Clear the event status */
	CT_INTC.SICR_bit.STS_CLR_IDX = FROM_ARM_HOST;
	while (pru_rpmsg_receive(&transport, &src, &dst, payload, &len) != PRU_RPMSG_SUCCESS) {
		//wait for start signal reception to work
	}
	/*
	if(len < 6 || payload[0] != 'b'){
		//did not get the start message
		goto waitForMessage;
	} 
	//Message is b##### where each # is a bit of the message to broadcast
	*/
	unsigned code = 0b00101;
	code = parseCode(payload);
	
	unsigned signal = 0x4080 | (code << 9) | (code << 1);
	
	//Actual beacon code ************************************************************
    
    //010* **** 10** ***0
    //0x4080 | (code << 9) | (code << 1)

    __R30 &= ~PRU0_GPIO;
    
    while(1){
    	mark(171);
    	space(171);
    	putBits(signal, 16);
    	
    	
    	if(__R31 & HOST_INT){
    		/* Clear the event status */
			CT_INTC.SICR_bit.STS_CLR_IDX = FROM_ARM_HOST;
			if (pru_rpmsg_receive(&transport, &src, &dst, payload, &len) == PRU_RPMSG_SUCCESS) {
				if(len >= 6 && payload[0] == 'b'){
					code = parseCode(payload);
					signal = 0x4080 | (code << 9) | (code << 1);
				}
			} else{
			  CT_INTC.SICR_bit.STS_CLR_IDX = FROM_ARM_HOST;
			}
			
		}
    	
    	
    	__delay_cycles(200000000);
    }
    
}
