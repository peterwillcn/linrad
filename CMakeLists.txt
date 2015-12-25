#
# CMakeLists.txt --- CMake input file for linrad
#
# Copyright (c) 2014 Juergen Kahrs
#
# Permission is hereby granted, free of charge, to any person 
# obtaining a copy of this software and associated documentation 
# files (the "Software"), to deal in the Software without restriction, 
# including without limitation the rights to use, copy, modify, 
# merge, publish, distribute, sublicense, and/or sell copies of 
# the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be 
# included in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES 
# OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT 
# HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
# WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE 
# OR OTHER DEALINGS IN THE SOFTWARE.

## process this file with CMake to produce Makefile

# Support for NASM is available only with CMake 2.8.8 or later.
cmake_minimum_required (VERSION 2.6)
project (linrad)

# Read file vernr.h and extract the parts of the version number.
FILE(   READ  vernr.h VERNR_H )
STRING( REGEX MATCH   "PROGRAM_NAME \"Linrad-[0-9][0-9].[0-9][0-9]" LINRAD_VERSION        "${VERNR_H}")
STRING( REGEX REPLACE "PROGRAM_NAME \"Linrad-"                  ""  LINRAD_VERSION        "${LINRAD_VERSION}")
STRING( REGEX REPLACE "([0-9][0-9]).[0-9][0-9]"              "\\1"  LINRAD_MAJOR_VERSION  "${LINRAD_VERSION}")
STRING( REGEX REPLACE "[0-9][0-9].([0-9][0-9])"              "\\1"  LINRAD_MINOR_VERSION  "${LINRAD_VERSION}")
STRING( REGEX REPLACE "[0-9][0-9].[0-9][0-9](.*)"            "\\1"  LINRAD_BUGFIX_VERSION "${LINRAD_VERSION}")

include(CheckIncludeFile)
include(CheckTypeSize)
include(CheckLibraryExists)
include(CheckPrototypeDefinition)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
# Set build type. We do this only to prevent compile flag -O3.
set(CMAKE_BUILD_TYPE Debug)

# Create empty files to be included by the .c sources.
# Instead of using these files we will set auto-detected definitions below.
file(WRITE ${CMAKE_SOURCE_DIR}/conf.h  "/* empty file, all settings defined by CMake. */\n" )
file(WRITE ${CMAKE_SOURCE_DIR}/lconf.h "/* empty file, all settings defined by CMake. */\n" )

set(EXTRA_LIBS "")
set(EXTRA_LIB_LOC "")

set(CMAKE_THREAD_PREFER_PTHREAD 1)
include(FindThreads)
if (NOT CMAKE_USE_PTHREADS_INIT)
  message(STATUS " Couldn't find pthreads")
endif()

if (MINGW)
  #set(EXTRA_LIB_LOC "${EXTRA_LIB_LOC} /usr/x86_64-w64-mingw32/sys-root/mingw/bin/")
  #message(STATUS " EXTRA_LIB_LOC = ${EXTRA_LIB_LOC}")
else()
  file(WRITE ${CMAKE_SOURCE_DIR}/conf.h  "#include <sys/ioctl.h>\n" )
  file(WRITE ${CMAKE_SOURCE_DIR}/lconf.h "#include <sys/ioctl.h>\n" )

  include(FindX11)
  if (X11_FOUND)
    include_directories(${X11_INCLUDE_DIR})
    set(EXTRA_LIBS ${EXTRA_LIBS} ${X11_LIBRARIES})
  else()
    message(STATUS " Couldn't find X11 headers or libraries")
  endif()

  if (X11_Xkb_FOUND)
    include_directories(${X11_Xkb_INCLUDE_PATH})
    add_definitions(-DHAVE_XKBLIB32=1)
    add_definitions(-DHAVE_XKBLIB64=1)
  else()
    message(STATUS " Couldn't find XKB headers or libraries")
  endif()

  if (X11_XShm_FOUND)
    include_directories(${X11_XShm_INCLUDE_PATH})
    add_definitions(-DSHM_INSTALLED=1)
  else()
    message(STATUS " Couldn't find XShm headers or libraries")
    add_definitions(-DSHM_INSTALLED=0)
  endif()

  # Would the compiler be fit to build FreeOCL ?
  execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
  # Try to find an OpenCL implementation and declare its presence.
  include(FindOpenCL)
  if (OPENCL_FOUND)
    add_definitions(-DOPENCL_PRESENT=1)
    include_directories( ${OPENCL_INCLUDE_DIRS} extlibs/clFFT/src/include )
    set(EXTRA_LIBS ${EXTRA_LIBS} ${OPENCL_LIBRARIES})
    # Our source shall be portable.
    # Ask the local OpenCL API for permission to use deprecated
    # functions of the old OpenCL 1 API.
    # http://stackoverflow.com/questions/28500496/opencl-function-found-deprecated-by-visual-studio
    add_definitions(-DCL_USE_DEPRECATED_OPENCL_2_0_APIS=1)
  elseif (GCC_VERSION VERSION_GREATER 4.7 OR GCC_VERSION VERSION_EQUAL 4.7)
    message(STATUS "Found no commercial OpenCL library; using FreeOCL instead")
    # If OpenCL is missing in this build environment, then build FreeOCL.
    # The header files of FreeOCL allow us to build our own software when 
    # it refers to OpenCL headers. Our software will be linked against
    # the libOpenCL that comes with FreeOCL. At run-time of our software, the
    # libOpenCL tries to load any vendor-specific OpenCL library that it can
    # find referenced in any ICD file under /etc/OpenCL/vendors. If you want
    # libFreeOCL.so to be found there, then you will probably want libFreeOCL
    # be installed in its default location by "make install".
    # This is a bit awkward but it allows to find all ICDs, not just FreeOCL.
    add_definitions(-DOPENCL_PRESENT=1)
    set(OPENCL_FOUND 1)
    set (OPENCL_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/extlibs/freeocl)
    set (OPENCL_LIBRARIES OpenCL)
    include_directories(${OPENCL_INCLUDE_DIRS})
    include_directories(${OPENCL_INCLUDE_DIRS}/src)
    include_directories(${OPENCL_INCLUDE_DIRS}/include)
    set(FreeOCL_SOURCE_DIR ${OPENCL_INCLUDE_DIRS})
    add_subdirectory(${OPENCL_INCLUDE_DIRS}/src)
    set_target_properties(FreeOCL PROPERTIES COMPILE_FLAGS "-std=c++11 -DFREEOCL_USE_CXX0X -DFREEOCL_ARCH_i386 -DFREEOCL_OS_LINUX")
    #install(TARGETS OpenCL  LIBRARY DESTINATION lib${LIB_SUFFIX})
    #install(TARGETS FreeOCL LIBRARY DESTINATION lib${LIB_SUFFIX})
    configure_file(${OPENCL_INCLUDE_DIRS}/config.h.cmake ${OPENCL_INCLUDE_DIRS}/include/FreeOCL/config.h)
    file(WRITE ${OPENCL_INCLUDE_DIRS}/icd/freeocl.icd "libFreeOCL.so")
    include_directories(extlibs/clFFT/src/include)

    #message(STATUS "Found no commercial OpenCL library; using POCL instead")
    # If OpenCL is missing in this build environment, then build POCL.
    # We would prefer POCL over FreeOCL, but as of POCL release 0.11
    # the CMake support of POCL is not yet ready to be used by us.
    # The POCL library could be linked statically into our software,
    # thus allowing us to avoid the tricky dynamic linking mentioned above.
    #http://krblogs.com/post/64565799647/opencl-on-arm-linux
    #https://nerdrambles.wordpress.com/2013/05/10/pocl-ported-on-arm-chromebook/
    #add_definitions(-DOPENCL_PRESENT=1)
    #set(OPENCL_FOUND 1)
    #add_definitions(-DENABLE_ICD=ON)
    #add_definitions(-DDIRECT_LINKAGE=ON)
    #add_definitions(-DINSTALL_OPENCL_HEADERS=OFF)
    #add_definitions(-DHAVE_OPENCL_HPP=OFF)
    #add_definitions(-DSTATIC_LLVM=ON)
    #include_directories(/usr/include/hwloc)
    #add_subdirectory(extlibs/pocl)
  else()
    message(STATUS "No OpenCL support")
    add_definitions(-DOPENCL_PRESENT=0)
    set(OPENCL_FOUND 0)
  endif() # OPENCL_FOUND

endif() # NOT MINGW

if (   ${CMAKE_SYSTEM_NAME} STREQUAL Linux)
  add_definitions(-DOSNUM=1)
  add_definitions (-DDARWIN=0)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL Windows)
  add_definitions(-DOSNUM=2)
  add_definitions (-DDARWIN=0)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL FreeBSD)
  add_definitions(-DOSNUM=3)
  add_definitions(-DBSD=1)
  add_definitions(-DDARWIN=0)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL Darwin)
  add_definitions(-DOSNUM=4)
  add_definitions(-DDARWIN=1)
else()
  # Every unknown OS will be handled like Linux.
  add_definitions(-DOSNUM=1)
  add_definitions(-DDARWIN=0)
endif()

if (${CMAKE_SIZEOF_VOID_P} STREQUAL 8)
  set(IA64 1)
else()
  set(IA64 0)
endif()
add_definitions(-DIA64=${IA64})

# Set CPU to ARM regardless of actual platform.
# This is necessary to avoid the usage of assembly language sources.
set (CPU "CPU_ARM")
message(STATUS "Found CPU type \"${CMAKE_SYSTEM_PROCESSOR}\" on target system")
if (
    "${MINGW}"                                    OR
    "${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86"    OR
    "${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64" 
   )
  if (${CMAKE_VERSION} VERSION_GREATER "2.8.7")
    message(STATUS "Checking for NASM compliance in CMake 2.8.8 or later")
    include(CheckLanguage)
    check_language(ASM_NASM)
  else()
    message(STATUS "No NASM compliance before CMake 2.8.8")
  endif()
  if(CMAKE_ASM_NASM_COMPILER)
    set (CPU "CPU_INTEL")
    set(CMAKE_ASM_NASM_SOURCE_FILE_EXTENSIONS s)
    if (${IA64} STREQUAL 0)
      if (MINGW)
        set(CMAKE_ASM_NASM_OBJECT_FORMAT "win32 -w-orphan-labels -D OS_WINDOWS")
      else()
        set(CMAKE_ASM_NASM_OBJECT_FORMAT "elf   -w-orphan-labels -D OS_LINUX ")
      endif()
    else()
      if (MINGW)
        set(CMAKE_ASM_NASM_OBJECT_FORMAT "win64 -w-orphan-labels -D OS_WINDOWS ")
      else()
        set(CMAKE_ASM_NASM_OBJECT_FORMAT "elf64 -w-orphan-labels -D OS_LINUX ")
      endif()
    endif()
    enable_language(ASM_NASM)
  endif()
endif()
add_definitions(-DCPU=${CPU})

add_definitions(-DALSA_LIBNAME=\"asound\")
add_definitions(-DFTDI_LIBNAME=\"ftdi\")
add_definitions(-DLIBUSB0_LIBNAME=\"loadusb\")
add_definitions(-DLIBUSB1_LIBNAME=\"libusb-1.0\")
add_definitions(-DUDEV_LIBNAME=\"libudev\")
add_definitions(-DBLADERF_LIBNAME=\"bladeRF\")
add_definitions(-DMIRISDR_LIBNAME=\"mirisdr\")
add_definitions(-DPERSEUS_LIBNAME=\"perseus-sdr\")
add_definitions(-DRTLSDR_LIBNAME=\"rtlsdr\")
add_definitions(-DFDMS1_LIBNAME=\"fdms1\")
add_definitions(-DAIRSPY_LIBNAME=\"airspy\")

#check_include_file(portaudio.h PORTAUDIO_PRESENT)
#check_library_exists(portaudio-2 Pa_Initialize "/usr/x86_64-w64-mingw32/sys-root/mingw/bin/" PORTAUDIO_PRESENT)
#check_library_exists(portaudio Pa_Initialize "${EXTRA_LIB_LOC}" PORTAUDIO_PRESENT)
if (${PORTAUDIO_PRESENT})
  add_definitions(-DPORTAUDIO_PRESENT=1)
  set(CMAKE_EXTRA_INCLUDE_FILES portaudio.h)
  check_type_size(PaStream PASTREAMSIZE)
  set(CMAKE_EXTRA_INCLUDE_FILES)
  if (${HAVE_PASTREAMSIZE} STREQUAL "")
    add_definitions(-DPA_LINUX_REALTIME=0)
  else()
    add_definitions(-DPA_LINUX_REALTIME=1)
  endif()
  set(EXTRA_LIBS ${EXTRA_LIBS} portaudio)
else()
  add_definitions(-DPORTAUDIO_PRESENT=0)
  add_definitions(-DPA_LINUX_REALTIME=0)
endif()
add_definitions(-DPA_LIBNAME=\"portaudio\")

#check_include_file(jack/jack.h JACK_PRESENT)
#if (${JACK_PRESENT})
#  add_definitions(-DJACK_PRESENT=1)
#  set(EXTRA_LIBS ${EXTRA_LIBS} jack)
#else()
#  add_definitions(-DJACK_PRESENT=0)
#endif()

check_include_file(linux/soundcard.h HAVE_OSS)
if (${HAVE_OSS})
  add_definitions(-DHAVE_OSS=1)
  # TODO: This is weird and works only for emulated OSS on ALSA.
  file(APPEND ${CMAKE_SOURCE_DIR}/lconf.h "#include <linux/soundcard.h>\n" )
  file(APPEND ${CMAKE_SOURCE_DIR}/conf.h "#define " SND_DEV " \"/dev/dsp\"\n" )
else()
  add_definitions(-DHAVE_OSS=0)
endif()
add_definitions(-DSOFT66_LIBNAME=\"soft66\")

if (${IA64} STREQUAL 0)
  add_definitions(-DPCI_LIBNAME=\"wd_dask\")
else()
  add_definitions(-DPCI_LIBNAME=\"wd_dask64\")
endif()


# This list of files should be sorted later
# so that the sequence makes sense to a reader.
set (LINRAD_SOURCES
  ui.c
  uivar.c
  screen.c
  mouse.c
  fonts.c
  screenvar.c
  screensub.c
  sigvar.c
  modesub.c
  palette.c
  extio.c
  thrvar.c
  tx.c
  txssb.c
  txvar.c
  hwaredriver.c
  wide_graph.c
  pol_graph.c
  afc_graph.c
  afcsub.c
  coh_osc.c
  network.c
  eme.c
  help.c
  wxsys.c
  keyboard_var.c
  powtvar.c
  powtim.c
  tx_graph.c
  fft0.c
  fft1.c
  fft1_re.c
  fft2.c
  fft3.c
  fft3var.c
  llsqvar.c
  fft1var.c
  fft2var.c
  fft3var.c
  calvar.c
  caliq.c
  calsub.c
  selvar.c
  buf.c
  blnkvar.c
  coherent.c
  cohsub.c
  freq_control.c
  gifsave.c
  calsub2.c
  llsq.c
  cwspeed.c
  cwdetect.c
  sdrip.c
  sdrvar.c
  baseb_graph.c
  wse.c
  hires_graph.c
  meter_graph.c
  rxin.c
  rxout.c
  wcw.c
  mix1.c
  menu.c
  tune.c
  morse.c
  calibrate.c
  spur.c
  spursub.c
  timing.c
  excalibur.c
  sdr14.c
  radar.c
  timf2.c
  pavar.c
  blank1.c
  mix2.c
  fm.c
  adtest.c
  perseus.c
  sellim.c
  txtest.c
  elektor.c
  fcdpp.c
  si570.c
  usb2lpt.c
  rtl2832.c
  bladerf.c
  mirics.c
  pcie9842.c
  pa.c
  hwaredriver.c
  openhpsdr.c
  httpd.c
  afedri.c
  soft66.c
  affinity.c
  loadusb.c
  elad.c
  airspy.c
  soundcard.c
)
 
if (NOT MINGW)
  set(LINRAD_SOURCES ${LINRAD_SOURCES} html_server.c)
  add_definitions(-DSERVER=1)
endif()

if (EXISTS ${CMAKE_SOURCE_DIR}/users_hwaredriver.c)
  add_definitions(-DLUSERS_ROUTINES_PRESENT=1)
else()
  add_definitions(-DLUSERS_ROUTINES_PRESENT=0)
endif()
if (EXISTS ${CMAKE_SOURCE_DIR}/wusers_hwaredriver.c)
  add_definitions(-DWUSERS_ROUTINES_PRESENT=1)
else()
  add_definitions(-DWUSERS_ROUTINES_PRESENT=0)
endif()
if (EXISTS ${CMAKE_SOURCE_DIR}/users_hwaredef.h)
  add_definitions(-DLUSERS_DEFINES_PRESENT=1)
else()
  add_definitions(-DLUSERS_DEFINES_PRESENT=0)
endif()
if (EXISTS ${CMAKE_SOURCE_DIR}/wusers_hwaredef.h)
  add_definitions(-DWUSERS_DEFINES_PRESENT=1)
else()
  add_definitions(-DWUSERS_DEFINES_PRESENT=0)
endif()
if (EXISTS ${CMAKE_SOURCE_DIR}/users_extra.c)
  add_definitions(-DUSERS_EXTRA_PRESENT=1)
else()
  add_definitions(-DUSERS_EXTRA_PRESENT=0)
endif()

# After completing the detection of the build environment,
# we can now decide about the external libraries to build upon it.
add_subdirectory(extlibs)

if (${CPU} STREQUAL "CPU_INTEL")
  if (${IA64} STREQUAL 0)
    set(LINRAD_SOURCES ${LINRAD_SOURCES} split.s fft2mmxa.s fft2mmxb.s fft2mmxc.s fftasm.s getiq.s simdasm.s hware.s  )
  else()
    set(LINRAD_SOURCES ${LINRAD_SOURCES} split64.s fft2mmxa64.s fft2mmxb64.s fft2mmxc64.s fftasm64.s getiq64.s simdasm64.s hware64.s )
  endif()
else()
  set(LINRAD_SOURCES ${LINRAD_SOURCES} csplit.c)
endif()

# TODO: Leif commented on this:
# On a linux platform with mingw32 as well as X11 and perhaps also svgalib,
# the make command should generate binaries for Windows, X11, terminal mode
# with svgalib as well as terminal mode with fbdev.
if (${MINGW})
  set(LINRAD_SOURCES ${LINRAD_SOURCES} wmain.c wsys.c winvar.c wsetad.c wsdr.c hidwin.c)
  set(EXTRA_LIBS ${EXTRA_LIBS} ws2_32 winmm gdi32 setupapi ole32)
elseif(X11_FOUND)
  set(LINRAD_SOURCES ${LINRAD_SOURCES} xmain.c xvar.c xsys.c lxsys.c lvar.c lsetad.c lsdr.c hid.c loadalsa.c)
else()
  # In the absence of X11 and svgalib we have no proper main.c yet.
  message(STATUS " To compile for Linux you need either libX11.so or libvga.so")
  message(STATUS " Can also not compile for Windows due to missing mingw or libraries.")
  message(FATAL_ERROR " Not enough packages on this system to compile anything.")
endif()

# Build clinrad, the only executable that can be built with CMake.
# clinrad uses xmain.c as the main program.
# Therefore clinrad should behave exactly as xlinrad.
add_executable (clinrad ${LINRAD_SOURCES})
add_definitions(-Wall -Werror)
target_link_libraries (clinrad m ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_DL_LIBS} ${EXTRA_LIBS})
install(PROGRAMS ${CMAKE_BINARY_DIR}/clinrad${CMAKE_EXECUTABLE_SUFFIX} DESTINATION linrad)

add_executable (sim2perseus sim2perseus.c)
target_link_libraries (sim2perseus)
install(PROGRAMS ${CMAKE_BINARY_DIR}/sim2perseus${CMAKE_EXECUTABLE_SUFFIX} DESTINATION linrad)

if (OPENCL_FOUND)
  add_executable (oclprogs oclprogs.c)
  target_link_libraries (oclprogs m ${CMAKE_THREAD_LIBS_INIT} ${OPENCL_LIBRARIES} ${CMAKE_DL_LIBS} clFFT)
  # Build the clinfo executable.
  include_directories(extlibs/clinfo/src/)
  add_executable (clinfo extlibs/clinfo/src/clinfo.c)
  target_link_libraries (clinfo m ${CMAKE_THREAD_LIBS_INIT} ${OPENCL_LIBRARIES} ${CMAKE_DL_LIBS})
endif()

# Build hlinrad, the executable that implements a medium wave radio.
# It is meant to be a prototype of a different GUI architecture.
# The simplistic GUI is implemented without access to any audiovisual devices.
# Instead, the complete GUI is implemented across a network connection
# with the help of HTTP, the protocol used for web browsers.
if (${PCIE9842_PRESENT})
  add_executable (hlinrad hmain.c httpd.c)
  target_link_libraries (hlinrad m ${PCIE9842_LIBNAME})
  install(PROGRAMS ${CMAKE_BINARY_DIR}/hlinrad${CMAKE_EXECUTABLE_SUFFIX} DESTINATION linrad)
endif()

# Put the executable into a proper installer file.
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "This is Linrad ${LINRAD_VERSION}")
set(CPACK_PACKAGE_VENDOR "Leif Asbrink (sm5bsz)")
set(CPACK_PACKAGE_NAME "Linrad")
set(CPACK_PACKAGE_VERSION "${LINRAD_VERSION}")
set(CPACK_PACKAGE_VERSION_MAJOR "${LINRAD_MAJOR_VERSION}")
set(CPACK_PACKAGE_VERSION_MINOR "${LINRAD_MINOR_VERSION}")
set(CPACK_PACKAGE_VERSION_PATCH "${LINRAD_BUGFIX_VERSION}")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README  "${CMAKE_SOURCE_DIR}/z_COPYRIGHT.txt")
set(CPACK_PACKAGE_CONTACT "sm5bsz@users.sourceforge.net")
install(DIRECTORY ${CMAKE_SOURCE_DIR}/
        DESTINATION linrad
        FILES_MATCHING
          PATTERN "par_*"
          PATTERN "*.[chs]"
          PATTERN "*.in"
          PATTERN "*.txt"
          PATTERN "*.lir"
          PATTERN "clean"
          PATTERN "configure"
          PATTERN "configure.exe"
          PATTERN "LICENSE"
          PATTERN "make.bat"
          PATTERN "svga"
          PATTERN "usb_sdr14"
          PATTERN "*.cmake"
          PATTERN "*.ico"
          PATTERN "build*" EXCLUDE
          PATTERN ".svn"   EXCLUDE
)

if (${MINGW})
  set(CPACK_GENERATOR "NSIS")
  set(CPACK_NSIS_INSTALL_ROOT "C:")
  set(CPACK_NSIS_MENU_LINKS "http://sourceforge.net/projects/linrad" "Linrad")
  set(CPACK_NSIS_MUI_ICON    "${CMAKE_SOURCE_DIR}/cmake/linrad.ico")
  set(CPACK_NSIS_MUI_UNIICON "${CMAKE_SOURCE_DIR}/cmake/linrad.ico")
  set(CPACK_NSIS_MODIFY_PATH true)
  set(CPACK_PACKAGE_EXECUTABLES "clinrad" "${CPACK_PACKAGE_DESCRIPTION_SUMMARY}")
  set(CPACK_NSIS_EXECUTABLES_DIRECTORY "linrad")
  set(CPACK_CREATE_DESKTOP_LINKS "Linrad")
  set(CPACK_NSIS_CONTACT "sm5bsz@users.sourceforge.net")
  set(CPACK_NSIS_DISPLAY_NAME "Linrad")
  include(CPack)
else()
  set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}_${CPACK_PACKAGE_VERSION}_${CMAKE_SYSTEM_PROCESSOR}")
  set(CPACK_PACKAGING_INSTALL_PREFIX "usr/local")
  if     (CPACK_GENERATOR STREQUAL "TGZ")
    include(CPack)
  elseif (CPACK_GENERATOR STREQUAL "RPM")
    include(CPack)
  elseif (CPACK_GENERATOR STREQUAL "DEB")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "${CPACK_PACKAGE_VENDOR} <${CPACK_PACKAGE_CONTACT}>")
    set(CPACK_DEBIAN_PACKAGE_SECTION "comm")
    set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
    set(CPACK_DEBIAN_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})
    set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "http://sourceforge.net/projects/linrad")
    set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.3.1-6), libgcc1 (>= 1:3.4.2-12)")
    include(CPack)
  else()
    # No package format selected. Don't create the "package" target.
  endif()
  if     (CPACK_GENERATOR)
    message(STATUS "CPACK_GENERATOR set to ${CPACK_GENERATOR}")
  endif()
endif()


