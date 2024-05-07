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
    VSCP bootloader code and for the standard VSCP firmware code.
   ----------------------------------------------------------------------------

  We can be in two conditions in any moment.
    * Bootloader mode. Firmware can be loaded. abd we can ente this mode
                        by issue VSCP events.
    * Firmware mode. A simulated hardware running as an app.
*/

#include <crc.h>
#include <guid.h>
#include <vscp-bootloader.h>
#include <vscp.h>

#include "btest.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//
//                            Firmware callbacks
//
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

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
vscpboot_sendEvent(vscpEventEx* pex)
{
  btest* pbtest = (btest*)QApplication::instance();
  return pbtest->vscpboot_sendEvent(pex);
}

/*!
  Get VSCP event
  -----------------------------------------------------------
  IMPORTANT!
  This routine should translate all VSCP_CLASS2_LEVEL1_PROTOCOL
  events to VSCP_CLASS1_PROTOCOL events.
  -----------------------------------------------------------
  @param Pointer to VSCP event structure.
   @return VSCP_ERROR_SUCCESS on success
*/
int
vscpboot_getEvent(vscpEventEx* pex)
{
  btest* pbtest = (btest*)QApplication::instance();
  return pbtest->vscpboot_getEvent(pex);
}
