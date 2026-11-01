import subprocess
import tempfile
import unittest
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]


class HeapAllocatorTests(unittest.TestCase):
    def test_alignment_invalid_free_and_coalescing(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            test_binary = Path(temp_dir) / "heap-allocator-tests"
            compile_result = subprocess.run(
                [
                    "gcc",
                    "-m32",
                    "-std=c23",
                    "-Wall",
                    "-Wextra",
                    "-I",
                    str(REPO_ROOT / "include"),
                    str(REPO_ROOT / "tests/heap_allocator_harness.c"),
                    str(REPO_ROOT / "src/arch/x86/mem/heap.c"),
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
                [str(test_binary)],
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
