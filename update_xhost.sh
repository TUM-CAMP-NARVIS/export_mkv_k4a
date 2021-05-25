

# enable pcpd to run on headless system
# must enable autologon in /etc/gdm3/custom.conf
# for more info see: https://github.com/microsoft/Azure-Kinect-Sensor-SDK/issues/810
export XAUTHORITY=/run/user/$UID/gdm/Xauthority
export DISPLAY=:0
#xhost +si:localhost:root
xhost +
