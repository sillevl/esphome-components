"""Run native regression tests; optionally use a Linux Docker image with g++."""
import argparse
import os
from pathlib import Path
import shutil
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
SOURCES = [
    "tests/test_kalfire.cpp",
    "components/kalfire/kalfire.cpp",
    "components/kalfire/session_duration_select.cpp",
    "components/kalfire/select/mode_select.cpp",
    "components/kalfire/number/flame_height.cpp",
]
FLAGS = ["-std=c++17", "-Wall", "-Wextra", "-Itests/stubs", "-I."]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--docker-image", help="Linux image containing g++, e.g. gcc:14")
    args = parser.parse_args()
    if args.docker_image:
        # Python invokes Docker directly, avoiding Git Bash path conversion.
        command = " ".join(["g++", *FLAGS, *SOURCES, "-o /tmp/test-kalfire && /tmp/test-kalfire"])
        subprocess.run([
            "docker", "run", "--rm", "--network", "none",
            "--mount", f"type=bind,source={ROOT},target=/work,readonly",
            "--workdir", "/work", "--entrypoint", "/bin/sh",
            args.docker_image, "-c", command,
        ], check=True)
    else:
        compiler = shutil.which("g++")
        if compiler is None:
            parser.error("g++ not found; install it or supply --docker-image gcc:14")
        with tempfile.TemporaryDirectory(prefix="kalfire-tests-") as directory:
            executable = str(Path(directory) / ("test-kalfire.exe" if os.name == "nt" else "test-kalfire"))
            subprocess.run([compiler, *FLAGS, *SOURCES, "-o", executable], cwd=ROOT, check=True)
            subprocess.run([executable], check=True)


if __name__ == "__main__":
    main()
