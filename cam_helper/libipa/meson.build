# SPDX-License-Identifier: CC0-1.0

libipa_headers = files([
    'camera_sensor_helper.h',
])

libipa_sources = files([
    'camera_sensor_helper.cpp',
])

libipa_includes = [
    ipa_includes,
]

libipa = static_library('ipa', [libipa_sources, libipa_headers],
                        include_directories : ipa_includes,
                        dependencies : libcamera_base)
