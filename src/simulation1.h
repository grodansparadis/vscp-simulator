/* ******************************************************************************
 * 	VSCP (Very Simple Control Protocol)
 * 	https://www.vscp.org
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2000-2024 Ake Hedman, Grodans Paradis AB
 * <info@grodansparadis.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * This file is part of VSCP - Very Simple Control Protocol
 * https://www.vscp.org
 *
 * ******************************************************************************
 */

/*!
  @file simulation1.h
  @defgroup VSCP node predefined simulation model 1

  @{
*/

#ifndef __VSCP_FIRMWARE_SIMULATION1_H
#define __VSCP_FIRMWARE_SIMULATION1_H

typedef struct simulation1 {
  uint8_t m_reg_zone;               // Zone for module
  uint8_t m_reg_subzone_S[10];      // S0-S9  Switch button
  uint8_t m_reg_subzone_C[10];      // C0-C9  Checkbox
  uint8_t m_reg_subzone_R[10];      // R0-R9  Radio button
  uint8_t m_reg_subzone_slider[10]; // slider0 - slider9

  uint8_t m_reg_value_S[10];      // S0-S9  Switch button value
  uint8_t m_reg_value_C[10];      // C0-C9  Checkbox value
  uint8_t m_reg_value_R[10];      // R0-R9  Radio button value
  uint8_t m_reg_value_slider[10]; // slider0 - slider9

  uint8_t m_background_color[3]; // RGB color for sim tab-sheet

  uint8_t m_period_measurement_event; // Period in seconds for status event
  uint8_t m_coding_measurement_event; // Coding for measuremet event

  uint8_t m_dm[10 * 9]; // Decision matrix 8-rows
} simulation1;

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif // __VSCP_FIRMWARE_SIMULATION1_H