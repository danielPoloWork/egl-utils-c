#!/usr/bin/env python3
"""Regression test for autotune's majority logic.

autotune proposes a default change only when one value is the majority across run records.
A key overridden twice WITHIN a single record must not inflate its count past the number of
records (a false majority). This pins that, plus the genuine cross-record majority. Reuses
render.load_yaml; dependency-free.

    python .eaao-core/tools/tests/test_autotune.py
"""

import contextlib
import io
import os
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
TOOLS = os.path.dirname(HERE)
sys.path.insert(0, TOOLS)
import autotune  # noqa: E402  (the module under test)


def _write_record(directory, name, overrides):
    lines = ["overrides:"]
    for key, chosen, default in overrides:
        lines += [f"  - key: {key}", f"    chosen: {chosen}", f"    default: {default}"]
    with open(os.path.join(directory, name), "w", encoding="utf-8", newline="\n") as fh:
        fh.write("\n".join(lines) + "\n")


def _run(threshold):
    out, argv = io.StringIO(), sys.argv
    sys.argv = ["autotune.py", "--threshold", str(threshold)]
    try:
        with contextlib.redirect_stdout(out):
            autotune.main()
    finally:
        sys.argv = argv
    return out.getvalue()


def check(label, cond, failures):
    if not cond:
        failures.append(label)


def main():
    failures = []
    with tempfile.TemporaryDirectory() as tmp:
        autotune.RUNS = tmp  # redirect the run-record directory at the module global

        # One record overriding the same key twice must NOT clear a threshold of 2.
        _write_record(tmp, "r1.yaml", [("toolchain.linter", "ruff", "flake8"),
                                       ("toolchain.linter", "ruff", "flake8")])
        out = _run(2)
        check("duplicate within one record is not a false majority",
              "no default is overridden" in out, failures)

        # A second, distinct record makes it a genuine 2/2 majority -> proposal.
        _write_record(tmp, "r2.yaml", [("toolchain.linter", "ruff", "flake8")])
        out = _run(2)
        check("genuine cross-record majority is proposed",
              "toolchain.linter" in out and "ruff" in out, failures)

    if failures:
        print("test-autotune: FAIL\n")
        for f in failures:
            print(f"  {f}")
        print(f"\n{len(failures)} autotune behaviour(s) wrong.")
        return 1
    print("test-autotune: OK — duplicate overrides don't fake a majority; real majorities propose.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
