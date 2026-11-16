#!/usr/bin/env python3
"""Update the VSCP simulator version (year.month.patch).

Used by the "Update version" GitHub workflow on every push to main:

  * same year and month as the current version -> patch + 1
  * new month (or year)                          -> patch = 0

The version lives in src/vscpsim-version.h. CMake reads it from there for
the project version. docs/variables.xml receives the same document-version.

Usage:
  tools/update_version.py [--header PATH] [--variables PATH] [--date YYYY-MM] [--check]

  --date   Use this year/month instead of the current UTC date (tests).
  --check  Only validate the header and document metadata, do not change them.
"""

import argparse
import datetime
import pathlib
import re
import sys
import xml.etree.ElementTree as ET

DEFAULT_HEADER = pathlib.Path(__file__).resolve().parent.parent / "src" / "vscpsim-version.h"
DEFAULT_VARIABLES = pathlib.Path(__file__).resolve().parent.parent / "docs" / "variables.xml"

FIELDS = ("YEAR", "MONTH", "PATCH")
PATCH_MAX = 0xFFFF  # 16 bits in VSCPSIM_VERSION


def define_pattern(name):
    return re.compile(r"^(#define\s+VSCPSIM_VERSION_%s\s+)(\S+)(.*)$" % name, re.MULTILINE)


STR_PATTERN = re.compile(r'^(#define\s+VSCPSIM_VERSION_STR\s+)"[^"]*"(.*)$', re.MULTILINE)
NUM_PATTERN = re.compile(r"^(#define\s+VSCPSIM_VERSION\s+)0x[0-9A-Fa-f]+UL(.*)$", re.MULTILINE)
DOCUMENT_PATTERN = re.compile(r"(<document-version>)[^<]*(</document-version>)")


def read_version(text):
    """Return (year, month, patch) from the header text."""
    values = []
    for name in FIELDS:
        matches = define_pattern(name).findall(text)
        if len(matches) != 1:
            raise ValueError("VSCPSIM_VERSION_%s must be defined exactly once" % name)
        values.append(int(matches[0][1], 10))
    year, month, patch = values
    if not (0 <= year <= 99 and 1 <= month <= 12 and 0 <= patch <= PATCH_MAX):
        raise ValueError("Invalid version %d.%d.%d" % (year, month, patch))
    if len(STR_PATTERN.findall(text)) != 1 or len(NUM_PATTERN.findall(text)) != 1:
        raise ValueError("VSCPSIM_VERSION_STR / VSCPSIM_VERSION missing")
    return year, month, patch


def next_version(current, year, month):
    """Version for a push in year/month (two digit year)."""
    cur_year, cur_month, cur_patch = current
    if (year, month) == (cur_year, cur_month):
        if cur_patch >= PATCH_MAX:
            raise ValueError("Patch number overflow")
        return year, month, cur_patch + 1
    if (year, month) < (cur_year, cur_month):
        raise ValueError("Date %02d.%02d is before the current version %d.%d" % (year, month, cur_year, cur_month))
    return year, month, 0


def write_version(text, version):
    year, month, patch = version
    for name, value in zip(FIELDS, version):
        text = define_pattern(name).sub(lambda m, v=value: "%s%d%s" % (m.group(1), v, m.group(3)), text)
    text = STR_PATTERN.sub(lambda m: '%s"%d.%d.%d"%s' % (m.group(1), year, month, patch, m.group(2)), text)
    number = (year << 24) | (month << 16) | patch
    text = NUM_PATTERN.sub(lambda m: "%s0x%08XUL%s" % (m.group(1), number, m.group(2)), text)
    return text


def write_document_version(text, version):
    root = ET.fromstring(text)
    if root.tag != "var" or len(root.findall("document-version")) != 1 or len(DOCUMENT_PATTERN.findall(text)) != 1:
        raise ValueError("Document metadata must contain exactly one /var/document-version")
    value = "%d.%d.%d" % version
    return DOCUMENT_PATTERN.sub(lambda m: "%s%s%s" % (m.group(1), value, m.group(2)), text)


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--header", type=pathlib.Path, default=DEFAULT_HEADER)
    parser.add_argument("--variables", type=pathlib.Path, default=DEFAULT_VARIABLES)
    parser.add_argument("--date", help="YYYY-MM to use instead of the current UTC date")
    parser.add_argument("--check", action="store_true", help="only validate the header and document metadata")
    args = parser.parse_args(argv)

    text = args.header.read_text(encoding="utf-8")
    variables = args.variables.read_text(encoding="utf-8")
    current = read_version(text)
    write_document_version(variables, current)
    if args.check:
        print("%d.%d.%d" % current)
        return 0

    if args.date:
        date = datetime.datetime.strptime(args.date, "%Y-%m")
    else:
        date = datetime.datetime.now(datetime.timezone.utc)
    version = next_version(current, date.year % 100, date.month)
    updated_variables = write_document_version(variables, version)

    args.header.write_text(write_version(text, version), encoding="utf-8")
    args.variables.write_text(updated_variables, encoding="utf-8")
    print("%d.%d.%d" % version)
    return 0


if __name__ == "__main__":
    sys.exit(main())
