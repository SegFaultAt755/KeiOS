import unittest
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]


class IsrAbiTests(unittest.TestCase):
    def test_isr_handler_signature_matches_stub_abi(self):
        header = (REPO_ROOT / "include/arch/x86/isr.h").read_text()
        self.assertIn("void isr_handler(struct registers *regs);", header)

        source = (REPO_ROOT / "src/arch/x86/cpu/isr.c").read_text()
        self.assertIn("void isr_handler(struct registers *regs)", source)
        self.assertIn("switch (regs->intr_num)", source)

        stub = (REPO_ROOT / "src/arch/x86/cpu/isr_stub.asm").read_text()
        self.assertIn("ISR_ERRCODE   14", stub)
        self.assertIn("ISR_ERRCODE   17", stub)
        self.assertIn("push esp", stub)
        self.assertIn("call isr_handler", stub)
        self.assertIn("add esp, 4", stub)

    def test_error_code_exceptions_match_cpu_frame_layout(self):
        stub = (REPO_ROOT / "src/arch/x86/cpu/isr_stub.asm").read_text()
        errcode_vectors = [
            "ISR_ERRCODE   8",
            "ISR_ERRCODE   10",
            "ISR_ERRCODE   11",
            "ISR_ERRCODE   12",
            "ISR_ERRCODE   13",
            "ISR_ERRCODE   14",
            "ISR_ERRCODE   17",
        ]
        for vector in errcode_vectors:
            self.assertIn(vector, stub)

        self.assertNotIn("ISR_NOERRCODE 14", stub)
        self.assertNotIn("ISR_NOERRCODE 17", stub)

    def test_debug_log_reports_clean_interrupt_setup(self):
        debug_log = (REPO_ROOT / "debug.log").read_text()
        self.assertIn("[CPU] [OK] IDT setup", debug_log)
        self.assertIn("[CPU] [OK] IRQ setup", debug_log)
        self.assertIn("[CPU] [OK] IDT initialized", debug_log)
        self.assertNotIn("Unhandled interrupt", debug_log)


if __name__ == "__main__":
    unittest.main()
