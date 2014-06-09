//------------------------------------------------------------------------------
//
//	Revison History and 
//              Simple Library Package Architecture & Build Order
//

//------------------------------------------------------------------------------
 -. Revision History
   2014.05.23
     Add GUI Solution.  

   2014.04.21 
     Stable Filter solution. ( Various Bug Fixed )
	 Broken data bug fixed (Text Overlay / TS Stream)
     Add Micom Senario.

   2014.02.18
     Modify Writing Time Handling. ( bug fixed )
     Add Rate control paramter handling. ( bug fixed )
     Add Message Queue. (apps)
     Modify FileWriter. ( bug fixed - thread base)
     
   2014.02.11
     Add simple MP4 encoding library and test application.
     Suuport Motion Detection.
     Support MP3 Audio Codec.

   2014.01.17
     Support TS Container.
     Support HLS Component.
     Support 3D Image effect. ( 3D Scaler )
	 
   2013.12.02
     First Release.

 -. Last Revision Number
    libnxdvr : 1.0.10

//------------------------------------------------------------------------------
-. Directory Architecture

 --+-- apps    --+-- nxdvrsol     : Simple Blackbox Encoding Application
   |			 |
   |             +-- nxmp4encsol  : Simple Mp4 Encoding Application
   |             |
   |             +-- nxguisol     : Simple GUI Based Blackbox / Player Application
   +-- bin
   |
   +-- include
   |
   +-- lib
   |
   +-- src     --+-- libnxdvrsol  : Blackbox Encoding Manager API
                 |
                 +-- libnxfilters : Base Filter Components
                 |
                 +-- libnxmp4manager : Simple MP4 File Encoding Manager API


//------------------------------------------------------------------------------
-. Build Sequence

 Step 1. Modify build.env file for each system.
     ...
     ARCHDIR   :=  
     KERNDIR   :=
     LIBSDIR   :=
     ....

 Step 2. Build library files.
   
   -. Dependent library for "Blackbox test application"
      [ARCHDIR]/library/src/libion
      [ARCHDIR]/library/src/libnxv4l2
      [ARCHDIR]/library/src/libnxvpu
      [ARCHDIR]/library/src/libnxgraphictools
      [ARCHDIR]/library/src/libnxnmeaparser

   -. Dependent library for "Simple MP4 Encoding application"
      [ARCHDIR]/library/src/libion
      [ARCHDIR]/library/src/libnxv4l2
      [ARCHDIR]/library/src/libnxvpu

      $ cd [ARCHDIR]/library/src/[Each library directory]
      $ make
      $ make install

 Step 3. Build module file.
      $ cd [ARCHDIR]/modules/coda960
      $ make ARCH=arm

 Step 4. Run build script.
      $ cd [SOLUTION]
      $ ./build-blackbox.sh or ./build-mp4.sh


//------------------------------------------------------------------------------
-. Run Sequence

 Step 1. Prepare root file system ( extract "rootfs-tiny.tar.gz" or etc.. )
 
 Step 2. Copy system depedent shared objects.
     ex)
      $ cp -a [ARCHDIR]/library/lib/*.so [ROOTDIR]/usr/lib

 Step 3. Copy Blackbox solution shared objects.
     ex)
      $ cp -a [SOLUTION]/lib/*.so [ROOTDIR]/usr/lib
 
 Step 4. Copy Encoder module driver.
     ex)
      $ cp -a [ARCHDIR]/modules/code960/nx_vpu.ko [ROOTDIR]/root

 Step 5. Copy Application & Resouce file to root file system.
     ex)
      $ cp -a [SOLUTION]/bin * [ROOTDIR]/root
 
 Step 6. System Booting.

 Step 7. Load Encoder Driver ( at Target board )
     ex)
      $ insmod /root/nx_vpu.ko

 Step 8. Run Blackbox Solution applications ( at Target board)
     ex)
      $ /root/nxdvrsol or /root/nxmp4encsol

