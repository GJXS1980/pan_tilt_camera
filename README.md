1..生成deb

运行：
```
bloom-generate rosdebian --os-name ubuntu --ros-distro melodic
```
找到Debian/rules，打开之后找到override_dh_shlibdeps，在dpkg-shlibdeps那一行最后加上如下选项：
```
--dpkg-shlibdeps-params=--ignore-missing-info
```
然后运行下面指令：
```
fakeroot debian/rules binary
```

# 安装
进入pan_tilt_camera/lib文件夹下，运行下面指令：
```
sudo cp *.so* /usr/lib/ && sudo cp -r HCNetSDKCom/ /usr/lib/

sudo apt-get install libopencv-dev
```

# 使用

| 按键 | 功能说明 | 备注 |
| :----: | :----: | :----: |
| 1 | 焦距变大 | 无 |
| 2 | 焦距变小 | 无 |
| 3 | 灯光电源 | 无 |
| 4 | 焦点前调 | 无 |
| 5 | 焦点后调 | 无 |
| 6 | 光圈扩大 | 无 |
| 7 | 光圈缩小 | 无 |
| 8 | 打开雨刷 | 无 |
| 9 | 关闭雨刷 | 无 |
| w | 云台上仰 | 无 |
| s | 云台下俯 | 无 |
| a | 云台左转 | 无 |
| d | 云台右转 | 无 |
| f | 截图 | 需要修改launch文件中的image_name为可执行路径 |

发布话题控制：
1.控制云台上仰
rostopic pub -r 100 /Camera_Control_TOPIC std_msgs/String "data: 'w'" 

2.控制云台下俯
rostopic pub -r 100 /Camera_Control_TOPIC std_msgs/String "data: 's'" 

3.控制云台左转
rostopic pub -r 100 /Camera_Control_TOPIC std_msgs/String "data: 'a'" 

4.控制云台右转
rostopic pub -r 100 /Camera_Control_TOPIC std_msgs/String "data: 'd'" 