# SPDX-License-Identifier: CC0-1.0

conf_files = files([
    'database_mx95mbcam_1920-1280_16bpp.bin',
    'database_os08a20_SDR_3840-2160_12bpp.bin',
    'database_os08a20_HDR_3840-2160_12bpp.bin',
    'database_ox05b1s_dual_2592-1944_10bpp.bin',
    'config_ipa_uguzzi.yaml',
])

install_data(conf_files,
             install_dir : ipa_data_dir / 'nxp' / 'neo' / 'uguzzi',
             install_tag : 'runtime')
