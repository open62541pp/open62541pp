import platform
from argparse import ArgumentParser
from pathlib import Path
from subprocess import Popen, PIPE, TimeoutExpired
from sys import exit
from textwrap import indent
from time import sleep
from typing import List, Optional, Union

WAIT_SERVER = 1
WAIT_CLIENT = 3
SKIP_MISSING = True


class Process:
    def __init__(self, cmd: Union[str, Path], args: List[str]):
        self.proc = Popen([cmd, *args], stdout=PIPE, stderr=PIPE)
        self.stdout = ""
        self.stderr = ""
        self.terminated = False

    @property
    def returncode(self) -> int | None:
        return self.proc.returncode
    
    @property
    def failed(self) -> bool:
        return self.returncode != 0 and not self.terminated

    @property
    def stdout_nologs(self) -> str:
        lines = self.stdout.split("\n")
        return "\n".join(line for line in lines if not line.startswith("["))

    def wait(self, timeout_terminate: int):
        try:
            self.proc.wait(timeout=timeout_terminate)
        except TimeoutExpired:
            self.proc.terminate()
            self.terminated = True
            self.proc.wait()
        self.stdout = self.proc.stdout.read().decode(encoding="utf-8") if self.proc.stdout else ""
        self.stderr = self.proc.stderr.read().decode(encoding="utf-8") if self.proc.stderr else ""


def run(
    server_exe: Path,
    client_exe: Optional[Path] = None,
    *,
    server_args: Optional[List[str]] = None,
    client_args: Optional[List[str]] = None,
):
    server_name = server_exe.stem
    client_name = client_exe.stem if client_exe else "None"
    print(f"[RUN] {server_name} {server_args or []} / {client_name} {client_args or []}")

    if SKIP_MISSING:
        if not server_exe.exists():
            print(f"Skip: {server_exe} not found")
            return
        if client_exe and not client_exe.exists():
            print(f"Skip: {client_exe} not found")
            return

    server_proc = Process(server_exe, client_args or [])
    sleep(WAIT_SERVER)
    if client_exe:
        client_proc = Process(client_exe, client_args or [])
        client_proc.wait(WAIT_CLIENT)
    server_proc.wait(0)

    if server_proc.stdout_nologs:
        print(f"Captured output {server_name}:")
        print(indent(server_proc.stdout_nologs, "    "))
    if client_exe and client_proc.stdout_nologs:
        print(f"Captured output {client_name}:")
        print(indent(client_proc.stdout_nologs, "    "))

    if server_proc.failed:
        print(f"Error {server_name} (return code: {server_proc.returncode}):")
        print(indent(server_proc.stderr, "    "))
        exit(1)
    if client_exe and client_proc.failed:
        print(f"Error {client_name} (return code: {client_proc.returncode}):")
        print(indent(client_proc.stderr, "    "))
        exit(1)


def main():
    parser = ArgumentParser(description="Run all server/client examples.")
    parser.add_argument("path", help="binary path with examples", type=Path)
    args = parser.parse_args()
    path = args.path

    def exe(name: str) -> Path:
        """Add executable extension."""
        if platform.system() == "Windows":
            return name + ".exe"
        return name

    run(path / exe("typeconversion"))

    run(path / exe("server"))
    run(path / exe("server_logger"))
    run(path / exe("server_instantiation"))
    run(path / exe("server_valuecallback"))
    run(path / exe("server_datasource"))
    run(path / exe("server_events"))

    run(path / exe("server_minimal"), path / exe("client_find_servers"))
    run(path / exe("server_minimal"), path / exe("client_browse"))
    run(path / exe("server_minimal"), path / exe("client_subscription"))

    run(path / exe("server_method"), path / exe("client_method"))
    run(path / exe("server_method"), path / exe("client_method_async"))

    run(path / exe("server_custom_datatypes"), path / exe("client_custom_datatypes"))

    run(
        path / exe("server_accesscontrol"),
        path / exe("client_connect"),
        client_args=["--username", "user", "--password", "user", "opc.tcp://localhost:4840"],
    )
    run(
        path / exe("server_accesscontrol"),
        path / exe("client_connect"),
        client_args=["--username", "admin", "--password", "admin", "opc.tcp://localhost:4840"],
    )


if __name__ == "__main__":
    main()
