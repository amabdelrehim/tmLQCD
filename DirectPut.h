/***********************************************************************
 * Copyright (C) 2002,2003,2004,2005,2006,2007,2008 Carsten Urbach
 *
 * This file is part of tmLQCD.
 *
 * tmLQCD is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * tmLQCD is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with tmLQCD.  If not, see <http://www.gnu.org/licenses/>.
 ***********************************************************************/

// Basic SPI and HWI includes
#include <hwi/include/bqc/A2_core.h>
#include <hwi/include/bqc/A2_inlines.h>
#include <hwi/include/bqc/MU_PacketCommon.h>
#include <firmware/include/personality.h>
#include <spi/include/mu/Descriptor.h>
#include <spi/include/mu/Descriptor_inlines.h>
#include <spi/include/mu/InjFifo.h>
#include <spi/include/mu/Addressing.h>
#include <spi/include/mu/Addressing_inlines.h>
#include <spi/include/mu/GIBarrier.h>
#include <spi/include/kernel/MU.h>
#include <spi/include/kernel/process.h>
#include <spi/include/kernel/location.h>

#define NUM_DIRS               8

// pointers to send and receive buffers
extern uint64_t messageSizes[NUM_DIRS];
extern uint64_t roffsets[NUM_DIRS], soffsets[NUM_DIRS];
extern uint64_t totalMessageSize;

extern char * SPIrecvBuffers;
extern char * SPIsendBuffers;
extern MUHWI_Descriptor_t *SPIDescriptors;

// receive counter
extern volatile uint64_t recvCounter;

// counter for injected messages
extern uint64_t descCount[NUM_DIRS];

// Fifo handles
extern msg_InjFifoHandle_t injFifoHandle;

// get the destinations for all neighbours
// will be saved in nb2dest
int get_destinations(int * mypers);

// Call to create the descriptors for all eight directions
void create_descriptors();

// Call to set up the base address table id and memory regions
void setup_mregions_bats_counters();

// global barrier using GIBarrier
MUSPI_GIBarrier_t GIBarrier;
void global_barrier();

/**
 * \brief Injection Fifo Handle
 *
 * This is a "handle" returned from msg_InjFifoInit() and passed into subsequent
 * calls to msg_InjFifoXXXX() functions.  It is used internally within the
 * msg_InjFifoXXXX() functions to anchor resources that have been allocated.
 */
typedef struct {
  void* pOpaqueObject;
} msg_InjFifoHandle_t;


int msg_InjFifoInit ( msg_InjFifoHandle_t *injFifoHandlePtr,
                      uint32_t             startingSubgroupId,
                      uint32_t             startingFifoId,
                      uint32_t             numFifos,
                      size_t               fifoSize,
                      Kernel_InjFifoAttributes_t  *injFifoAttrs );


/**
 * \brief Terminate Injection Fifos
 * 
 * Terminate the usage of injection fifos.  This deactivates the fifos and
 * frees all of the storage associated with them (previously allocated during
 * msg_InjFifoInit()).
 * 
 * \param [in]  injFifoHandle  The handle returned from msg_InjFifoInit().
 *                             It must be passed into this function untouched
 *                             from when it was returned from msg_InjFifoInit().
 *
 * \note After this function returns, no more InjFifo functions should be called
 *       with this injFifoHandle.
 */
void msg_InjFifoTerm ( msg_InjFifoHandle_t injFifoHandle );


/**
 * \brief Inject Descriptor into Injection Fifo
 * 
 * Inject the specified descriptor into the specified injection fifo.
 * 
 * \param [in]  injFifoHandle  The handle returned from msg_InjFifoInit().
 *                             It must be passed into this function untouched
 *                             from when it was returned from msg_InjFifoInit().
 * \param [in]  relativeFifoId  The fifo number, relative to the start of
 *                              the fifos managed by this opaque object.
 *                              For example, if msg_InjFifoInit() was called
 *                              to init fifos in subgroup 2, starting with
 *                              fifo Id 3, the relativeFifoNumber of the
 *                              first fifo is 0, not 3.
 * \param [in]  descPtr         Pointer to the descriptor to be injected.
 *
 * \retval  positiveNumber  The descriptor was successfully injected.  The
 *                          returned value is the sequence number of this 
 *                          descriptor.
 * \retval  -1              The descriptor was not injected, most likely because
 *                          there is no room in the fifo.
 */
uint64_t msg_InjFifoInject ( msg_InjFifoHandle_t injFifoHandle,
                             uint32_t            relativeFifoId,
                             MUHWI_Descriptor_t *descPtr );

