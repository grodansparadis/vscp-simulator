// vscpsim-version.h
//
// Version of the VSCP simulator: year.month.patch
//
//   VSCPSIM_VERSION_YEAR   Two digit year (26 = 2026)
//   VSCPSIM_VERSION_MONTH  Month 1-12
//   VSCPSIM_VERSION_PATCH  Push number within the month, from 0
//
// The values are updated automatically on every push to master by the
// "Update version" GitHub workflow (tools/update_version.py). Do not edit
// them by hand.
//
// VSCPSIM_VERSION is one number for compile time checks:
// (year << 24) | (month << 16) | patch
//
// This file is part of the VSCP (https://www.vscp.org)
//
// Copyright (C) 2000-2026 Ake Hedman, Grodans Paradis AB
// <info@grodansparadis.com>
//
// SPDX-License-Identifier: MIT

#ifndef VSCPSIM_VERSION_H
#define VSCPSIM_VERSION_H

#define VSCPSIM_VERSION_YEAR  26
#define VSCPSIM_VERSION_MONTH 11
#define VSCPSIM_VERSION_PATCH 5

#define VSCPSIM_VERSION_STR   "26.11.5"
#define VSCPSIM_VERSION       0x1A0B0005UL

#define VSCPSIM_VERSION_CODE(year, month, patch) (((unsigned long) (year) << 24) | ((unsigned long) (month) << 16) | (patch))

#endif
