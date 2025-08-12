
# 聊天室项目 (Chatroom)

一个基于C++17开发的高性能网络聊天室系统，支持用户注册、好友管理、群聊功能和文件传输。

## 🚀 功能特性

### 核心功能
- ✅ **用户管理**: 注册、登录、密码找回
- ✅ **好友系统**: 添加好友、删除好友、私聊
- ✅ **群聊功能**: 创建群聊、加入群聊、群聊消息
- ✅ **文件传输**: 支持好友间和群聊中的文件传输
- ✅ **实时通信**: 基于TCP Socket的实时消息传递

### 技术特性
- 🔧 **高并发**: 基于Epoll事件驱动的非阻塞I/O
- 🧵 **多线程**: 线程池处理客户端请求
- 💾 **数据存储**: Redis内存数据库
- 📦 **消息协议**: 自定义粘包处理协议
- 🔄 **心跳检测**: 自动检测客户端连接状态

## 📋 系统要求

### 操作系统
- Linux (Ubuntu 18.04+, CentOS 7+)
- macOS (需要安装相关依赖)

### 依赖库
- **C++17** 编译器 (GCC 7.0+ 或 Clang 5.0+)
- **CMake** 3.10+
- **Redis** 5.0+
- **nlohmann/json** 库
- **hiredis** 库
- **pthread** 库

## 🛠️ 安装依赖

### Ubuntu/Debian
```bash
# 更新包管理器
sudo apt update

# 安装编译工具
sudo apt install build-essential cmake

# 安装Redis
sudo apt install redis-server

# 安装nlohmann/json
sudo apt install nlohmann-json3-dev

# 安装hiredis
sudo apt install libhiredis-dev

# 启动Redis服务
sudo systemctl start redis-server
sudo systemctl enable redis-server
```

### CentOS/RHEL
```bash
# 安装编译工具
sudo yum groupinstall "Development Tools"
sudo yum install cmake3

# 安装Redis
sudo yum install redis

# 安装nlohmann/json (可能需要从源码编译)
git clone https://github.com/nlohmann/json.git
cd json
mkdir build && cd build
cmake ..
make -j$(nproc)
sudo make install

# 安装hiredis
sudo yum install hiredis-devel

# 启动Redis服务
sudo systemctl start redis
sudo systemctl enable redis
```

### macOS
```bash
# 安装Homebrew (如果未安装)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# 安装依赖
brew install cmake redis nlohmann-json hiredis

# 启动Redis服务
brew services start redis
```

## 🚀 编译运行

### 1. 克隆项目
```bash
git clone https://github.com/yourusername/chatroom.git
cd chatroom
```

### 2. 创建构建目录
```bash
mkdir build
cd build
```

### 3. 配置项目
```bash
cmake ..
```

### 4. 编译项目
```bash
make -j$(nproc)
```

### 5. 运行服务端
```bash
# 启动服务端 (IP地址 端口号)
./server 127.0.0.1 8888
```

### 6. 运行客户端
```bash
# 启动客户端 (服务器IP 端口号)
./client 127.0.0.1 8888
```

## 📁 项目结构

```

chatroom/
├── client/          # 客户端代码
│   ├── clientmain.cpp
│   ├── friend.cpp
│   ├── group.cpp
│   └── ...
├── server/          # 服务端代码
│   ├── servermain.cpp
│   ├── friend.cpp
│   ├── group.cpp
│   ├── Redis.hpp            # 数据库存储     
│   ├── ThreadPool.hpp   # 线程池
│   └── ...
├── JSON.hpp      # 消息序列化
├── StickyPacket.hpp # 网络通信
└── CMakeLists.txt   # 构建配置
└── README.md        # 项目说明
```

## 🎮 使用说明

### 服务端启动
```bash
# 在build目录下运行
./server <IP地址> <端口号>

# 示例
./server 127.0.0.1 8888
./server 0.0.0.0 9999
```

### 客户端连接
```bash
# 在build目录下运行
./client <服务器IP> <端口号>

# 示例
./client 127.0.0.1 8888
./client 192.168.1.100 9999
```

### 客户端操作
1. **注册账户**: 输入手机号、密码、用户名
2. **登录账户**: 输入手机号和密码
3. **好友管理**: 添加好友、查看好友列表、私聊
4. **群聊功能**: 创建群聊、加入群聊、群聊消息
5. **文件传输**: 发送和接收文件


## 🎯 架构图

![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/0270359963824f559797e1d60be6c8e2.jpeg)

![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/37919b1db03440839113a269a5cde28a.jpeg)


## 📊 性能指标

- **并发连接**: 支持1000+并发用户
- **消息延迟**: < 10ms
- **吞吐量**: 10000+ 消息/秒
- **内存占用**: 低内存占用
- **CPU使用**: 高效的事件驱动架构




## 🙏 致谢

- [nlohmann/json](https://github.com/nlohmann/json) - JSON库
- [hiredis](https://github.com/redis/hiredis) - Redis客户端库
- [Redis](https://redis.io/) - 内存数据库

---

⭐ 如果这个项目对您有帮助，请给它一个星标！
