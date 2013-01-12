
仅支持手工加载此检测功能。
1.用adb shell连接手机，并使用insmod命令加载rsm.ko模块，加载完后，rsm默认会对所有进程进行监控，默认监控时间间隔为120s。
2.插入模块后在/proc/rsm/目录下生成pname和timeout两个文件，使用下面的命令进行参数的设置：
echo xx > pname 设置不需要监控的进程名称
echo xx > timeout 设置监控的间隔时间
最多可以排除监控的进程数为16个。
例如不需要监控rpcrotuer_smd_x，krmt_storagecln，krmt_storagecln，krtcclntd，krtcclntcbd，kbatteryclntd，kbatteryclntcbd等进程（多个进程间用逗号或者空格隔开），监控时间间隔为60s：
echo rpcrotuer_smd_x，krmt_storagecln，krmt_storagecln，krtcclntd，krtcclntcbd，kbatteryclntd，kbatteryclntcbd > pname
echo 60 > timeout

