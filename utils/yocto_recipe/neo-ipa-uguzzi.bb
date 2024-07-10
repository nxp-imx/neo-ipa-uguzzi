SUMMARY = "Linux libcamera neo out-of-tree MMS uGuzzi IPA"
SECTION = "libs"

LICENSE = "LGPL-2.1-or-later & Proprietary"

LIC_FILES_CHKSUM = "\
    file://LICENSES/LGPL-2.1-or-later.txt;md5=3c328714bf889b2c3c7cd842e3e4893b \
    file://LICENSES/LA_OPT_NXP_Software_License.txt;md5=b5ee000acad87f2e615a327bfe6f0828 \
"

SRC_URI = "git://github.com/nxp-imx/neo-ipa-uguzzi;protocol=https;branch=lf-6.12.3_1.0.0"
SRCREV = "${AUTOREV}"

S = "${WORKDIR}/git"

DEPENDS = "libcamera"

EXTRA_OEMESON = " \
"

inherit meson pkgconfig

FILES:${PN} += " ${libdir}/ ${datadir}/libcamera/ipa "

INSANE_SKIP:${PN} += "already-stripped"
FILES_SOLIBSDEV = ""
