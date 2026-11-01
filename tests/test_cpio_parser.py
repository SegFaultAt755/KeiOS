import subprocess
import tempfile
import unittest
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]


class CpioParserTests(unittest.TestCase):
    def test_parser_regressions(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            test_binary = Path(temp_dir) / "cpio-parser-tests"
            compile_result = subprocess.run(
                [
                    "rustc",
                    "--edition=2024",
                    "--test",
                    str(REPO_ROOT / "tests/cpio_parser_harness.rs"),
                    "-o",
                    str(test_binary),
                ],
                cwd=REPO_ROOT,
                capture_output=True,
                text=True,
                check=False,
            )
            self.assertEqual(
                compile_result.returncode,
                0,
                compile_result.stdout + compile_result.stderr,
            )

            test_result = subprocess.run(
                [str(test_binary), "--quiet"],
                cwd=REPO_ROOT,
                capture_output=True,
                text=True,
                check=False,
            )
            self.assertEqual(
                test_result.returncode,
                0,
                test_result.stdout + test_result.stderr,
            )


if __name__ == "__main__":
    unittest.main()
