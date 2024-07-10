.. SPDX-License-Identifier: CC-BY-SA-4.0

************
Introduction
************

This is the uGuzzi IPA for the libcamera NXP NEO pipeline.

The uGuzzi IPA should be used with a matching version of the libcamera.

Respective versions are associated to NXP BSP quaterly release and is reflected in the branch name that is common to the two components.

How to build with meson?
=========================

Prerequisites
-------------

* Installation of Meson
* Installation of the Yocto SDK toolchain

Update SDK with compatible version of libcamera
-----------------------------------------------

Clone the libcamera version compatible with the release uGuzzi IPA version.

From the cross-build environment, run following script at the root of the libcamera source tree:

.. code-block:: shell

  #!/bin/bash -ex
  
  SDK=<YOUR_SDK_PATH>
    
  # setup build libcamera
  meson setup  --buildtype=plain -Dprefix=/usr build -Dpipelines="nxp/neo" -Dv4l2=true -Dcam=enabled -Dlc-compliance=disabled -Dtest=false -Ddocumentation=disabled -Dqcam=disabled
  
  # build and install libcamera dependencies in SDK sysroots
  DESTDIR=${SDK}/sysroots/armv8a-poky-linux ninja -C build install

uGuzzi IPA build instructions
-----------------------------

From the cross-build environment, run following script at the root of the uGuzzi IPA source tree:

.. code-block:: shell

  #!/bin/bash -ex
  
  # setup build uguzzi IPA
  meson setup --buildtype=plain -Dprefix=/usr build
  
  # build and install in local directory
  DESTDIR=$(realpath .)/install ninja -C build install
 

How to build with Yocto?
========================

The reference recipe ``neo-ipa-uguzzi.bb`` provided from the ``<neo-ipa-uguzzi>/utils/yocto_recipe/`` can be used to compile the uGuzzi IPA with Yocto.

This recipe should be added into an existing meta-layer part of the Yocto BSP.


How to enable the uGuzzi IPA?
=============================

By default, the IPA loaded by the NXP NEO pipeline is the NXP enablement IPA.

The uGuzzi IPA can be enabled to run on target by setting following environment variable.
This variable specifies the IPA module search path used by the pipeline handler on the target:

.. code-block:: shell

  export LIBCAMERA_IPA_MODULE_PATH="/usr/lib/libcamera/ipa-nxp-neo-uguzzi" 

How to configure for tuning?
============================

By default, the uGuzzi IPA runs in isolated mode. 

However for tuning and development purposes (connection with the Tuning Tool), the isolation mode should be disabled by setting following environment variable: 

.. code-block:: shell

  export LIBCAMERA_IPA_DISABLE_ISOLATION="yes" 

The IP connection with the uGuzzi IPA can be performed using the port 50069.

How to configure logging?
=========================

By default, the uGuzzi IPA logs are displayed in the debug console. 

It is possible to configure the IPA to redirect the logs into a file by setting following environment variable: 

.. code-block:: shell

  export LIBCAMERA_IPA_UGUZZI_LOG_DIR=”/tmp/” 

With this configuration, the files generated are named with the PID of the IPA: <ipa_pid>.log

How to port a new sensor?
=========================

Integrating a new sensor into the i.MX95 Camera Software Stack consists essentially in the following tasks:

* Writing a Linux Kernel V4L2 driver for this sensor
* Implement a CamHelper class for the new sensor
* Provide a calibration file that the Neo IPA will consume to manage IPA algorithms

 
Please refer to the ``i.MX95 Camera Porting Guide`` document for detailed instructions.