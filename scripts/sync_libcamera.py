#!/usr/bin/env python3
# SPDX-License-Identifier: BSD-2-Clause
# Copyright 2026 NXP
"""
Synchronize (copy) selected libcamera shared files from a libcamera source tree
to a neo-ipa-uguzzi destination tree.

Usage:
    sync_libcamera.py <libcamera_tree> <neo_ipa_uguzzi_tree>
"""

import os
import sys
import shutil


def usage():
    print(
        f"Usage: {os.path.basename(sys.argv[0])} <libcamera_tree> <neo_ipa_uguzzi_tree>"
    )


# List of files to synchronize.
# Each entry is:
#   (source_header_path_relative_to_source_tree,
#    destination_directory_relative_to_destination_tree)
FILES = [
    # Libcamera headers
    ("include/libcamera/base/file.h", "inc/libcamera/base"),
    ("include/libcamera/base/log.h", "inc/libcamera/base"),
    ("include/libcamera/base/private.h", "inc/libcamera/base"),
    ("include/libcamera/base/utils.h", "inc/libcamera/base"),

    ("include/libcamera/internal/mapped_framebuffer.h", "inc/libcamera/internal"),
    ("include/libcamera/internal/yaml_parser.h", "inc/libcamera/internal"),
    ("include/libcamera/internal/value_node.h", "inc/libcamera/internal"),

    ("src/ipa/libipa/v4l2_params.h", "inc/libipa"),

    # Linux headers
    ("include/linux/media/v4l2-isp.h", "inc/linux/media"),

    ("include/linux/nxp_neoisp.h", "inc/linux"),
    ("include/linux/ox03c10.h", "inc/linux"),

    # camhelper files
    ("src/ipa/libipa/camera_sensor_helper.cpp", "cam_helper/libipa"),
    ("src/ipa/libipa/camera_sensor_helper.h", "cam_helper/libipa"),

    ("src/ipa/nxp/cam_helper/camera_helper.cpp", "cam_helper"),
    ("src/ipa/nxp/cam_helper/camera_helper.h", "cam_helper"),

    ("src/ipa/nxp/cam_helper/camera_helper_ar0235.cpp", "cam_helper"),
    ("src/ipa/nxp/cam_helper/camera_helper_imx219.cpp", "cam_helper"),
    ("src/ipa/nxp/cam_helper/camera_helper_imx519.cpp", "cam_helper"),
    ("src/ipa/nxp/cam_helper/camera_helper_imx708.cpp", "cam_helper"),
    ("src/ipa/nxp/cam_helper/camera_helper_mx95mbcam.cpp", "cam_helper"),
    ("src/ipa/nxp/cam_helper/camera_helper_os08a20.cpp", "cam_helper"),
    ("src/ipa/nxp/cam_helper/camera_helper_ox05b1s.cpp", "cam_helper"),
    ("src/ipa/nxp/cam_helper/camera_helper_vivid.cpp", "cam_helper"),

    ("src/ipa/rpi/cam_helper/md_parser.h", "cam_helper"),
    ("src/ipa/nxp/cam_helper/md_parser_ox.h", "cam_helper"),
    ("src/ipa/nxp/cam_helper/md_parser_ox.cpp", "cam_helper"),

    # gen-version.h
    ("utils/gen-version.sh", "scripts/libcamera"),

    # Add more entries as needed
]


def error(msg):
    print(f"Error: {msg}", file=sys.stderr)


def main():
    # Handle help or missing arguments
    if "--help" in sys.argv or "-h" in sys.argv or len(sys.argv) != 3:
        usage()
        return 1

    source_tree = sys.argv[1]
    destination_tree = sys.argv[2]

    # Validate source and destination trees
    if not os.path.isdir(source_tree):
        error(f"source tree does not exist: {source_tree}")
        return 1

    if not os.path.isdir(destination_tree):
        error(f"destination tree does not exist: {destination_tree}")
        return 1

    # Validate all paths before copying anything
    for src_rel, dst_rel_dir in FILES:
        src_path = os.path.join(source_tree, src_rel)
        dst_dir = os.path.join(destination_tree, dst_rel_dir)

        if not os.path.isfile(src_path):
            error(f"source file does not exist: {src_path}")
            return 1

        if not os.path.isdir(dst_dir):
            error(f"destination directory does not exist: {dst_dir}")
            return 1

    # Perform the copy
    for src_rel, dst_rel_dir in FILES:
        src_path = os.path.join(source_tree, src_rel)
        dst_dir = os.path.join(destination_tree, dst_rel_dir)
        dst_path = os.path.join(dst_dir, os.path.basename(src_path))

        shutil.copy2(src_path, dst_path)
        print(f"Copied {src_path} -> {dst_path}")

    return 0


if __name__ == "__main__":
    sys.exit(main())

