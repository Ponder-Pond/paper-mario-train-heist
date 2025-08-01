#!/usr/bin/env python3

from functools import lru_cache
import os
import shutil
from typing import List, Dict, Set, Union
from pathlib import Path
import subprocess
import sys
import ninja_syntax
from glob import glob
import json

# Configuration:
VERSIONS = ["us"]
DO_SHA1_CHECK = False

# Paths:
ROOT = Path(__file__).parent.parent.parent
if ROOT.is_absolute():
    ROOT = ROOT.relative_to(Path.cwd())

BUILD_TOOLS = Path("tools/build")
CRC_TOOL = f"{BUILD_TOOLS}/rom/n64crc"

PIGMENT64 = "pigment64"
CRUNCH64 = "crunch64"

RUST_TOOLS = [
    (PIGMENT64, "pigment64", "0.4.2"),
    (CRUNCH64, "crunch64-cli", "0.3.1"),
]


def exec_shell(command: List[str]) -> str:
    ret = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    return ret.stdout


def write_ninja_rules(
    ninja: ninja_syntax.Writer,
    cpp: str,
    extra_cppflags: str,
    extra_cflags: str,
    use_ccache: bool,
    shift: bool,
    debug: bool,
):
    # platform-specific

    ccache = ""

    if use_ccache:
        ccache = "ccache "
        try:
            subprocess.call(["ccache"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        except FileNotFoundError:
            ccache = ""

    cross = "mips-linux-gnu-"
    cc_modern = f"{cross}gcc"
    cxx_modern = f"{cross}g++"

    BFDNAME = "elf32-tradbigmips"

    CPPFLAGS_COMMON = (
        "-Iver/$version/include -Iver/$version/build/include -Iinclude -Isrc -Iassets/$version -D_FINALROM "
        "-DVERSION=$version -DF3DEX_GBI_2 -D_MIPS_SZLONG=32"
    )

    CPPFLAGS = CPPFLAGS_COMMON

    cflags_modern = f"-c -G0 -O2 -g1 -gdwarf -gz -gas-loc-support -ffast-math -fno-unsafe-math-optimizations -fdiagnostics-color=always -funsigned-char -mgp32 -mfp32 -mabi=32 -mfix4300 -march=vr4300 -mno-gpopt -mno-abicalls -fno-pic -fno-exceptions -fno-stack-protector -fno-toplevel-reorder -fno-zero-initialized-in-bss -Wno-builtin-declaration-mismatch {extra_cflags}"

    ninja.variable("python", sys.executable)

    ld_args = f"-T ver/$version/build/undefined_syms.txt -T ver/$version/undefined_syms_auto.txt -T ver/$version/undefined_funcs_auto.txt -Map $mapfile --no-check-sections --whole-archive -T $in -o $out"
    ld = f"{cross}ld" if not "PAPERMARIO_LD" in os.environ else os.environ["PAPERMARIO_LD"]

    ninja.rule(
        "ld",
        description="link($version) $out",
        command=f"{ld} {ld_args}",
    )

    ninja.rule(
        "shape_ld",
        description="link($version) shape $out",
        command=f"{ld} -T src/map_shape.ld $in -o $out",
    )

    ninja.rule(
        "shape_objcopy",
        description="objcopy($version) shape $out",
        command=f"{cross}objcopy $in $out -O binary",
    )

    Z64_DEBUG = ""
    if debug:
        Z64_DEBUG = " -gS -R .data -R .note -R .eh_frame -R .gnu.attributes -R .comment -R .options"
    ninja.rule(
        "z64",
        description="rom $out",
        command=f"{cross}objcopy $in $out -O binary{Z64_DEBUG} && python3 {BUILD_TOOLS}/append_symbol_table.py $out && {BUILD_TOOLS}/rom/n64crc $out",
        pool="console",
    )

    ninja.rule(
        "z64_ique",
        description="rom $out",
        command=f"{cross}objcopy $in $out -O binary{Z64_DEBUG}",
    )

    ninja.rule(
        "sha1sum",
        description="check $in",
        command="sha1sum -c $in && touch $out" if DO_SHA1_CHECK else "touch $out",
    )

    ninja.rule("cpp", description="cpp $in", command=f"{cpp} $in {extra_cppflags} -P -o $out")

    ninja.rule(
        "cc_modern",
        description="CC $in",
        command=f"{ccache}{cc_modern} {cflags_modern} $cflags {CPPFLAGS} {extra_cppflags} $cppflags -D_LANGUAGE_C -Werror=implicit -Werror=old-style-declaration -Werror=missing-parameter-type -Wno-error=int-conversion -Wno-error=incompatible-pointer-types -MD -MF $out.d $in -o $out",
        depfile="$out.d",
        deps="gcc",
    )

    ninja.rule(
        "cxx_modern",
        description="CXX $in",
        command=f"{ccache}{cxx_modern} {cflags_modern} $cflags {CPPFLAGS} {extra_cppflags} $cppflags -std=c++20 -D_LANGUAGE_C_PLUS_PLUS -MD -MF $out.d $in -o $out",
        depfile="$out.d",
        deps="gcc",
    )

    ninja.rule(
        "dead_cc_fix",
        description="dead_cc_fix $in",
        command=f"{cross}objcopy --redefine-sym sqrtf=dead_sqrtf $in $out",
    )

    ninja.rule(
        "bin",
        description="bin $in",
        command=f"{cross}objcopy -I binary -O {BFDNAME} --set-section-alignment .data=8 $in $out",
    )

    ninja.rule(
        "cp",
        description="cp $in $out",
        command=f"cp $in $out",
    )

    ninja.rule(
        "as",
        description="as $in",
        command=f"{cpp} {CPPFLAGS} {extra_cppflags} $cppflags $in -o  - | {cross}as -EB -march=vr4300 -mtune=vr4300 -Iinclude -o $out",
    )

    ninja.rule(
        "img",
        description="img($img_type) $in",
        command=f"$python {BUILD_TOOLS}/img/build.py $img_type $in $out $img_flags",
    )

    ninja.rule(
        "pigment",
        description="img($img_type) $in",
        command=f"{PIGMENT64} to-bin $img_flags -f $img_type -o $out $in",
    )

    ninja.rule(
        "img_header",
        description="img_header $in",
        command=f'$python {BUILD_TOOLS}/img/header.py $in $out "$c_name"',
    )

    ninja.rule(
        "yay0",
        description="yay0 $in",
        command=f"crunch64 compress yay0 $in $out",
    )

    ninja.rule(
        "npc_sprite",
        description="sprite $sprite_name",
        command=f"$python {BUILD_TOOLS}/sprite/npc_sprite.py $out $sprite_name $asset_stack",
    )

    ninja.rule(
        "sprites",
        description="sprites $out $header_out",
        command=f"$python {BUILD_TOOLS}/sprite/sprites.py $out $header_out $build_dir $asset_stack",
    )

    ninja.rule(
        "sprite_header",
        description="sprite_header $sprite_name",
        command=f"$python {BUILD_TOOLS}/sprite/header.py $out $sprite_name $sprite_id $asset_stack",
    )

    ninja.rule(
        "msg",
        description="msg $in",
        command=f"$python {BUILD_TOOLS}/msg/parse_compile.py $version $in $out",
    )

    ninja.rule(
        "icons",
        command=f"$python {BUILD_TOOLS}/icons.py $out $header_path $asset_stack",
    )

    ninja.rule(
        "move_data",
        command=f"$python {BUILD_TOOLS}/move_data.py $out $in",
    )

    ninja.rule(
        "item_data",
        command=f"$python {BUILD_TOOLS}/item_data.py $out $in $asset_stack",
    )

    ninja.rule(
        "actor_types",
        command=f"$python {BUILD_TOOLS}/actor_types.py $out $in",
    )

    ninja.rule(
        "world_map",
        command=f"$python {BUILD_TOOLS}/world_map.py $in $out",
    )

    ninja.rule(
        "recipes",
        command=f"$python {BUILD_TOOLS}/recipes.py $in $out",
    )

    ninja.rule(
        "msg_combine",
        description="msg_combine $out",
        command=f"$python {BUILD_TOOLS}/msg/combine.py $out $in",
    )

    ninja.rule(
        "mapfs",
        description="mapfs $out",
        command=f"$python {BUILD_TOOLS}/mapfs/combine.py $version $out $in",
    )

    ninja.rule(
        "tex",
        description="tex $out",
        command=f"$python {BUILD_TOOLS}/mapfs/tex.py $out $tex_name $asset_stack",
    )

    ninja.rule(
        "pack_title_data",
        description="pack_title_data $out",
        command=f"$python {BUILD_TOOLS}/mapfs/pack_title_data.py $version $out $in",
    )

    ninja.rule("map_header", command=f"$python {BUILD_TOOLS}/mapfs/map_header.py $in $out")

    ninja.rule("charset", command=f"$python {BUILD_TOOLS}/pm_charset.py $out $in")

    ninja.rule(
        "charset_palettes",
        command=f"$python {BUILD_TOOLS}/pm_charset_palettes.py $out $in",
    )

    ninja.rule(
        "sprite_shading_profiles",
        command=f"$python {BUILD_TOOLS}/sprite/sprite_shading_profiles.py $in $out $header_path",
    )

    ninja.rule("imgfx_data", command=f"$python {BUILD_TOOLS}/imgfx/imgfx_data.py $in $out")

    ninja.rule("shape", command=f"$python {BUILD_TOOLS}/mapfs/shape.py $in $out")

    ninja.rule("effect_data", command=f"$python {BUILD_TOOLS}/effects.py $in_yaml $out_dir")

    ninja.rule("pm_sbn", command=f"$python {BUILD_TOOLS}/audio/sbn.py $out $asset_stack")

    ninja.rule("flips", command=f"bash -c 'flips $baserom $in $out || true'")

    ninja.rule(
        "check_segment_sizes",
        description="check segment sizes $in",
        command=f"$python {BUILD_TOOLS}/check_segment_sizes.py $in $data > $out",
    )


def write_ninja_for_tools(ninja: ninja_syntax.Writer):
    ninja.rule(
        "cc_tool",
        description="cc_tool $in",
        command=f"cc -w $in -O3 -o $out",
    )

    ninja.build(CRC_TOOL, "cc_tool", f"{BUILD_TOOLS}/rom/n64crc.c")


def does_iconv_work() -> bool:
    # run iconv and see if it works
    stdin = "エリア ＯＭＯ２＿１".encode("utf-8")

    def run(command, stdin):
        sub = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, input=stdin, cwd=ROOT)
        return sub.stdout

    expected_stdout = run(["tools/build/iconv.py", "UTF-8", "CP932"], stdin)
    actual_stdout = run(["iconv", "--from", "UTF-8", "--to", "CP932"], stdin)
    return expected_stdout == actual_stdout


use_python_iconv = not does_iconv_work()
if use_python_iconv:
    print("warning: iconv doesn't work, using python implementation")


class Configure:
    def __init__(self, version: str):
        self.version = version
        self.version_path = ROOT / f"ver/{version}"
        self.linker_entries = None

    def split(self, assets: bool, code: bool, shift: bool, debug: bool):
        import splat.scripts.split as split

        modes = ["ld"]
        if assets:
            modes.extend(
                [
                    "bin",
                    "rodatabin",
                    "textbin",
                    "yay0",
                    "img",
                    "vtx",
                    "vtx_common",
                    "gfx",
                    "gfx_common",
                    "pm_map_data",
                    "pm_icons",
                    "pm_msg",
                    "pm_sprites",
                    "pm_charset",
                    "pm_charset_palettes",
                    "pm_effect_loads",
                    "pm_effect_shims",
                    "pm_sprite_shading_profiles",
                    "pm_imgfx_data",
                    "pm_sbn",
                ]
            )
        if code:
            modes.extend(["code", "c", "data", "rodata"])

        splat_files = [Path(self.version_path / "splat.yaml")]
        if debug:
            splat_files += [Path(self.version_path / "splat-debug.yaml")]

        if shift:
            splat_files += [Path(self.version_path / "splat-shift.yaml")]
        split.main(
            splat_files,
            modes,
            verbose=False,
        )
        self.linker_entries = split.linker_writer.entries
        self.asset_stack: List[str] = split.config["asset_stack"]

    def build_path(self) -> Path:
        return Path(f"ver/{self.version}/build")

    def undefined_syms_path(self) -> Path:
        return self.build_path() / "undefined_syms.txt"

    def elf_path(self) -> Path:
        # TODO: read basename and build_path from splat.yaml
        return Path(f"ver/{self.version}/build/papermario.elf")

    def rom_path(self) -> Path:
        return self.elf_path().with_suffix(".z64")

    def rom_ok_path(self) -> Path:
        return self.elf_path().with_suffix(".ok")

    def patch_path(self) -> Path:
        return self.elf_path().with_suffix(".bps")

    def baserom_path(self) -> Path:
        return Path(f"ver/{self.version}/baserom.z64")

    def linker_script_path(self) -> Path:
        # TODO: read from splat.yaml
        return Path(f"ver/{self.version}/papermario.ld")

    def map_path(self) -> Path:
        return self.elf_path().with_suffix(".map")

    def resolve_src_paths(self, src_paths: List[Path]) -> List[str]:
        out = []

        for path in src_paths:
            path = self.resolve_asset_path(path)

            if path is not None:
                if path.is_dir():
                    out.extend(glob(str(path) + "/**/*", recursive=True))
                else:
                    out.append(str(path))

        return out

    # Given a directory relative to assets/, return a list of all assets in the directory
    # for all layers of the asset stack
    def get_asset_list(self, asset_dir: str) -> List[str]:
        ret: Dict[Path, Path] = {}

        for stack_dir in self.asset_stack:
            path_stem = f"assets/{stack_dir}/{asset_dir}"

            for p in Path(path_stem).glob("**/*"):
                glob_part = p.relative_to(path_stem)
                if glob_part not in ret:
                    ret[glob_part] = p

        return [str(v) for v in ret.values()]

    @lru_cache(maxsize=None)
    def resolve_asset_path(self, path: Path) -> Path:
        # Remove nonsense
        path = Path(os.path.normpath(path))

        parts = list(path.parts)

        if parts[0] != "assets":
            return path

        for asset_dir in self.asset_stack:
            parts[1] = asset_dir
            new_path = Path("/".join(parts))
            if new_path.exists():
                return new_path

        return path

    def write_ninja(
        self,
        ninja: ninja_syntax.Writer,
        skip_outputs: Set[str],
        non_matching: bool,
        c_maps: bool = False,
    ):
        assert self.linker_entries is not None

        built_objects = set()
        generated_code = []
        inc_img_bins = []
        precompiled_header_path = self.build_path() / "include" / "precompiled.h.gch"

        def build(
            object_paths: Union[Path, List[Path]],
            src_paths: List[Path],
            task: str,
            variables: Dict[str, str] = {},
            implicit_outputs: List[str] = [],
            asset_deps: List[str] = [],
        ):
            if not isinstance(object_paths, list):
                object_paths = [object_paths]

            object_strs = [str(obj) for obj in object_paths]
            needs_build = False

            for object_path in object_paths:
                if object_path.suffixes[-1] == ".o":
                    built_objects.add(str(object_path))
                elif object_path.suffix.endswith(".h") or object_path.suffix.endswith(".c"):
                    generated_code.append(str(object_path))
                elif object_path.name.endswith(".png.bin") or object_path.name.endswith(".pal.bin"):
                    inc_img_bins.append(str(object_path))

                # don't rebuild objects if we've already seen all of them
                if not str(object_path) in skip_outputs:
                    needs_build = True

            for i_output in implicit_outputs:
                if i_output.endswith(".h"):
                    generated_code.append(i_output)

            if needs_build:
                skip_outputs.update(object_strs)

                implicit = []
                order_only = []

                if task in ["cc", "cxx", "cc_modern", "cxx_modern"]:
                    order_only.append("generated_code_" + self.version)
                    order_only.append("inc_img_bins_" + self.version)
                    if task == "cc_modern" and object_paths[0].suffixes[-1] != ".gch":
                        implicit.append(str(precompiled_header_path))

                inputs = self.resolve_src_paths(src_paths)
                for dir in asset_deps:
                    inputs.extend(self.get_asset_list(dir))
                ninja.build(
                    outputs=object_strs,  # $out
                    rule=task,
                    inputs=inputs,  # $in
                    implicit=implicit,
                    order_only=order_only,
                    variables={"version": self.version, **variables},
                    implicit_outputs=implicit_outputs,
                )

        # Effect data includes
        effect_yaml = ROOT / "src/effects.yaml"
        effect_data_outdir = ROOT / "assets" / version / "effects"
        effect_macros_path = effect_data_outdir / "effect_macros.h"
        effect_defs_path = effect_data_outdir / "effect_defs.h"
        effect_table_path = effect_data_outdir / "effect_table.c"

        build(
            [effect_macros_path, effect_defs_path, effect_table_path],
            [effect_yaml],
            "effect_data",
            variables={
                "in_yaml": str(effect_yaml),
                "out_dir": str(effect_data_outdir),
            },
        )

        build(
            self.build_path() / "include/world_map.inc.c",
            [Path("src/world_map.xml")],
            "world_map",
        )

        if self.version == "jp":
            build(
                self.build_path() / "include/recipes.inc.c",
                [Path("src/recipes_jp.yaml")],
                "recipes",
            )
        else:
            build(
                self.build_path() / "include/recipes.inc.c",
                [Path("src/recipes.yaml")],
                "recipes",
            )

        build(
            [
                self.build_path() / "include/move_data.inc.c",
                self.build_path() / "include/move_enum.h",
            ],
            [Path("src/move_table.yaml")],
            "move_data",
        )

        build(
            [
                self.build_path() / "include/item_data.inc.c",
                self.build_path() / "include/item_enum.h",
            ],
            [Path("src/item_table.yaml")],
            "item_data",
            variables={
                "asset_stack": ",".join(self.asset_stack),
            },
        )

        if self.version == "jp":
            build(
                [
                    self.build_path() / "include/battle/actor_types.inc.c",
                    self.build_path() / "include/battle/actor_types.h",
                ],
                [
                    Path("src/battle/actors_jp.yaml"),
                ],
                "actor_types",
            )
        else:
            build(
                [
                    self.build_path() / "include/battle/actor_types.inc.c",
                    self.build_path() / "include/battle/actor_types.h",
                ],
                [
                    Path("src/battle/actors.yaml"),
                ],
                "actor_types",
            )

        build([precompiled_header_path], [Path("include/common.h")], "cc_modern")

        import splat

        # Build objects
        for entry in self.linker_entries:
            seg = entry.segment

            if seg.type == "linker" or seg.type == "linker_offset":
                continue

            assert entry.object_path is not None

            if isinstance(seg, splat.segtypes.n64.header.N64SegHeader):
                build(entry.object_path, entry.src_paths, "as")
            elif isinstance(seg, splat.segtypes.common.hasm.CommonSegHasm):
                cppflags = f"-DVERSION_{self.version.upper()}"

                if version == "ique" and seg.name.startswith("os/"):
                    cppflags += " -DBBPLAYER"

                build(entry.object_path, entry.src_paths, "as", variables={"cppflags": cppflags})
            elif isinstance(seg, splat.segtypes.common.asm.CommonSegAsm) or (
                isinstance(seg, splat.segtypes.common.data.CommonSegData)
                and not seg.type[0] == "."
                or isinstance(seg, splat.segtypes.common.textbin.CommonSegTextbin)
            ):
                build(entry.object_path, entry.src_paths, "as")
            elif seg.type in ["pm_effect_loads", "pm_effect_shims"]:
                build(entry.object_path, entry.src_paths, "as")
            elif isinstance(seg, splat.segtypes.common.c.CommonSegC) or (
                isinstance(seg, splat.segtypes.common.data.CommonSegData) and seg.type[0] == "."
            ):
                cflags = None
                if isinstance(seg.yaml, dict):
                    cflags = seg.yaml.get("cflags")
                elif len(seg.yaml) >= 4:
                    cflags = seg.yaml[3]

                cppflags = f"-DVERSION_{self.version.upper()}"

                # default cflags where not specified
                if cflags is None:
                    if "nusys" in entry.src_paths[0].parts:
                        cflags = ""
                    elif "os" in entry.src_paths[0].parts:  # libultra
                        cflags = ""
                    else:  # papermario
                        cflags = "-fforce-addr"

                # c
                task = "cc_modern"
                if entry.src_paths[0].suffixes[-1] == ".cpp":
                    task = "cxx_modern"

                if task == "cxx":
                    task = "cxx_modern"

                if entry.src_paths[0].suffixes[-1] == ".s":
                    task = "as"

                cflags = cflags.replace("gcc_modern", "").replace("gcc_272", "")

                cppflags += " -DMODERN_COMPILER"

                if version == "ique":
                    if "nusys" in entry.src_paths[0].parts:
                        pass
                    elif "os" in entry.src_paths[0].parts:
                        cppflags += " -DBBPLAYER"
                    elif entry.src_paths[0].parts[-2] == "bss":
                        cppflags += " -DBBPLAYER"

                # Effects must call via shims due to being TLB mapped
                if "effects" in entry.src_paths[0].parts:
                    cflags += " -fno-tree-loop-distribute-patterns"  # Don't call memset etc

                encoding = "CP932"  # similar to SHIFT-JIS, but includes backslash and tilde
                if version == "ique":
                    encoding = "EUC-JP"

                if use_python_iconv:
                    iconv = f"tools/build/iconv.py UTF-8 {encoding}"
                else:
                    iconv = f"iconv --from UTF-8 --to {encoding}"

                # use tools/sjis-escape.py for src/battle/area/tik2/area.c
                if version != "ique" and seg.dir.parts[-3:] == ("battle", "area", "tik2") and seg.name == "area":
                    iconv += " | tools/sjis-escape.py"

                # Dead cod
                if isinstance(seg.parent.yaml, dict) and seg.parent.yaml.get("dead_code", False):
                    obj_path = str(entry.object_path)
                    init_obj_path = Path(obj_path + ".dead")
                    build(
                        init_obj_path,
                        entry.src_paths,
                        task,
                        variables={
                            "cflags": cflags,
                            "cppflags": cppflags,
                            "iconv": iconv,
                        },
                    )
                    build(
                        entry.object_path,
                        [init_obj_path],
                        "dead_cc_fix",
                    )
                # Not dead cod
                else:
                    if non_matching or seg.get_most_parent().name not in [
                        "main",
                        "engine1",
                        "engine2",
                    ]:
                        cflags += " -fno-common"
                    build(
                        entry.object_path,
                        entry.src_paths,
                        task,
                        variables={
                            "cflags": cflags,
                            "cppflags": cppflags,
                            "iconv": iconv,
                        },
                    )

                # images embedded inside data aren't linked, but they do need to be built into .bin files
                if isinstance(seg, splat.segtypes.common.group.CommonSegGroup):
                    for seg in seg.subsegments:
                        if isinstance(seg, splat.segtypes.n64.img.N64SegImg):
                            flags = ""
                            if seg.n64img.flip_h:
                                flags += "--flip-x "
                            if seg.n64img.flip_v:
                                flags += "--flip-y "

                            src_paths = [seg.out_path().relative_to(ROOT)]
                            inc_dir = self.build_path() / "include" / seg.dir
                            bin_path = self.build_path() / seg.dir / (seg.name + ".png.bin")

                            build(
                                bin_path,
                                src_paths,
                                "pigment",
                                variables={
                                    "img_type": seg.type,
                                    "img_flags": flags,
                                },
                            )

                            assert seg.vram_start is not None, "img with vram_start unset: " + seg.name

                            c_sym = seg.create_symbol(
                                addr=seg.vram_start,
                                in_segment=True,
                                type="data",
                                define=True,
                            )
                            name = c_sym.name
                            if "namespaced" in seg.args:
                                name = f"N({name[7:]})"
                            vars = {"c_name": name}
                            build(
                                inc_dir / (seg.name + ".png.h"),
                                src_paths,
                                "img_header",
                                vars,
                            )
                        elif isinstance(seg, splat.segtypes.n64.palette.N64SegPalette):
                            src_paths = [seg.out_path().relative_to(ROOT)]
                            inc_dir = self.build_path() / "include" / seg.dir
                            bin_path = self.build_path() / seg.dir / (seg.name + ".pal.bin")

                            build(
                                bin_path,
                                src_paths,
                                "pigment",
                                variables={
                                    "img_type": seg.type,
                                    "img_flags": "",
                                },
                            )

                            assert seg.vram_start is not None
                            c_sym = seg.create_symbol(
                                addr=seg.vram_start,
                                in_segment=True,
                                type="data",
                                define=True,
                            )
                        elif seg.type == "pm_charset":
                            rasters = []
                            entry = seg.get_linker_entries()[0]

                            for src_path in entry.src_paths:
                                out_path = self.build_path() / seg.dir / seg.name / (src_path.stem + ".bin")
                                build(
                                    out_path,
                                    [src_path],
                                    "pigment",
                                    variables={
                                        "img_type": "ci4",
                                        "img_flags": "",
                                    },
                                )
                                rasters.append(out_path)

                            build(entry.object_path.with_suffix(""), rasters, "charset")
                            build(entry.object_path, [entry.object_path.with_suffix("")], "bin")
                        elif seg.type == "pm_charset_palettes":
                            palettes = []
                            entry = seg.get_linker_entries()[0]

                            for src_path in entry.src_paths:
                                out_path = self.build_path() / seg.dir / seg.name / "palette" / (src_path.stem + ".bin")
                                build(
                                    out_path,
                                    [src_path],
                                    "pigment",
                                    variables={
                                        "img_type": "palette",
                                        "img_flags": "",
                                    },
                                )
                                palettes.append(out_path)

                            build(entry.object_path.with_suffix(""), palettes, "charset_palettes")
                            build(entry.object_path, [entry.object_path.with_suffix("")], "bin")
            elif isinstance(seg, splat.segtypes.common.bin.CommonSegBin):
                build(entry.object_path, entry.src_paths, "bin")
            elif isinstance(seg, splat.segtypes.n64.yay0.N64SegYay0):
                compressed_path = entry.object_path.with_suffix("")  # remove .o
                build(compressed_path, entry.src_paths, "yay0")
                build(entry.object_path, [compressed_path], "bin")
            elif isinstance(seg, splat.segtypes.n64.img.N64SegImg):
                flags = ""
                if seg.n64img.flip_h:
                    flags += "--flip-x "
                if seg.n64img.flip_v:
                    flags += "--flip-y "

                bin_path = entry.object_path.with_suffix(".bin")
                inc_dir = self.build_path() / "include" / seg.dir

                build(
                    bin_path,
                    entry.src_paths,
                    "pigment",
                    variables={
                        "img_type": seg.type,
                        "img_flags": flags,
                    },
                )
                build(entry.object_path, [bin_path], "bin")

                # c_sym = seg.create_symbol(
                #     addr=seg.vram_start, in_segment=True, type="data", define=True
                # )
                # vars = {"c_name": c_sym.name}
                build(inc_dir / (seg.name + ".png.h"), entry.src_paths, "img_header")
            elif isinstance(seg, splat.segtypes.n64.palette.N64SegPalette):
                bin_path = entry.object_path.with_suffix(".bin")

                build(
                    bin_path,
                    entry.src_paths,
                    "pigment",
                    variables={
                        "img_type": seg.type,
                        "img_flags": "",
                    },
                )
                build(entry.object_path, [bin_path], "bin")
            elif seg.type == "a":
                build(entry.object_path, entry.src_paths, "cp")
            elif seg.type == "pm_sprites":
                assert entry.object_path is not None

                sprite_yay0s = []

                npc_obj_path = entry.object_path.parent / "npc"

                # NPC sprite headers
                for sprite_id, sprite_dir in enumerate(entry.src_paths[1:], 1):
                    sprite_name = sprite_dir.name

                    bin_path = npc_obj_path / (sprite_name + ".bin")
                    yay0_path = bin_path.with_suffix(".Yay0")
                    sprite_yay0s.append(yay0_path)

                    build(
                        bin_path,
                        [sprite_dir],
                        "npc_sprite",
                        variables={
                            "sprite_name": sprite_name,
                            "asset_stack": ",".join(self.asset_stack),
                        },
                        asset_deps=[str(sprite_dir)],
                    )
                    build(yay0_path, [bin_path], "yay0")

                    # NPC sprite header
                    build(
                        self.build_path() / "include/sprite/npc" / (sprite_name + ".h"),
                        [sprite_dir, yay0_path],
                        "sprite_header",
                        variables={
                            "sprite_name": sprite_name,
                            "sprite_id": str(sprite_id),
                            "asset_stack": ",".join(self.asset_stack),
                        },
                    )

                # Sprites .bin
                sprite_player_header_path = str(self.build_path() / "include/sprite/player.h")

                build(
                    entry.object_path.with_suffix(".bin"),
                    [entry.src_paths[0], *sprite_yay0s],
                    "sprites",
                    variables={
                        "header_out": sprite_player_header_path,
                        "build_dir": str(self.build_path() / "assets" / self.version / "sprite"),
                        "asset_stack": ",".join(self.asset_stack),
                    },
                    implicit_outputs=[sprite_player_header_path],
                    asset_deps=["sprite/player"],
                )

                # Sprites .o
                build(entry.object_path, [entry.object_path.with_suffix(".bin")], "bin")

            elif seg.type == "pm_msg":
                msg_bins = []

                for section_idx, msg_path in enumerate(entry.src_paths):
                    bin_path = entry.object_path.with_suffix("") / f"{section_idx:02X}.bin"
                    msg_bins.append(bin_path)
                    build(bin_path, [msg_path], "msg")

                build(
                    [
                        entry.object_path.with_suffix(".bin"),
                        self.build_path() / "include" / "message_ids.h",
                    ],
                    msg_bins,
                    "msg_combine",
                )
                build(entry.object_path, [entry.object_path.with_suffix(".bin")], "bin")

            elif seg.type == "pm_icons":
                # make icons.bin
                header_path = str(self.build_path() / "include" / "icon_offsets.h")
                build(
                    entry.object_path.with_suffix(""),
                    entry.src_paths,
                    "icons",
                    variables={
                        "header_path": header_path,
                        "asset_stack": ",".join(self.asset_stack),
                    },
                    implicit_outputs=[header_path],
                    asset_deps=["icon"],
                )
                # make icons.bin.o
                build(entry.object_path, [entry.object_path.with_suffix("")], "bin")

            elif seg.type == "pm_map_data":
                # flat list of (uncompressed path, compressed? path) pairs
                bin_yay0s: List[Path] = []
                src_dir = Path("assets/x") / seg.name

                for path in entry.src_paths:
                    name = path.stem
                    out_dir = entry.object_path.with_suffix("").with_suffix("")
                    bin_path = out_dir / f"{name}.bin"

                    if name.startswith("party_"):
                        compress = True
                        build(
                            bin_path,
                            [path],
                            "img",
                            variables={
                                "img_type": "party",
                                "img_flags": "",
                            },
                        )
                    elif path.suffixes[-2:] == [".raw", ".dat"]:
                        compress = False
                        bin_path = path
                    elif name == "title_data":
                        compress = True

                        logotype_path = out_dir / "title_logotype.bin"
                        copyright_path = out_dir / "title_copyright.bin"
                        copyright_pal_path = out_dir / "title_copyright.pal"  # jp only
                        press_start_path = out_dir / "title_press_start.bin"

                        build(
                            logotype_path,
                            [src_dir / "title/logotype.png"],
                            "pigment",
                            variables={
                                "img_type": "rgba32",
                                "img_flags": "",
                            },
                        )
                        build(
                            press_start_path,
                            [src_dir / "title/press_start.png"],
                            "pigment",
                            variables={
                                "img_type": "ia8",
                                "img_flags": "",
                            },
                        )

                        if self.version == "jp":
                            build(
                                copyright_path,
                                [src_dir / "title/copyright.png"],
                                "pigment",
                                variables={
                                    "img_type": "ci4",
                                    "img_flags": "",
                                },
                            )
                            build(
                                copyright_pal_path,
                                [src_dir / "title/copyright.png"],
                                "pigment",
                                variables={
                                    "img_type": "palette",
                                    "img_flags": "",
                                },
                            )
                            imgs = [
                                logotype_path,
                                copyright_path,
                                press_start_path,
                                copyright_pal_path,
                            ]
                        else:
                            build(
                                copyright_path,
                                [src_dir / "title/copyright.png"],
                                "pigment",
                                variables={
                                    "img_type": "ia8",
                                    "img_flags": "",
                                },
                            )
                            imgs = [logotype_path, copyright_path, press_start_path]

                        build(bin_path, imgs, "pack_title_data")
                    elif name.endswith("_bg"):
                        compress = True
                        build(
                            bin_path,
                            [path],
                            "img",
                            variables={
                                "img_type": "bg",
                                "img_flags": "",
                            },
                        )
                    elif name.endswith("_tex"):
                        compress = False
                        tex_dir = path.parent / name
                        build(
                            bin_path,
                            [tex_dir, path.parent / (name + ".json")],
                            "tex",
                            variables={
                                "tex_name": name,
                                "asset_stack": ",".join(self.asset_stack),
                            },
                            asset_deps=[f"mapfs/tex/{name}"],
                        )
                    elif name.endswith("_shape_built"):
                        base_name = name[:-6]
                        map_name = base_name[:-6]
                        raw_bin_path = self.resolve_asset_path(f"assets/x/mapfs/geom/{base_name}.bin")
                        bin_path = bin_path.parent / "geom" / (base_name + ".bin")

                        if c_maps:
                            # raw bin -> c -> o -> elf -> objcopy -> final bin file
                            c_file_path = (bin_path.parent / "geom" / base_name).with_suffix(".c")
                            o_path = bin_path.parent / "geom" / (base_name + ".o")
                            elf_path = bin_path.parent / "geom" / (base_name + ".elf")

                            build(c_file_path, [raw_bin_path], "shape")
                            build(
                                o_path,
                                [c_file_path],
                                "cc_modern",
                                variables={
                                    "cflags": "",
                                    "cppflags": f"-DVERSION_{self.version.upper()}",
                                    "iconv": "iconv --from UTF-8 --to CP932",  # similar to SHIFT-JIS, but includes backslash and tilde
                                },
                            )
                            build(elf_path, [o_path], "shape_ld")
                            build(bin_path, [elf_path], "shape_objcopy")
                        else:
                            build(bin_path, [raw_bin_path], "cp")

                        xml_path = self.resolve_asset_path(f"assets/x/mapfs/geom/{map_name}.xml")
                        if xml_path.exists():
                            build(self.build_path() / "include/mapfs" / (base_name + ".h"), [xml_path], "map_header")

                        compress = True
                        out_dir = out_dir / "geom"
                    elif name.endswith("_hit"):
                        base_name = name
                        map_name = base_name[:-4]
                        raw_bin_path = self.resolve_asset_path(f"assets/x/mapfs/geom/{base_name}.bin")

                        # TEMP: star rod compatiblity
                        old_raw_bin_path = self.resolve_asset_path(f"assets/x/mapfs/{base_name}.bin")
                        if old_raw_bin_path.is_file():
                            raw_bin_path = old_raw_bin_path

                        bin_path = bin_path.parent / "geom" / (base_name + ".bin")
                        build(bin_path, [raw_bin_path], "cp")

                        xml_path = self.resolve_asset_path(f"assets/x/mapfs/geom/{map_name}.xml")
                        if xml_path.exists():
                            build(self.build_path() / "include/mapfs" / (base_name + ".h"), [xml_path], "map_header")
                    else:
                        compress = True
                        bin_path = path

                    if compress:
                        yay0_path = out_dir / f"{name}.Yay0"
                        build(yay0_path, [bin_path], "yay0")
                    else:
                        yay0_path = bin_path

                    bin_yay0s.append(bin_path)
                    bin_yay0s.append(yay0_path)

                # combine
                build(entry.object_path.with_suffix(""), bin_yay0s, "mapfs")
                build(entry.object_path, [entry.object_path.with_suffix("")], "bin")
            elif seg.type == "pm_sprite_shading_profiles":
                header_path = str(self.build_path() / "include/sprite/sprite_shading_profiles.h")
                build(
                    entry.object_path.with_suffix(""),
                    entry.src_paths,
                    "sprite_shading_profiles",
                    implicit_outputs=[header_path],
                    variables={
                        "header_path": header_path,
                    },
                )
                build(entry.object_path, [entry.object_path.with_suffix("")], "bin")
            elif seg.type == "pm_sbn":
                sbn_path = entry.object_path.with_suffix("")
                build(
                    sbn_path,
                    entry.src_paths,
                    "pm_sbn",
                    variables={
                        "asset_stack": ",".join(self.asset_stack),
                    },
                    asset_deps=entry.src_paths,
                )
                build(entry.object_path, [sbn_path], "bin")
            elif seg.type == "linker" or seg.type == "linker_offset":
                pass
            elif seg.type == "pm_imgfx_data":
                c_file_path = Path(f"assets/{self.version}") / "imgfx" / (seg.name + ".c")
                build(c_file_path, entry.src_paths, "imgfx_data")

                build(
                    entry.object_path,
                    [c_file_path],
                    "cc_modern",
                    variables={
                        "cflags": "",
                        "cppflags": f"-DVERSION_{self.version.upper()}",
                        "iconv": "iconv --from UTF-8 --to CP932",  # similar to SHIFT-JIS, but includes backslash and tilde
                    },
                )
            else:
                raise Exception(f"don't know how to build {seg.__class__.__name__} '{seg.name}'")

        # Run undefined_syms through cpp
        ninja.build(
            str(self.undefined_syms_path()),
            "cpp",
            str(self.version_path / "undefined_syms.txt"),
        )

        # Build elf, z64, ok
        additional_objects = [str(self.undefined_syms_path())]

        ninja.build(
            str(self.elf_path()),
            "ld",
            str(self.linker_script_path()),
            implicit=[str(obj) for obj in built_objects] + additional_objects,
            variables={"version": self.version, "mapfile": str(self.map_path())},
        )

        if self.version == "ique":
            ninja.build(
                str(self.rom_path()),
                "z64_ique",
                str(self.elf_path()),
                variables={"version": self.version},
            )
        else:
            ninja.build(
                str(self.rom_path()),
                "z64",
                str(self.elf_path()),
                implicit=[CRC_TOOL],
                variables={"version": self.version},
            )

        if not non_matching:
            ninja.build(
                str(self.rom_ok_path()),
                "sha1sum",
                f"ver/{self.version}/checksum.sha1",
                implicit=[str(self.rom_path())],
            )
        else:
            ninja.build(
                str(self.rom_ok_path()),
                "check_segment_sizes",
                str(self.elf_path()),
                variables={"data": json.dumps(json.dumps(self.get_segment_max_sizes(), separators=(",", ":")))},
                implicit=[str(self.rom_path())],
            )

        ninja.build(
            str(self.patch_path()),
            "flips",
            str(self.rom_path()),
            variables={"baserom": str(self.baserom_path())},
        )

        ninja.build("generated_code_" + self.version, "phony", generated_code)
        ninja.build("inc_img_bins_" + self.version, "phony", inc_img_bins)

    def get_segment_max_sizes(self):
        assert self.linker_entries is not None
        segment_size_map = {}

        # depth-first search
        def visit(segment):
            if hasattr(segment, "parent") and segment.parent is not None:
                visit(segment.parent)
            if hasattr(segment, "yaml") and isinstance(segment.yaml, dict) and "max_size" in segment.yaml:
                segment_size_map[segment.name] = segment.yaml["max_size"]
        for entry in self.linker_entries:
            visit(entry.segment)

        return segment_size_map

    def make_current(self, ninja: ninja_syntax.Writer):
        current = Path("ver/current")

        try:
            current.unlink()
        except Exception:
            pass

        current.symlink_to(self.version)

        ninja.build("ver/current/build/papermario.z64", "phony", str(self.rom_path()))


if __name__ == "__main__":
    from argparse import ArgumentParser

    parser = ArgumentParser(description="Paper Mario build.ninja generator")
    parser.add_argument(
        "version",
        nargs="*",
        default=[],
        choices=[*VERSIONS, []],
        help="Version(s) to configure for. Most tools will operate on the first-provided only. Supported versions: "
        + ",".join(VERSIONS),
    )
    parser.add_argument("--cpp", help="GNU C preprocessor command")
    parser.add_argument(
        "-c",
        "--clean",
        action="store_true",
        help="Delete assets and previously-built files",
    )
    parser.add_argument("--splat", default="tools/splat", help="Path to splat tool to use")
    parser.add_argument("--split-code", action="store_true", help="Re-split code segments to asm files")
    parser.add_argument(
        "--no-split-assets",
        action="store_true",
        help="Don't split assets from the baserom(s)",
    )
    parser.add_argument("-d", "--debug", action="store_true", help="Generate debugging information")
    parser.add_argument(
        "-N",
        "--no-non-matching",
        action="store_true",
        help="Compile nonmatching code. Combine with --debug for more detailed debug info",
    )
    parser.add_argument(
        "--no-shift",
        action="store_true",
        help="Build a shiftable version of the game (non-matching)",
    )
    parser.add_argument(
        "--no-modern-gcc",
        action="store_true",
        help="Use modern GCC instead of the original compiler",
    )
    parser.add_argument("--no-ccache", action="store_true", help="Use ccache")
    parser.add_argument(
        "--c-maps",
        action="store_true",
        help="Convert map binaries to C as part of the build process",
    )
    args = parser.parse_args()
    args.shift = not args.no_shift
    args.non_matching = not args.no_non_matching
    args.ccache = not args.no_ccache

    version_err_msg = ""
    missing_tools = []
    version_old_tools = []
    for tool, crate_name, req_version in RUST_TOOLS:
        try:
            version = exec_shell([tool, "--version"]).split(" ")[1].strip()

            if version < req_version:
                version_err_msg += (
                    f"error: {tool} version {req_version} or newer is required, system version is {version}"
                )
                version_old_tools.append(crate_name)
        except (FileNotFoundError, PermissionError):
            missing_tools.append(crate_name)

    if version_old_tools or missing_tools:
        if version_err_msg:
            print(version_err_msg)
        if missing_tools:
            print(f"error: cannot find required Rust tool(s): {', '.join(missing_tools)}")
        print()
        print("To install/update dependencies, obtain cargo:\n\tcurl https://sh.rustup.rs -sSf | sh")
        print(f"and then run:")
        for tool in missing_tools:
            print(f"\tcargo install {tool}")
        for tool in version_old_tools:
            print(f"\tcargo install {tool}")
        exit(1)

    # default version behaviour is to only do those that exist
    if len(args.version) > 0:
        versions = args.version
    else:
        versions = []

        for version in VERSIONS:
            rom = ROOT / f"ver/{version}/baserom.z64"

            print(f"configure: looking for baserom {rom.relative_to(ROOT)}", end="")

            if rom.exists():
                print("...found")
                versions.append(version)
            else:
                print("...missing")

        if len(versions) == 0:
            print("error: no baseroms found")
            exit(1)

    if args.clean:
        print("configure: cleaning...")

        exec_shell(["ninja", "-t", "clean"])

        for version in versions:
            shutil.rmtree(ROOT / f"assets/{version}", ignore_errors=True)
            shutil.rmtree(ROOT / f"ver/{version}/assets", ignore_errors=True)
            shutil.rmtree(ROOT / f"ver/{version}/build", ignore_errors=True)
            try:
                os.remove(ROOT / f"ver/{version}/.splat_cache")
            except OSError:
                pass

    args.debug = True

    extra_cflags = ""
    extra_cppflags = ""
    if args.non_matching:
        extra_cppflags += " -DNON_MATCHING"

        if args.debug:
            # extra_cflags += " -ggdb3"
            extra_cppflags += " -DDEBUG"  # e.g. affects ASSERT macro

    if args.shift:
        extra_cppflags += " -DSHIFT"

    extra_cflags += " -Wall -Wno-narrowing -Winline"

    # Warnings made into errors by default in GCC 14
    # https://gcc.gnu.org/gcc-14/porting_to.html#warnings-as-errors
    extra_cflags += " --warn-missing-parameter-type -Wincompatible-pointer-types -Wint-conversion  -Wreturn-type"

    # add splat to python import path
    sys.path.insert(0, str((ROOT / args.splat / "src").resolve()))

    ninja = ninja_syntax.Writer(open(str(ROOT / "build.ninja"), "w"), width=9999)

    non_matching = args.non_matching or True or args.shift

    write_ninja_rules(
        ninja,
        args.cpp or "mips-linux-gnu-cpp",
        extra_cppflags,
        extra_cflags,
        args.ccache,
        args.shift,
        args.debug,
    )
    write_ninja_for_tools(ninja)

    skip_files: Set[str] = set()
    all_rom_oks: List[str] = []
    first_configure = None

    for version in versions:
        print(f"configure: configuring version {version}")

        if version == "ique" and not args.non_matching and sys.platform == "darwin":
            print(
                "configure: refusing to build iQue Player version on macOS because EGCS compiler is not available (use --non-matching to use default compiler)"
            )
            continue

        configure = Configure(version)

        if not first_configure:
            first_configure = configure

        # include tools/splat_ext in the python path
        sys.path.append(str((ROOT / "tools/splat_ext").resolve()))

        configure.split(not args.no_split_assets, args.split_code, args.shift, args.debug)
        configure.write_ninja(ninja, skip_files, non_matching, args.c_maps)

        all_rom_oks.append(str(configure.rom_ok_path()))

    assert first_configure, "no versions configured"
    first_configure.make_current(ninja)

    ninja.build("all", "phony", all_rom_oks)
    ninja.default("all")
