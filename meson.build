# SPDX-License-Identifier: CC0-1.0
project('neo-ipa-uguzzi', 'c', 'cpp',
        meson_version : '>= 0.63',
        version : '0.2.2',
        default_options : [
            'werror=true',
            'warning_level=2',
            'cpp_std=c++17',
        ],
        license : 'LGPL 2.1+')

ipa_name = 'neo-ipa-uguzzi'

ipa_install_dir = get_option('libdir') / 'libcamera' / 'ipa-nxp-neo-uguzzi'
live_control_enabled = get_option('live_control').enabled()
integration_tests_enabled = get_option('test')

ipa_data_dir = get_option('datadir') / 'libcamera' /  'ipa'

cc = meson.get_compiler('c')
libcamera_dep = dependency('libcamera')
libcamera_base = dependency('libcamera-base')
libatomic = cc.find_library('atomic', required : false)
libuguzzi = cc.find_library('uguzzi',
                            dirs : [meson.current_source_dir() + '/libs'],
                            required : true)
install_data(meson.current_source_dir() + '/libs/libuguzzi.so',
             install_dir : get_option('libdir'))
if live_control_enabled
    libuguzzi_connect = cc.find_library('uguzzi_connect',
                                        dirs : [meson.current_source_dir() + '/libs'],
                                        required : true)
install_data(meson.current_source_dir() + '/libs/libuguzzi_connect.so',
             install_dir : get_option('libdir'))
endif

ipa_includes = [
    include_directories('dtp_utils'),
    include_directories('inc'),
    include_directories('.'),
]

common_arguments = [
    '-DLIBCAMERA_BASE_PRIVATE',
    '-DUGUZZI_CAMERA_CNT=8',
    '-Wshadow',
]

if live_control_enabled
    common_arguments += [
        '-DUSE_LIVE_CONTROL',
        '-DUGUZZI_CONNECT_COMMAND_MAX_PAYLOAD_SIZE=65536',
    ]
endif
if integration_tests_enabled
    common_arguments += [
        '-DUGUZZI_TESTS_ENABLED',
    ]
endif

c_arguments = []
cpp_arguments = []

if cc.get_id() == 'clang'
    if cc.version().version_compare('<5')
        error('clang version is too old, neo-ipa-uguzzi requires 5.0 or newer')
    endif

    # Use libc++ by default if available instead of libstdc++ when compiling
    # with clang.
    if cc.find_library('libc++', required: false).found()
        cpp_arguments += [
            '-stdlib=libc++',
        ]
    endif

    cpp_arguments += [
        '-Wextra-semi',
    ]
endif

c_arguments += common_arguments
cpp_arguments += common_arguments

add_project_arguments(c_arguments, language : 'c')
add_project_arguments(cpp_arguments, language : 'cpp')

neo_ipa_uguzzi_files = []

neo_ipa_uguzzi_deps = [
    libcamera_base,
    libcamera_dep,
    libuguzzi,
]

if live_control_enabled
    neo_ipa_uguzzi_deps += [
        libuguzzi_connect,
    ]
endif

subdir('cam_helper')
subdir('data')
subdir('dtp_utils')
subdir('inc')
subdir('src')
subdir('utils')

if live_control_enabled
    subdir('live_control')
endif

if integration_tests_enabled
    subdir('tests')
endif

mod = shared_module(ipa_name,
                    [neo_ipa_uguzzi_files],
                    name_prefix : '',
                    include_directories : ipa_includes,
                    dependencies : neo_ipa_uguzzi_deps,
                    link_with : libipa,
                    link_whole : nxp_ipa_cam_helper_lib,
                    install : true,
                    install_dir : ipa_install_dir)
