.. SPDX-License-Identifier: CC-BY-SA-4.0

Change Log
==========

Release lf-6.18.20_2.0.0 (June 2026)
====================================

Associated versions
-------------------

* IPA release: lf-6.18.20_2.0.0
* IPA version: UGUZZI_IPA_v0.2.7
* libcamera release: lf-6.18.20_2.0.0
* libcamera version: v0.7.1
* uGuzzi version: 2025. 7.24[1]
* Tuning Tool version: 0.95.07
* libuguzzi.so sha256sum: 1d7f7721b74e52563bdebc088de63169f2a15771fd899bde7d2e640a20791318
* libuguzzi_connect.so sha256sum: 8cf435a34b31647f661564c07563beabf1ec41c533b8d2698bbeb5aac590655d

Added
-------

* Enable ox05b1s multi-capture controls in RGBIr Dual Mode
* Add camhelper for vivid cameras
* Enable uGuzzi processing with multi channels

Changed
-------

* Use v4l2-isp generic framework for params and stats
* Update IPA NEO mojom interface with 2026 Q2 release
   * Enums transformed as scoped enums
* Remove rgbIr member from camHelper attributes
* Rename the CameraHelper setControls into sensorControlList
* Change the uGuzzi AE mode to non-harmonized
* Synchronize header files to libcamera v0.7.1
* Update for libcamera v0.7.1:
   * Replace usage of YamlObject class by ValueNode

Release lf-6.18.2_1.0.0 (March 2026)
====================================

Associated versions
-------------------

* IPA release: lf-6.18.2_1.0.0
* IPA version: UGUZZI_IPA_v0.2.6
* libcamera release: lf-6.18.2_1.0.0
* libcamera version: v0.6.0
* uGuzzi version: 2025. 7.24[1]
* Tuning Tool version: 0.95.07
* libuguzzi.so sha256sum: 1d7f7721b74e52563bdebc088de63169f2a15771fd899bde7d2e640a20791318
* libuguzzi_connect.so sha256sum: 8cf435a34b31647f661564c07563beabf1ec41c533b8d2698bbeb5aac590655d

Added
-------

* Add camera helper support for ox05b1s sensor
* Add camera helper support for ar0235 sensor
* Add camera helper support for sensors commonly used by community:
   * Sony imx219 (RPi camera module 2)
   * Sony imx519 (Arducam imx519 module)
   * Sony imx708 (RPi camera module 3)
* Add support for lens control
* Report the AutoFocus statistics to uGuzzi
* Enable the AutoFocus algorithm at the condition that the sensor supports lens control
* Add ox05b1s DTP for RGBIr normal mode, resolution 2592x1944 and 10 bit depth

Changed
-------

* Update IPA NEO mojom interface with 2026 Q1 release
   * Remove IPAContextType from setSensorControls
* Update the NXP neoisp uapi file with v6.18 kernel
* Synchronize header files to libcamera v0.6.0
* Change the CameraHelper::controlListSetAGC() prototype where span of values for the exposure and gain is provided instead of a single value.
* Remove the sensor WB gains reset to 1.0 when WB gains apply in ISP

Fixed
-------

* Solve issue where pink artifact was observed on bright area
   * The ox03c10 SPD min again is changed to 4.5 (instead of 1.0)
* Solve ON_CONFIG settings not applied in a config/start/stop/start calls sequence

Release lf-6.12.49-2.2.0 (December 2025)
====================================

Associated versions
-------------------

* IPA release: lf-6.12.49-2.2.0
* IPA version: UGUZZI_IPA_v0.2.5
* libcamera release: lf-6.12.49-2.2.0
* libcamera version: v0.5.2
* uGuzzi version: 2025. 7.24[1]
* Tuning Tool version: 0.95.07
* libuguzzi.so sha256sum: 1d7f7721b74e52563bdebc088de63169f2a15771fd899bde7d2e640a20791318
* libuguzzi_connect.so sha256sum: 8cf435a34b31647f661564c07563beabf1ec41c533b8d2698bbeb5aac590655d

Added
-------

* Give access to Image1 buffer for live tuning
* Add stream mode trigger for the selected tuning configuration
* Use multi-capture controls in HDR mode for the os08a20 sensor
* Add os08a20 DTP with HDR configuration for the 3840x2160 resolution and 12 bit depth
* Add checksum verification on embedded data for the ox03c10 sensor
* Rename the DTPs explicitly with supported sensor mode, resolution and bit depth

Changed
-------

* Update IPA NEO mojom interface with 2025 Q4 release
* Update with extended metadata interface which provides an abstraction access to the NEO ISP driver metadata.
* Calculate the sensor line duration based on the sensor hblank
* Calculate the sensor VTS based on the sensor height and vblank
* Define the socket port for the sensor model instead of the sensor entity
   * The socket port is the same for any sensor entity from the same model
   * This change applies in the IPA configuration file

Fixed
-------

* Fix swapped HDR merge input0 and input1 setting

Release lf-6.12.34-2.1.0 (September 2025)
====================================

Associated versions
-------------------

* IPA release: lf-6.12.34-2.1.0
* IPA version: UGUZZI_IPA_v0.2.3
* libcamera release: lf-6.12.34-2.1.0
* libcamera version: v0.5.1
* uGuzzi version: 2025. 2.24[1]
* Tuning Tool version: 0.94.99
* libuguzzi.so sha256sum: c40fad89151916d01d86d5f857b6c079a00520b59de1a8c8c93774a1943074a3
* libuguzzi_connect.so sha256sum: 37cddebb75aa12a39cf897ca659ce31c6d7b087396c22d9a17c7ace84e47ac81

Changed
-------

* Update IPA NEO mojom interface with 2025 Q3 release

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
* Add configuration file config_ipa_uguzzi.yaml with sensor configuration defined by user

Changed
-------

* Change the ox03c10 sensor HDR mode to HDR4 - 4 exposures (versus HDR3)
* The socket port number has changed and is defined per sensor entry in config_ipa_uguzzi.yaml
* Any new additional sensor support should have its model or entity entry in config_ipa_uguzzi.yaml
* The sensor to connect with Tuning Tool can be configured using the "entity-filter" setting​ from config_ipa_uguzzi.yaml
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