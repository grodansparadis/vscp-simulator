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
// Copyright:  (C) 2000-2025
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

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifdef WIN32
#include <pch.h>
#endif

#ifdef WIN32
#include <windows.h>
#include <winsock2.h>
#endif

#include <crc.h>
#include <guid.h>
#include <vscp-bootloader.h>
#include <vscp-firmware-level2.h>
#include <vscp.h>

#include "btest.h"

#include <chrono>
#include <cstring>

#include <spdlog/spdlog.h>

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
  The communication interface should be opened here and the system
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

/*!
  Get pointer to configuration
*/
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
  over all blocks programmed calculated with CRC-CCITT. For a successful
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
  int rv;
  vscpEventEx ex;
  btest* pbtest = (btest*)QApplication::instance();
  rv = pbtest->getEventEx(ex, 100);
  if (VSCP_ERROR_SUCCESS != rv) {
    return rv;
  }

  // The bootloader only understands CLASS1.PROTOCOL. Turn Level I events
  // sent over Level II (class 512-1023, interface GUID first in data) into
  // standard Level I events.
  if ((ex.vscp_class >= 512) && (ex.vscp_class < 1024) && (ex.sizeData >= 16)) {
    ex.vscp_class -= 512;
    memmove(ex.data, ex.data + 16, ex.sizeData - 16);
    ex.sizeData -= 16;
  }

  vscp_copyEventEx(pex, &ex);
  return rv;
}

/*!
  Unallocate storage for VSCP event ex
  @param pex Pointer to event ex to delete
*/
// void
// vscpboot_deleteEventEx(vscpEventEx** pex)
// {
//   btest* pbtest = (btest*)QApplication::instance();
//   pbtest->vscpboot_deleteEventEx(pex);
// }



////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//
//                            Firmware callbacks
//
// The VSCP level II firmware calls these through the operations table
// btest::s_firmwareOps. The context's puserdata points to the btest object.
//
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

namespace {

btest*
app(vscp_frmw2_firmware_context_t* pctx)
{
  if ((nullptr != pctx) && (nullptr != pctx->puserdata)) {
    return static_cast<btest*>(pctx->puserdata);
  }
  return (btest*)QApplication::instance();
}

// Time in milliseconds
uint32_t
frmw2_get_milliseconds(vscp_frmw2_firmware_context_t* /*pctx*/)
{
  return vscp_getMsTimeStamp();
}

// Timestamp in microseconds
uint64_t
frmw2_get_timestamp(vscp_frmw2_firmware_context_t* /*pctx*/)
{
  return (uint64_t)std::chrono::duration_cast<std::chrono::microseconds>(
           std::chrono::steady_clock::now().time_since_epoch())
    .count();
}

// Send an event. The firmware owns the event, we send a copy.
int
frmw2_send_event(vscp_frmw2_firmware_context_t* pctx, vscp_event_t* pev)
{
  if (nullptr == pev) {
    return VSCP_ERROR_INVALID_POINTER;
  }

  vscpEventEx ex;
  if (!vscp_convertEventToEventEx(&ex, pev)) {
    return VSCP_ERROR_PARAMETER;
  }

  return app(pctx)->sendEvent(ex);
}

// Decision matrix action. The simulation has no actions of its own.
int
frmw2_dm_action(vscp_frmw2_firmware_context_t* /*pctx*/,
                const vscp_event_t* pev,
                uint8_t action,
                const uint8_t* /*pparam*/)
{
  if (nullptr != pev) {
    spdlog::debug("DM action {0} for event {1}:{2}", action, pev->vscp_class, pev->vscp_type);
  }
  return VSCP_ERROR_SUCCESS;
}

int
frmw2_segment_ctrl_heartbeat(vscp_frmw2_firmware_context_t* pctx, uint16_t segcrc, uint32_t time)
{
  return app(pctx)->receivedSegCtrlHeartBeat(segcrc, time);
}

int
frmw2_report_events_of_interest(vscp_frmw2_firmware_context_t* pctx)
{
  return app(pctx)->reportEventsOfInterest();
}

// Time stamp events with a nanosecond Unix time (UNIX_NS frame version)
int
frmw2_set_event_time(vscp_frmw2_firmware_context_t* /*pctx*/, vscp_event_t* pev)
{
  if (nullptr == pev) {
    return VSCP_ERROR_INVALID_POINTER;
  }

  pev->head         = (pev->head & ~VSCP_HEADER16_FRAME_VERSION_MASK) | VSCP_HEADER16_FRAME_VERSION_UNIX_NS;
  pev->year         = 0xffff;
  pev->month        = 0xff;
  pev->timestamp_ns = vscp_makeTimeStampNs();
  return VSCP_ERROR_SUCCESS;
}

int
frmw2_get_ip_addr(vscp_frmw2_firmware_context_t* pctx, uint8_t* ip, uint8_t size)
{
  if ((nullptr == pctx) || (nullptr == ip)) {
    return VSCP_ERROR_INVALID_POINTER;
  }
  memcpy(ip, pctx->ipaddr, (size < sizeof(pctx->ipaddr)) ? size : sizeof(pctx->ipaddr));
  return VSCP_ERROR_SUCCESS;
}

int
frmw2_read_reg(vscp_frmw2_firmware_context_t* pctx, uint16_t page, uint32_t reg, uint8_t* pval)
{
  if (nullptr == pval) {
    return VSCP_ERROR_INVALID_POINTER;
  }
  return app(pctx)->readRegister(page, reg, pval);
}

int
frmw2_write_reg(vscp_frmw2_firmware_context_t* pctx, uint16_t page, uint32_t reg, uint8_t val)
{
  return app(pctx)->writeRegister(page, reg, val);
}

int
frmw2_stdreg_change(vscp_frmw2_firmware_context_t* pctx, uint32_t stdreg)
{
  return app(pctx)->standardRegHasChanged(stdreg);
}

int
frmw2_restore_defaults(vscp_frmw2_firmware_context_t* /*pctx*/)
{
  spdlog::info("Restore defaults requested");
  return VSCP_ERROR_SUCCESS;
}

/*
  Enter the bootloader. On real hardware this never returns. The simulator
  sets the boot flag and restarts, which starts the bootloader.
*/
void
frmw2_enter_bootloader(vscp_frmw2_firmware_context_t* /*pctx*/)
{
  spdlog::info("Entering bootloader");
  vscpboot_setBootFlag(btest::mode::BOOTLOADER);
  vscpboot_reboot();
}

// Warm reset: restart the simulated node in its current mode
void
frmw2_reset(vscp_frmw2_firmware_context_t* /*pctx*/)
{
  spdlog::info("Reset requested");
  vscpboot_reboot();
}

void
frmw2_feed_watchdog(vscp_frmw2_firmware_context_t* /*pctx*/)
{
  // No watchdog in the simulator
}

} // namespace

const vscp_frmw2_ops_t btest::s_firmwareOps = {
  frmw2_get_milliseconds,
  frmw2_get_timestamp,
  frmw2_send_event,
  frmw2_dm_action,
  frmw2_segment_ctrl_heartbeat,
  frmw2_report_events_of_interest,
  frmw2_set_event_time,
  frmw2_get_ip_addr,
  frmw2_read_reg,
  frmw2_write_reg,
  frmw2_stdreg_change,
  frmw2_restore_defaults,
  frmw2_enter_bootloader,
  frmw2_reset,
  frmw2_feed_watchdog,
};
