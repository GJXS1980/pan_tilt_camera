cmake_minimum_required(VERSION 3.0.2)
project(pan_tilt_camera)


add_compile_options(-std=c++11)


find_package(catkin REQUIRED COMPONENTS
  cv_bridge
  image_transport
  roscpp
  rospy
  sensor_msgs
  std_msgs
)


catkin_package(
   INCLUDE_DIRS include
#  LIBRARIES car_sensor
   CATKIN_DEPENDS cv_bridge image_transport roscpp sensor_msgs std_msgs rospy
#  DEPENDS system_lib
)

include_directories(
  include
  ${catkin_INCLUDE_DIRS}
)

link_directories(
  ${catkin_LIB_DIRS} lib
)

 
find_package(OpenCV)
add_executable(Pan_Tilt_Camera_NODE src/Alarm.cpp src/ConfigParams.cpp  src/main.cpp  src/tool.cpp
		src/CapPicture.cpp  src/GetStream.cpp   src/playback.cpp  src/Voice.cpp)

#target_link_libraries(Pan_Tilt_Camera_NODE ${catkin_LIBRARIES} -lpthread  hcnetsdk PlayCtrl  AudioRender SuperRender)

target_link_libraries(Pan_Tilt_Camera_NODE ${catkin_LIBRARIES} ${OpenCV_LIBRARIES} libhcnetsdk.so libPlayCtrl.so libAudioRender.so libSuperRender.so -lpthread)


install(TARGETS Pan_Tilt_Camera_NODE
  ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
  LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
  RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
)

install(
  DIRECTORY launch
  DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}
)

install(
  DIRECTORY sdkLog
  DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}
)

install(
  DIRECTORY record
  DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}
)

install(
  DIRECTORY lib
  DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}
)

install(
  DIRECTORY img
  DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}
)

