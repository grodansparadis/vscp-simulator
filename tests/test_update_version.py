#!/usr/bin/env python3
"""Tests for tools/update_version.py (run: python3 -B tests/test_update_version.py)."""

import pathlib
import re
import shutil
import sys
import tempfile
import unittest
import xml.etree.ElementTree as ET

ROOT = pathlib.Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "tools"))

import update_version  # noqa: E402

HEADER = ROOT / "src" / "vscpsim-version.h"
CMAKELISTS = ROOT / "CMakeLists.txt"


class UpdateVersionTest(unittest.TestCase):
    def setUp(self):
        self.dir = tempfile.mkdtemp()
        self.header = pathlib.Path(self.dir) / "vscpsim-version.h"
        self.variables = pathlib.Path(self.dir) / "variables.xml"
        shutil.copy(HEADER, self.header)
        shutil.copy(ROOT / "docs" / "variables.xml", self.variables)

    def tearDown(self):
        shutil.rmtree(self.dir)

    def set_version(self, version):
        self.header.write_text(update_version.write_version(self.header.read_text(), version))

    def run_update(self, date):
        update_version.main(["--header", str(self.header), "--variables", str(self.variables), "--date", date])
        version = update_version.read_version(self.header.read_text())
        self.assertEqual("%d.%d.%d" % version, ET.fromstring(self.variables.read_text()).findtext("document-version"))
        return version

    def test_repository_header_is_valid(self):
        update_version.read_version(HEADER.read_text())

    def test_default_header_is_valid(self):
        self.assertEqual(0, update_version.main(["--check"]))

    def test_same_month_increments_patch(self):
        self.set_version((26, 11, 4))
        self.assertEqual((26, 11, 5), self.run_update("2026-11"))
        self.assertEqual((26, 11, 6), self.run_update("2026-11"))

    def test_new_month_resets_patch(self):
        self.set_version((26, 11, 17))
        self.assertEqual((26, 12, 0), self.run_update("2026-12"))

    def test_new_year_resets_patch(self):
        self.set_version((26, 12, 9))
        self.assertEqual((27, 1, 0), self.run_update("2027-01"))

    def test_string_and_number_follow(self):
        self.set_version((26, 11, 2))
        self.run_update("2026-11")
        text = self.header.read_text()
        self.assertIn('VSCPSIM_VERSION_STR   "26.11.3"', text)
        self.assertIn("VSCPSIM_VERSION       0x1A0B0003UL", text)

    def test_date_before_version_is_refused(self):
        self.set_version((26, 11, 0))
        with self.assertRaises(ValueError):
            self.run_update("2026-10")

    def test_check_does_not_change(self):
        before = self.header.read_text()
        variables_before = self.variables.read_text()
        update_version.main(["--header", str(self.header), "--variables", str(self.variables), "--check"])
        self.assertEqual(before, self.header.read_text())
        self.assertEqual(variables_before, self.variables.read_text())

    def test_document_update_preserves_other_metadata(self):
        text = '<var>\n    <creation-time>2026-11-16 13:55</creation-time>\n    <document-version>0.0.2</document-version>\n</var>'
        self.variables.write_text(text)
        self.set_version((26, 11, 4))
        self.run_update("2026-11")
        self.assertEqual(text.replace("0.0.2", "26.11.5"), self.variables.read_text())

    def test_invalid_document_metadata_leaves_header_unchanged(self):
        for text in (
            "<var>",
            "<var></var>",
            "<var><document-version>1</document-version><document-version>2</document-version></var>",
        ):
            with self.subTest(text=text):
                self.variables.write_text(text)
                before = self.header.read_text()
                with self.assertRaises((ValueError, ET.ParseError)):
                    self.run_update("2026-11")
                self.assertEqual(before, self.header.read_text())
                self.assertEqual(text, self.variables.read_text())

    def test_cmake_reads_the_same_fields(self):
        # CMakeLists.txt takes the project version from the header
        cmake = CMAKELISTS.read_text()
        self.assertIn("src/vscpsim-version.h", cmake)
        self.assertTrue(re.search(r"VSCPSIM_VERSION_\(YEAR\|MONTH\|PATCH\)", cmake))


if __name__ == "__main__":
    unittest.main()
