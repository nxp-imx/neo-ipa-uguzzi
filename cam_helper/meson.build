# SPDX-License-Identifier: CC0-1.0

nxp_ipa_cam_helper_sources = files([
    'camera_helper.cpp',
    'camera_helper_ar0235.cpp',
    'camera_helper_imx219.cpp',
    'camera_helper_imx519.cpp',
    'camera_helper_imx708.cpp',
    'camera_helper_mx95mbcam.cpp',
    'camera_helper_os08a20.cpp',
    'camera_helper_ox05b1s.cpp',
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
