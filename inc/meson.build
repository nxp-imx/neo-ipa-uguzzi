# SPDX-License-Identifier: CC0-1.0

gen_version = meson.project_source_root() /  'scripts' / 'libcamera' / 'gen-version.sh'

# Use vcs_tag() and not configure_file() or run_command(), to ensure that the
# version gets updated with every ninja build and not just at meson setup time.
version_h = vcs_tag(command : [gen_version, meson.project_build_root(), meson.project_source_root(), meson.project_version()],
                    input : 'version.h.in',
                    output : 'version.h',
                    fallback : meson.project_version())

neo_ipa_uguzzi_files += version_h
