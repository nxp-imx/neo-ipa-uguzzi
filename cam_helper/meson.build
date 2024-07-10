# SPDX-License-Identifier: CC0-1.0

nxp_ipa_cam_helper_sources = files([
    'camera_helper.cpp',
    'camera_helper_mx95mbcam.cpp',
    'md_parser_ox.cpp',
])

nxp_ipa_cam_helper_includes = [
    ipa_includes,
]

nxp_ipa_cam_helper_deps = [
    libcamera_base,
    libcamera_dep,
]

subdir('libipa')

nxp_ipa_cam_helper_lib = static_library('nxp_ipa_cam_helper', nxp_ipa_cam_helper_sources,
                                         include_directories : nxp_ipa_cam_helper_includes,
                                         dependencies : nxp_ipa_cam_helper_deps)
