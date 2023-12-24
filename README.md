




# ChatGLM_cpp_QT

## 介绍

该项目基于chatglm.cpp项目，项目地址如下
```
https://github.com/li-plus/chatglm.cpp
```

ChatGLM3项目地址如下
```
https://github.com/THUDM/ChatGLM3
```

感谢以上项目的开源分享


该项目只是将chatglm.cpp项目源码通过QT编译运行，祝大家玩的开心

## 使用方式

### 环境安装

该项目使用CPU运行
**需要有4G以上空闲的内存(注意不是显存)**

**该项目使用QT6.5.3版本**

将ChatGLM.cpp存储库克隆到本地计算机中：

```
git clone --recursive https://github.com/CsBoBoNice/chatglm_cpp_QT
```

如果在克隆存储库时忘记了--recursive标志，请在项目文件夹中运行以下命令：

```
git submodule update --init --recursive
```





### 获取模型文件

#### 通过chatglm.cpp项目生成模型文件

如果有必要可以根据该文档前面提到的两个项目地址进行模型的获取与生成

#### 通过我分享的百度网盘地址获取模型文件

分享地址:

```
还没上传
```

成功获取模型后将模型修改名称为ggml.bin放至代码运行路径，即exe文件夹

### 编译运行

重要的事情说三遍:

**注意编译时使用Release进行编译**

**注意编译时使用Release进行编译**

**注意编译时使用Release进行编译**






### QT Fot Android

### 切换分支

使用如下命令切换到android分支

```
git checkout android
```

### 编译
你需要提前将模型文件放至 /storage/emulated/0/chatglm 文件夹
```
/storage/emulated/0/chatglm/ggml.bin
```
或者你有别的想法也可以自行去代码中修改

还需要修改一处代码，文件路径：

```
/chatglm_cpp_QT/third_party/sentencepiece/third_party/protobuf-lite/common.cc
```


将__android_log_write函数调用注释
```
  // Output the log string the Android log at the appropriate level.
  // __android_log_write(android_log_level, "libprotobuf-native",
  //                     ostr.str().c_str());
  // Also output to std::cerr.
  fprintf(stderr, "%s", ostr.str().c_str());
  fflush(stderr);

  // Indicate termination if needed.
  if (android_log_level == ANDROID_LOG_FATAL) {
    // __android_log_write(ANDROID_LOG_FATAL, "libprotobuf-native",
    //                     "terminating.\n");
  }
```

这样就能正常编译通过了

#### 部署

当你尝试部署这个apk时会发现部署失败
你需要点击左边选项卡的"项目"
在安卓构建里找到"构建安卓 APK"
在"应用签名"里创建密钥
然后勾选"包签名"
选择你刚才创建的证书即可




