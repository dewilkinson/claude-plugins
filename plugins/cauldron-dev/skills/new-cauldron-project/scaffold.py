#!/usr/bin/env python3
"""
Scaffold an empty AMD Cauldron (DirectX 12) application. See SKILL.md next to this file.

    python scaffold.py HelloCauldron --dir C:\\github\\HelloCauldron

Produces a git repository laid out like SplatLab / Surfels: root CMakeLists.txt, CMakeSettings.json,
libs/cauldron (submodule), optional libs/cauldron-prebuilt, src/DX12/<app>, and generates the Visual
Studio solution into build/. Every file under templates/ is copied with {{PLACEHOLDER}} substitution;
"__PROJECT__" in a template file name becomes the project name and "_gitignore" becomes ".gitignore".
"""
import argparse
import datetime
import os
import re
import shutil
import subprocess
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
TEMPLATES = HERE / "templates"
DEFAULT_CAULDRON_URL = "https://github.com/dewilkinson/Cauldron.git"
# Known local Cauldron checkouts, tried in order for --cauldron-from auto (cwd's repo is tried first).
KNOWN_CAULDRON_CHECKOUTS = [
    Path(r"C:\github\splatlab\libs\cauldron"),
    Path(r"C:\github\surfels\libs\cauldron"),
]
PREBUILT_LIBS = ["Cauldron_Common", "Cauldron_DX12", "ImGUI"]
FILE_RENAMES = {"_gitignore": ".gitignore"}


def run(cmd, cwd=None, check=True, capture=False):
    print("  $ " + " ".join(str(c) for c in cmd), flush=True)
    r = subprocess.run([str(c) for c in cmd], cwd=cwd, text=True, capture_output=capture)
    if check and r.returncode != 0:
        if capture:
            sys.stdout.write(r.stdout or "")
            sys.stderr.write(r.stderr or "")
        sys.exit(f"error: command failed with exit code {r.returncode}: {' '.join(map(str, cmd))}")
    return r


def vswhere(*props):
    exe = Path(os.environ.get("ProgramFiles(x86)", r"C:\Program Files (x86)")) / "Microsoft Visual Studio" / "Installer" / "vswhere.exe"
    if not exe.exists():
        return None
    r = subprocess.run([str(exe), "-latest", "-products", "*", "-requires", "Microsoft.Component.MSBuild", "-property", *props],
                       capture_output=True, text=True)
    out = r.stdout.strip()
    return out or None


def find_cmake():
    p = shutil.which("cmake")
    if p:
        return p
    inst = vswhere("installationPath")
    if inst:
        c = Path(inst) / "Common7" / "IDE" / "CommonExtensions" / "Microsoft" / "CMake" / "CMake" / "bin" / "cmake.exe"
        if c.exists():
            return str(c)
    return None


def detect_generator():
    ver = vswhere("installationVersion")
    major = int(ver.split(".")[0]) if ver else 0
    return {18: "Visual Studio 18 2026", 17: "Visual Studio 17 2022", 16: "Visual Studio 16 2019"}.get(major, "Visual Studio 17 2022")


def git_repo_root(path):
    r = subprocess.run(["git", "-C", str(path), "rev-parse", "--show-toplevel"], capture_output=True, text=True)
    return Path(r.stdout.strip()) if r.returncode == 0 and r.stdout.strip() else None


def resolve_cauldron_from(value):
    """A path to a Cauldron checkout (has common.cmake), or None for a network submodule add."""
    if value is None or value.lower() == "none":
        return None
    candidates = []
    if value.lower() == "auto":
        root = git_repo_root(Path.cwd())
        if root:
            candidates.append(root / "libs" / "cauldron")
        candidates += KNOWN_CAULDRON_CHECKOUTS
    else:
        candidates.append(Path(value))
    for c in candidates:
        if (c / "common.cmake").exists() and (c / "src" / "DX12" / "CMakeLists.txt").exists():
            return c.resolve()
    if value.lower() != "auto":
        sys.exit(f"error: {value} is not a Cauldron checkout (no common.cmake / src/DX12/CMakeLists.txt)")
    return None


def render_templates(dest, subs):
    written = []
    for src in sorted(TEMPLATES.rglob("*")):
        if src.is_dir():
            continue
        rel = src.relative_to(TEMPLATES)
        parts = [FILE_RENAMES.get(p, p).replace("__PROJECT__", subs["{{PROJECT}}"]) for p in rel.parts]
        out = dest.joinpath(*parts)
        out.parent.mkdir(parents=True, exist_ok=True)
        text = src.read_text(encoding="utf-8")
        for k, v in subs.items():
            text = text.replace(k, v)
        leftover = re.findall(r"\{\{[A-Z_]+\}\}", text)
        if leftover:
            sys.exit(f"error: unreplaced placeholder(s) {sorted(set(leftover))} in template {rel}")
        out.write_text(text, encoding="utf-8", newline="\n")
        written.append(out.relative_to(dest))
    return written


def add_cauldron(dest, source, url, ref):
    lib = dest / "libs" / "cauldron"
    lib.parent.mkdir(parents=True, exist_ok=True)
    if source:
        print(f"Cloning Cauldron locally from {source}")
        run(["git", "clone", "--quiet", "--local", "--no-hardlinks", source, lib])
        head = run(["git", "-C", source, "rev-parse", "HEAD"], capture=True).stdout.strip()
        run(["git", "-C", lib, "checkout", "--quiet", "--detach", head])
        run(["git", "-C", lib, "remote", "set-url", "origin", url])
        # Registers the existing clone as the submodule ("Adding existing repo at 'libs/cauldron' to the index").
        run(["git", "submodule", "add", url, "libs/cauldron"], cwd=dest)
    else:
        print(f"Adding Cauldron submodule from {url} (network)")
        run(["git", "submodule", "add", url, "libs/cauldron"], cwd=dest)
        if ref:
            run(["git", "-C", lib, "checkout", "--quiet", "--detach", ref])
    return run(["git", "-C", lib, "rev-parse", "--short", "HEAD"], capture=True).stdout.strip()


def add_prebuilt(dest, prebuilt_from, cauldron_source):
    src = Path(prebuilt_from) if prebuilt_from else (cauldron_source.parent / "cauldron-prebuilt" if cauldron_source else None)
    if src is None:
        return None
    libdir = src / "lib"
    release = libdir / "Release" / "Cauldron_DX12.lib"
    if not release.exists():
        print(f"No prebuilt Cauldron libraries at {libdir}; the project will build Cauldron from source.")
        return None
    if release.stat().st_size < 10_000:
        print(f"{release} looks like an unpulled Git LFS pointer; skipping prebuilt libraries (run `git lfs pull` there first).")
        return None
    out = dest / "libs" / "cauldron-prebuilt" / "lib"
    copied = 0
    for cfg, suffix in (("Debug", "d"), ("Release", "")):
        for name in PREBUILT_LIBS:
            f = libdir / cfg / f"{name}{suffix}.lib"
            if f.exists():
                (out / cfg).mkdir(parents=True, exist_ok=True)
                shutil.copy2(f, out / cfg / f.name)
                copied += 1
    print(f"Copied {copied} prebuilt Cauldron libraries from {libdir}")
    lfs = subprocess.run(["git", "lfs", "version"], capture_output=True, text=True).returncode == 0
    if lfs:
        run(["git", "lfs", "install", "--local"], cwd=dest, capture=True)
        (dest / ".gitattributes").write_text("*.lib filter=lfs diff=lfs merge=lfs -text\n", encoding="utf-8", newline="\n")
        print("Tracking *.lib through Git LFS (.gitattributes written).")
    else:
        print("Git LFS not installed: the prebuilt .lib files will be committed as plain binaries (about 100 MB).")
    return out


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("name", help="Project name; a C++ identifier (executable, <Name>App and <Name>Renderer classes)")
    ap.add_argument("--dir", help="Directory to create (default: ./<name>)")
    ap.add_argument("--title", help="Window title (default: the project name)")
    ap.add_argument("--cauldron-from", default="auto", metavar="PATH|auto|none",
                    help="Local Cauldron checkout to clone from (default auto: this repo's libs/cauldron, then the known SplatLab/Surfels checkouts; none = network submodule add)")
    ap.add_argument("--cauldron-url", default=DEFAULT_CAULDRON_URL, help=f"Submodule URL (default {DEFAULT_CAULDRON_URL})")
    ap.add_argument("--cauldron-ref", help="Commit/tag to check out when adding the submodule over the network")
    ap.add_argument("--prebuilt-from", metavar="PATH", help="cauldron-prebuilt directory to copy (default: the one beside --cauldron-from)")
    ap.add_argument("--no-prebuilt", action="store_true", help="Do not copy prebuilt Cauldron libraries")
    ap.add_argument("--generator", help='CMake generator (default: newest installed Visual Studio, e.g. "Visual Studio 18 2026")')
    ap.add_argument("--no-configure", action="store_true", help="Skip running CMake")
    ap.add_argument("--build", choices=["Debug", "Release"], help="Build this configuration after configuring")
    ap.add_argument("--no-commit", action="store_true", help="git init and stage, but do not commit")
    ap.add_argument("--force", action="store_true", help="Allow a non-empty target directory")
    args = ap.parse_args()

    name = args.name
    if not re.fullmatch(r"[A-Za-z_][A-Za-z0-9_]*", name):
        sys.exit(f"error: project name {name!r} must be a C++ identifier (letters, digits, underscores; not starting with a digit)")
    dest = Path(args.dir).resolve() if args.dir else (Path.cwd() / name).resolve()
    if dest.exists() and any(dest.iterdir()) and not args.force:
        sys.exit(f"error: {dest} exists and is not empty (pass --force to scaffold into it anyway)")
    if not TEMPLATES.is_dir():
        sys.exit(f"error: templates directory missing: {TEMPLATES}")

    generator = args.generator or detect_generator()
    cauldron_source = resolve_cauldron_from(args.cauldron_from)
    author = subprocess.run(["git", "config", "user.name"], capture_output=True, text=True).stdout.strip() or "Author"
    subs = {
        "{{PROJECT}}": name,
        "{{WINDOW_TITLE}}": args.title or name,
        "{{GENERATOR}}": generator,
        "{{GENERATOR_SETTINGS}}": generator + " Win64",
        "{{CAULDRON_URL}}": args.cauldron_url,
        "{{YEAR}}": str(datetime.date.today().year),
        "{{AUTHOR}}": author,
    }

    print(f"Scaffolding {name} in {dest}")
    dest.mkdir(parents=True, exist_ok=True)
    run(["git", "init", "--quiet", "-b", "main"], cwd=dest)

    files = render_templates(dest, subs)
    print(f"Wrote {len(files)} files from templates")

    cauldron_commit = add_cauldron(dest, cauldron_source, args.cauldron_url, args.cauldron_ref)
    prebuilt = None if args.no_prebuilt else add_prebuilt(dest, args.prebuilt_from, cauldron_source)

    run(["git", "add", "-A"], cwd=dest)
    if not args.no_commit:
        run(["git", "commit", "--quiet", "-m", f"Scaffold empty Cauldron DX12 project {name}"], cwd=dest)

    solution = None
    if not args.no_configure:
        cmake = find_cmake()
        if not cmake:
            print("warning: cmake not found on PATH or in Visual Studio; skipping configure. Run: cmake -S . -B build -G \"%s\" -A x64" % generator)
        else:
            run([cmake, "-S", ".", "-B", "build", "-G", generator, "-A", "x64"], cwd=dest)
            solution = next(iter(sorted((dest / "build").glob(f"{name}.sln*"))), None)
            if args.build:
                run([cmake, "--build", "build", "--config", args.build], cwd=dest)

    print()
    print("=" * 72)
    print(f"Project:      {dest}")
    print(f"Executable:   bin/{name}.exe (Debug: bin/{name}d.exe) once built")
    print(f"Cauldron:     libs/cauldron @ {cauldron_commit} ({'local clone of ' + str(cauldron_source) if cauldron_source else args.cauldron_url})")
    print(f"Prebuilt:     {'libs/cauldron-prebuilt/lib (CAULDRON_USE_PREBUILT=ON)' if prebuilt else 'none; Cauldron builds from source'}")
    print(f"Generator:    {generator}")
    print(f"Solution:     {solution.relative_to(dest) if solution else 'not generated'}")
    print("Next:")
    print(f"  cd {dest}")
    if solution:
        print(f"  cmake --build build --config Debug      (or open {solution.name} in Visual Studio and press F5)")
    else:
        print(f"  cmake -S . -B build -G \"{generator}\" -A x64")
    print("=" * 72)


if __name__ == "__main__":
    main()
