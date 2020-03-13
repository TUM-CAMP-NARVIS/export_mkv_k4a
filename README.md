Artekmed Application Framework for Development and Research
===========================================================

Docker Build Howto:
-------------------

This requires a recent version of docker (18.09+)
get the deployment key for artekmed_p2

DOCKER_BUILDKIT=1 docker image build --ssh github=./config/id_rsa -t artekmed_p2:v0.0.1 --label artekmed_p2_testing .


ARTEKMED Prototype II
---------------------

Prototype to explore 3D Pointcloud fusion and rendering.

by ulrich eck <ulrich.eck@tum.de>



Installation of dependencies on Ubuntu Linux 18.04
++++++++++++++++++++++++++++++++++++++++++++++++++


- Install Desktop Version X64

- Install system packages
 ```
  $ sudo apt-get update
  $ sudo apt-get upgrade
  $ sudo apt-get install build-essential libv4l-dev qv4l2 opencl-headers ocl-icd-opencl-dev libusb-1.0-0-dev libgtk2.0-dev pkg-config libomp-dev
 ```


- Install Graphics Drivers (Nvidia Howto: https://linuxconfig.org/how-to-install-the-nvidia-drivers-on-ubuntu-18-04-bionic-beaver-linux)
 ```
  $ sudo add-apt-repository ppa:graphics-drivers/ppa
  $ sudo apt update
  $ ubuntu-drivers devices
  $ sudo ubuntu-drivers autoinstall
 ```

- Install CUDA
 ```
  $ wget https://developer.download.nvidia.com/compute/cuda/repos/ubuntu1804/x86_64/cuda-ubuntu1804.pin
  $ sudo mv cuda-ubuntu1804.pin /etc/apt/preferences.d/cuda-repository-pin-600
  $ wget http://developer.download.nvidia.com/compute/cuda/10.1/Prod/local_installers/cuda-repo-ubuntu1804-10-1-local-10.1.243-418.87.00_1.0-1_amd64.deb
  $ sudo dpkg -i cuda-repo-ubuntu1804-10-1-local-10.1.243-418.87.00_1.0-1_amd64.deb
  $ sudo apt-key add /var/cuda-repo-10-1-local-10.1.243-418.87.00/7fa2af80.pub
  $ sudo apt-get update
  $ sudo apt-get -y install cuda
 ```

  carefully read the "UEFI" related dialogs and register the MOK in bios if asked
  make sure your installation works (glxinfo / nvidia-settings, cuda examples)
  maybe you need to blacklist some nvidia related modules (https://linuxconfig.org/how-to-disable-nouveau-nvidia-driver-on-ubuntu-18-04-bionic-beaver-linux)

Now restart the computer !!
Now Launch Software & Updates, goto Additional Drivers and select nvidia-driver-435 and apply
Restart again.

 ```
  $ cd /usr/local/cuda/samples/1_Utilities/deviceQuery
  $ sudo make
  $ ./deviceQuery
 ```
  

- Install developer Tools
 ```
  $ sudo apt-get install git cmake python3-pip python3-dev
 ```

- Install conan package manager
 ```
  $ sudo pip3 install --upgrade conan
  $ conan remote add bincrafters "https://api.bintray.com/conan/bincrafters/public-conan"
  $ conan remote add camposs "https://conan.campar.in.tum.de/api/conan/conan-camposs"
  $ conan remote add ubitrack "https://conan.campar.in.tum.de/api/conan/conan-ubitrack"
  $ conan remote add artekmed "https://conan.campar.in.tum.de/api/conan/conan-artekmed"
  $ conan remote add vendor "https://conan.campar.in.tum.de/api/conan/conan-vendor"
  $ conan profile new --detect default
 ```

  change c++ default for stdlib in ~/.conan/profiles/default:
 ```
  $ conan profile update settings.compiler.libcxx=libstdc++11 default
 ```

  maybe needs fix: only python3 executable is installed but python is missing
  should be corrected in python_dev-config package; temporary fix:
 ```
  $ sudo ln -s /usr/bin/python3 /usr/local/bin/python
 ```

- Install Docker + Nvidia Container Runtime

 ```
  $ sudo apt-get install apt-transport-https ca-certificates curl gnupg-agent software-properties-common
  $ curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo apt-key add -
  $ sudo apt-key fingerprint 0EBFCD88
  $ sudo add-apt-repository "deb [arch=amd64] https://download.docker.com/linux/ubuntu $(lsb_release -cs) stable"
  $ sudo apt-get update
  $ sudo apt-get install docker-ce docker-ce-cli containerd.io
    
  $ curl -s -L https://nvidia.github.io/nvidia-docker/gpgkey | sudo apt-key add -
  $ distribution=$(. /etc/os-release;echo $ID$VERSION_ID)
  $ curl -s -L https://nvidia.github.io/nvidia-docker/$distribution/nvidia-docker.list | sudo tee /etc/apt/sources.list.d/nvidia-docker.list
  $ sudo apt-get update
  $ sudo apt-get install nvidia-docker2
  $ sudo pkill -SIGHUP dockerd
  $ sudo docker run --runtime nvidia nvidia/cuda:10.1-base nvidia-smi
  $ sudo usermod -aG docker narvis
 ```
 Restart your computer or logout/login to activate the new user group (the above is assuming you are using username "narvis"

- Build Artekmed Prototype Docker Container:
 ```
  $ git clone https://github.com/TUM-CAMP-NARVIS/artekmed_prototyp_02.git
  $ cd artekmed_prototyp_02
  $ docker build -t artekmed_prototyp_02 .
 ```


Notes for CUDA Implementation:
------------------------------

Note for CLion users: Download the CUDARunPatcher Plugin(https://plugins.jetbrains.com/plugin/10691-clion-cuda-run-patcher), if you can not launch an application which uses CUDA.
If you cannot find it in the marketplace, it means your CLion version is too new for this plugin and you have to download it yourself and bump the version in the plugin.xml (yes, thanks JetBrains).

if you get an error because <pcl/..> can't be included, download and conan create the fixed repository:
https://github.com/ulricheck/conan-pcl/tree/fc8f2474cf9b37cf908aecc80dc036d64ced66d8



============================================


OLD PARTS - only needed if not using Docker


- Build an Ubitrack 1.3 release from scratch [optional]
 ```
  $ sudo pip3 install doit
  $ git clone https://github.com/ubitrack/ubitrack_release_tools.git
  $ cd ubitrack_release_tools
  $ cp custom_options_example.yml local_options.yml
 ```
  now adjust your configuration in local_options.yml
 ```
  $ doit custom_options=local_options.yml
 ```


- Build Artekmed Prototype (Example assumes CLion or cmd line):
 ```
  $ git clone https://github.com/TUM-CAMP-NARVIS/artekmed_prototyp_01.git
  $ cd artekmed_prototyp_01
  $ mkdir cmake-build-release && cd cmake-build-release
  $ conan install .. --build missing --build outdated -s build_type=Release
  $ cmake .. -DCMAKE_BUILD_TYPE=Release
  $ make -j 8
 ```

  Or launch Clion and use CMake / Build / Run to develop.

- Run Artekmed Protoype (assumes download of sample data and adjustments of paths in artekmed_zed_only_player.dfg)
 ```
  $ cd artekmed_prototyp_01/cmake-build-release
  $ source activate_run.sh
  $ ./bin/artekmed_p1 ../config/dfgs/artekmed_zed_only_player.dfg
 ```




Notes:
- NVIDIDA Docker Installation: https://github.com/NVIDIA/nvidia-docker
