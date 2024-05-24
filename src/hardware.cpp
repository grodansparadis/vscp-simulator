/////////////////////////////////////////////////////////////////////////////
// hardware.cpp
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
//
// This file is part of the VSCP (https://www.vscp.org)
//
// Copyright:  (C) 2000-2024
// Ake Hedman, the VSCP project, <info@vscp.org>
//
// This file is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this file see the file COPYING.  If not, write to
// the Free Software Foundation, 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.

/*
   ----------------------------------------------------------------------------
    This file contains the hardware callbacks for the standard
    VSCP bootloader code and callbacks for the standard VSCP firmware code.
   ----------------------------------------------------------------------------

  We can be in two conditions in any moment.
    * Bootloader mode. Firmware can be loaded. We can go to firmware mode by
                        write a bootflag and reboot.
    * Firmware mode. A simulated hardware running as an app. - We can go to
                        the bootloader by issue VSCP events to do so.
*/

#include <crc.h>
#include <guid.h>
#include <vscp-bootloader.h>
#include <vscp-firmware-level2.h>
#include <vscp.h>

#include "btest.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//
//                            Bootloader callbacks
//
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// goApplication
//
// This code jumps to the application. Typically this is code
//  like
//
//  void (*foo)(void) = (void (*)())0x12345678;
//  foo();
//
//  where 0x12345678 is the address to jump to.
//

void
vscpboot_goApplication(void)
{
  btest* pbtest = (btest*)QApplication::instance();
  pbtest->vscpboot_goApplication();
}

int
vscpboot_run(void)
{
  btest* pbtest = (btest*)QApplication::instance();
  return pbtest->m_bRun;
}

/*!
  Initialize hardware. This should be the first method called.
  The communication interface should be opended here and the system
  should be ready to receive firmware code events when done.
*/
int
vscpboot_init_hardware(void)
{
  btest* pbtest = (btest*)QApplication::instance();
  return pbtest->vscpboot_init_hardware();
}

/*!
  Free all used hardware resources
*/
int
vscpboot_release_hardware(void)
{
  btest* pbtest = (btest*)QApplication::instance();
  return pbtest->vscpboot_release_hardware();
}

/*!
  Get Device bootflag from persistent storage.
  @return Bootflag for node. Zero means application should start. Nonzero
    means bootloader should start. Application can use other values for
    bootflag or multibyte storage as long as this routine report expected state.
*/
uint8_t
vscpboot_getBootFlag(void)
{
  btest* pbtest = (btest*)QApplication::instance();
  return pbtest->vscpboot_getBootFlag();
}

/*!
  Set Device bootflag in persistent storage. Application can use other values for
  bootflag or multibyte storage as long as this routine set proper state.
  @param bootflag. New bootflag to set. Zero means application should start. Nonzero
    means bootloader should start.
*/
int
vscpboot_setBootFlag(uint8_t bootflag)
{
  btest* pbtest = (btest*)QApplication::instance();
  return pbtest->vscpboot_setBootFlag(bootflag);
}

/*!
  Reboot the board. This method should never return.
*/
void
vscpboot_reboot(void)
{
  btest* pbtest = (btest*)QApplication::instance();
  pbtest->vscpboot_reboot();
}

vscpboot_config_t*
vscpboot_getConfig(void)
{
  btest* pbtest = (btest*)QApplication::instance();
  return pbtest->vscpboot_getConfig();
}

/*!
  Get GUID for device.
  On some devices GUID may be programmed by the firmware load
  process. This is typical for devices that have a id/userid
  in flash. For devices using Ethernet etc construct GUID in
  the usual manner.
  @return Pointer to GUID
*/
uint8_t*
vscpboot_getGUID(void)
{
  btest* pbtest = (btest*)QApplication::instance();
  return pbtest->vscpboot_getGUID();
}

/*!
  Check if memtype is valid. type == 0 is always valid
  @return non zero if valid
*/
int
vscpboot_isMemTypeValid(uint8_t type)
{
  btest* pbtest = (btest*)QApplication::instance();
  return pbtest->vscpboot_isMemTypeValid(type);
}

/*!
  Check if membank is valid. type == 0 is always valid
  @return non zero if valid
*/
int
vscpboot_isMemBankValid(uint8_t bank)
{
  btest* pbtest = (btest*)QApplication::instance();
  return pbtest->vscpboot_isMemBankValid(bank);
}

/*!
  Program a block
  @param pblock Pointer to the block to program
  @param type Memory type
  @param bank Memory bank to program
  @return VSCP_ERROR_SUCCESS on success and else errocode
*/
int
vscpboot_programBlock(const uint8_t* pblock, uint8_t type, uint8_t bank)
{
  btest* pbtest = (btest*)QApplication::instance();
  return pbtest->vscpboot_programBlock(pblock, type, bank);
}

/*!
  The CRC for the loaded data is calculated here. This is the CRC
  over all blocks programmed calulated with CRC-CCITT. For a successful
  programming this value should be the same as the one provided in the
  activate new image event.
  @return crc CRC-CCITT for programmed area.
*/
crc
vscpboot_calcPrgCrc(void)
{
  btest* pbtest = (btest*)QApplication::instance();
  return pbtest->vscpboot_calcPrgCrc();
}

/*!
  Send VSCP event
  @param pex Pointer to VSCP event ex.
  @return VSCP_ERROR_SUCCESS on success
*/
int
vscpboot_sendEventEx(vscpEventEx* pex)
{
  btest* pbtest = (btest*)QApplication::instance();
  return pbtest->vscpboot_sendEventEx(pex);
}

/*!
  Block until VSCP event is available or app. quits.
  -----------------------------------------------------------
  IMPORTANT!
  This routine should translate all VSCP_CLASS2_LEVEL1_PROTOCOL
  events to VSCP_CLASS1_PROTOCOL events.
  -----------------------------------------------------------
  @param Pointer to VSCP event structure.
   @return VSCP_ERROR_SUCCESS on success
*/
int
vscpboot_getEventEx(vscpEventEx* pex)
{
  btest* pbtest = (btest*)QApplication::instance();
  return pbtest->vscpboot_getEventEx(pex);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//
//                            Firmware callbacks
//
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/*!
 * @brief Get the time in milliseconds.
 *
 * @param pdata Pointer to user data (typical points to context)
 * @param ptime Pointer to unsigned integer that will get the time in milliseconds.
 * @return VSCP_ERROR_SUCCESS on success, else error code.
 */

int
vscp_frmw2_callback_get_ms(void* const puserdata, uint32_t* ptime)
{
  // Check pointers
  if (nullptr == ptime) {
    return VSCP_ERROR_INVALID_POINTER;
  }

  *ptime = vscp_getMsTimeStamp();
  return VSCP_ERROR_SUCCESS;
}


/*!
  @brief Get name of the device.  Used by the device capabilities report.

  @param pdata Pointer to user data (typical points to context)
  @param pname Pointer to device name. The name should be no more than 64
  characters including terminating zero.
  @return VSCP_ERROR_SUCCESS on success, or error code.
*/
int
vscp_frmw2_callback_get_device_name(void* const puserdata, const char* pname)
{
  return VSCP_ERROR_SUCCESS;
}

/*!
 * @brief Read user register
 *
 * Read value of one of the user registers. Valid register range
 * is 0x00000000 - 0xffff0000
 *
 * @param pdata Pointer to user data (typical points to context).
 * @param reg Register to read.
 * @param pval Pointer to variable that will get read value.
 * @return VSCP_ERROR_SUCCESS on success, else error code. VSCP_ERROR_PARAMETER is
 * returned if the register is invalid.
 *
 */
int
vscp_frmw2_callback_read_reg(void* const puserdata, uint16_t page, uint32_t reg, uint8_t* pval)
{
  btest* pbtest = (btest*)QApplication::instance();

  // Check pointers
  if (nullptr == pval) {
    return VSCP_ERROR_INVALID_POINTER;
  }

  return pbtest->readRegister(0, reg, pval);
}

/*!
 * @brief Write user register
 *
 * Write value of one of the user registers. Valid register range
 * is 0x00000000 - 0xffff0000
 *
 * @param pdata Pointer to user data (typical points to context)
 * @param reg Register to write
 * @param val Value to write
 * @return VSCP_ERROR_SUCCESS on success, else error code. VSCP_ERROR_PARAMETER is
 * returned if the register is invalid.
 */

int
vscp_frmw2_callback_write_reg(void* const puserdata, uint16_t page, uint32_t reg, uint8_t val)
{
  btest* pbtest = (btest*)QApplication::instance();
  return pbtest->writeRegister(page, reg, val);
}

/*!
  @brief Send event to transport sublayer.

  @param pdata Pointer to user data (typical points to context)
  @param pex Pointer to Event to send.
  @return VSCP_ERROR_SUCCESS on success, or error code.

  - ev is taken over in the callback and it is responsible for releasing it.
  - if GUID is all zero then the GUID is set to the GUID of the node.
*/

int
vscp_frmw2_callback_send_event(void* const puserdata, vscpEvent* pev)
{
  // Check pointers
  if (nullptr == pev) {
    return VSCP_ERROR_INVALID_POINTER;
  }
  vscpEventEx ex;
  vscp_convertEventToEventEx(&ex, pev);

  return vscpboot_sendEventEx(&ex);
}

/*!
  @brief Send eventex to transport sublayer.

  @param pdata Pointer to user data (typical points to context)
  @param ex Pointer to EventEx to send.
  @return VSCP_ERROR_SUCCESS on success, or error code.

  - ex is copied in the callback.
  - if GUID is all zero then the GUID is set to the GUID of the node.
*/

int
vscp_frmw2_callback_send_event_ex(void* const puserdata, vscpEventEx* pex)
{
  // Check pointers
  if (nullptr == pex) {
    return VSCP_ERROR_INVALID_POINTER;
  }

  return vscpboot_sendEventEx(pex);
}

/*!
 * @brief Enter bootloader. SHOULD NEVER RETURN!!!
 *
 * @param pdata Pointer to user data (typical points to context).
 */

void
vscp_frmw2_callback_enter_bootloader(void* const puserdata)
{
  btest* pbtest = (btest*)QApplication::instance();
  vscpboot_setBootFlag(0);
  vscpboot_reboot();
}

/*!
 * @brief Reply with DM content.
 *
 * @param pdata Pointer to user data (typical points to context).
 * @return VSCP_ERROR_SUCCESS on success, or error code.
 *
 * Report full content of DM back.
 */

int
vscp_frmw2_callback_report_dmatrix(void* const puserdata)
{
  btest* pbtest = (btest*)QApplication::instance();
  pbtest->reportDM();
  return VSCP_ERROR_SUCCESS;
}



/*!
 * @brief Report back events that this node is interested in
 *
 * @param pdata Pointer to user data (typical points to context)
 * @return VSCP_ERROR_SUCCESS on success, else error code.
 */

int
vscp_frmw2_callback_report_events_of_interest(void* const puserdata)
{
  btest* pbtest = (btest*)QApplication::instance();
  pbtest->reportEventsOfInterest();
  return VSCP_ERROR_SUCCESS;
}

/*!
 * @brief Get timestamp
 *
 * @param pdata Pointer to context.
 * @return Timestamp in microseconds.
 */

uint32_t
vscp_frmw2_callback_get_timestamp(void* const puserdata)
{
  uint32_t ts = vscp_makeTimeStamp();
  return ts;
}

/*!
 * @brief  Fill in event date/time information
 *
 * @param pdata
 * @param pex Event to get info
 * @return VSCP_ERROR_SUCCESS on success, else error code.
 *
 * Set all to zero or do nothing if not time information is available
 * and the time values are already set to zero.
 */

int
vscp_frmw2_callback_get_time(void* const puserdata, vscpEventEx* pex)
{
  // Check pointers
  if (nullptr == pex) {
    return VSCP_ERROR_INVALID_POINTER;
  }

  time_t t     = time(NULL);
  struct tm tm = *localtime(&t);
  vscp_setEventExDateTime(pex, &tm);
  return VSCP_ERROR_SUCCESS;
}

/*!
  @brief Initialize persistent storage

  @param pdata Pointer to user data (typical points to context)
 */
int
vscp_frmw2_callback_init_persistent_storage(void* const puserdata)
{
  return VSCP_ERROR_SUCCESS;
}

/*!
  @brief Feed the decision matrix with one Event

  @param pdata Pointer to user data (typical points to context)
  @param ev Event to feed the DM with.
  @return VSCP_ERROR_SUCCESS on success, or error code.
*/
int
vscp_frmw2_callback_feed_dm(void* const puserdata, vscpEventEx* ev)
{
  return VSCP_ERROR_SUCCESS;
}

/*!
  @brief All events except level I/II protocol events is sent to the
  application for handling.

  @param pdata Pointer to user data (typical points to context)
  @param pex Event to feed the DM with.
  @return VSCP_ERROR_SUCCESS on success, or error code.
*/

int
vscp_frmw2_callback_feed_app(void* const puserdata, vscpEvent* pev)
{
  return VSCP_ERROR_SUCCESS;
}

/*!
  @brief Get DM matrix info

  The output message data structure should be filled with
  the following data by this routine.
      byte 0 - Number of DM rows. 0 if none.
      byte 1 - offset in register space.
      byte 2 - start page MSB
      byte 3 - start page LSB
      byte 4 - End page MSB
      byte 5 - End page LSB
      byte 6 - Level II size of DM row (Just for Level II nodes).

  @param pdata Pointer to user data (typical points to context)
  @param pDM Pointer to DM info structure.
  @return VSCP_ERROR_SUCCESS on success, or error code.
 */

int
vscp_frmw2_callback_send_dm_info(void* const puserdata, char* pDM)
{
  return VSCP_ERROR_SUCCESS;
}

/*!
  @brief Get embedded MDF info

  If available this routine sends an embedded MDF file
  in several events. See specification CLASS1.PROTOCOL
  Type=35/36

  @param pdata Pointer to user data (typical points to context)
  @return VSCP_ERROR_SUCCESS on success, or error code.
 */
int
vscp_frmw2_callback_send_embedded_mdf(void* const puserdata)
{
  btest* pbtest = (btest*)QApplication::instance();
  return pbtest->sendEmbeddedMDF();
}

/*!
  @brief Go boot loader mode

  This routine force the system into boot loader mode according
  to the selected protocol.

  @param pdata Pointer to user data (typical points to context)
  @param[out] palgorithm Pointer to bootloader algorithm to use.
  @return VSCP_ERROR_SUCCESS on success, or error code.
 */

int
vscp_frmw2_callback_enter_bootloader(void* const puserdata, uint8_t* palgorithm)
{
  // TODO
  return VSCP_ERROR_SUCCESS;
}

/*!
  @brief Restore defaults

  If 0x55/0xaa is written to register location
  162 within one second defaults should be loaded
  by the device.

  @param pdata Pointer to user data (typical points to context)
  @return VSCP_ERROR_SUCCESS on success, or error code.
 */

int
vscp_frmw2_callback_restore_defaults(void* const puserdata)
{
  return VSCP_ERROR_SUCCESS;
}



/*!
  @brief Handle high end server response

  @param pdata Pointer to user data (typical points to context)
  @return VSCP_ERROR_SUCCESS on success, or error code.
*/

int
vscp_frmw2_callback_high_end_server_response(void* const puserdata)
{
  return VSCP_ERROR_SUCCESS;
}

/*!
  Notification receivced that standard register has been changed.
*/
int
vscp_frmw2_callback_stdreg_change(void* const puserdata, uint32_t stdreg)
{
  btest* pbtest = (btest*)QApplication::instance();
  return pbtest->standardRegHasChanged(stdreg);
}

/*!
  Feed the watchdog on systems that need to do
  that during lengthy operations.
*/
void
vscp_frmw2_callback_feed_watchdog(void* const puserdata)
{
  // We do nothing
}

/*!
  Do cold reset
*/
void
vscp_frmw2_callback_reset(void* const puserdata)
{
  // TODO
}
