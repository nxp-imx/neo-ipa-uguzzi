.. SPDX-License-Identifier: CC-BY-SA-4.0

Change Log
==========

Release lf-6.12.20_2.0.0 (June 2025)
====================================

Associated versions
-------------------

* IPA release: lf-6.12.20_2.0.0
* IPA version: UGUZZI_IPA_v0.2.2
* libcamera release: lf-6.12.20_2.0.0
* libcamera version: v0.5.0
* uGuzzi version: 2025. 2.24[1]
* Tuning Tool version: 0.94.99
* libuguzzi.so sha256sum: c40fad89151916d01d86d5f857b6c079a00520b59de1a8c8c93774a1943074a3
* libuguzzi_connect.so sha256sum: 37cddebb75aa12a39cf897ca659ce31c6d7b087396c22d9a17c7ace84e47ac81

Added
-------

* Support for the os08a20 sensor 3840x2160 - 12bpp mode
* Add configuration file config.yaml with sensor configuration defined by user

Changed
-------

* Change the ox03c10 sensor HDR mode to HDR4 - 4 exposures (versus HDR3)
* The socket port number has changed and is defined per sensor entry in config.yaml​
* Any new additional sensor support should have its model or entity entry in config.yaml
* The sensor to connect with Tuning Tool can be configured using the "entity-filter" setting​ from config.yaml
* The subset of parameters INALIGN and LPALIGN from the Pipeline Configuration ISP block is now applied to the ISP. For consistency, DTPs MUST now be configured with those parameters as follows:​
   * Update flag for PIPE_CONF set to "ON_CONFIG"​
   * INALIGN0/1 set to 1 (MSB aligned) - ISI hardware constraint​
   * LPALIGN0/1 set according to the calibration needs

Release lf-6.12.3_1.0.0 (March 2025)
====================================

Associated versions
-------------------

* IPA release: lf-6.12.3_1.0.0
* IPA version: UGUZZI_IPA_v0.2.0
* libcamera release: lf-6.12.3_1.0.0
* libcamera version: v0.3.2
* uGuzzi version: 2025. 2.24[1]
* Tuning Tool version: 0.94.99
* libuguzzi.so sha256sum: c40fad89151916d01d86d5f857b6c079a00520b59de1a8c8c93774a1943074a3
* libuguzzi_connect.so sha256sum: 37cddebb75aa12a39cf897ca659ce31c6d7b087396c22d9a17c7ace84e47ac81

Changed
-------

* Initial release
* Support for the ox03c10 sensor
* Support the AEC, AWB and AGLCE algorithms from uGuzzi for color and brightness management, tone mapping, automatic global and local contrast enhancement.