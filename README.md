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
1：控制焦距变大
2：焦距变小
3：灯光电源
4：焦点前调
5：焦点后调
6：光圈扩大
7：光圈缩小
8：雨刷开关
9:关闭雨刷
w:云台上仰
s:云台下俯
a:云台左转
d:云台右转
f:截图(需要截图功能的，修改launch文件中的image_name为可执行路径即可)




