#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <cstring>

using namespace std;

void test_blocking_read() {
    cout << "=== 测试阻塞模式下的read行为 ===" << endl;
    
    // 创建一个无效的socket来模拟错误
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    
    // 确保是阻塞模式
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);
    
    char buffer[1024];
    ssize_t nread = read(fd, buffer, sizeof(buffer));
    
    cout << "read() 返回值: " << nread << endl;
    if (nread < 0) {
        cout << "errno: " << errno << " (" << strerror(errno) << ")" << endl;
        cout << "EWOULDBLOCK: " << EWOULDBLOCK << endl;
        cout << "EAGAIN: " << EAGAIN << endl;
        cout << "EINTR: " << EINTR << endl;
    }
    
    close(fd);
}

void test_readn_behavior() {
    cout << "\n=== 测试readn函数的行为 ===" << endl;
    
    // 模拟readn函数的逻辑
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    char buffer[1024];
    size_t left = 1024;
    size_t count = 0;
    char *buf = buffer;
    
    cout << "开始模拟readn循环..." << endl;
    
    while (left > 0) {
        ssize_t nread = read(fd, buf, left);
        cout << "read() 返回: " << nread << endl;
        
        if (nread == 0) {
            cout << "连接关闭" << endl;
            break;
        } else if (nread < 0) {
            cout << "read() 错误，errno: " << errno << endl;
            if (errno == EINTR || errno == EWOULDBLOCK) {
                cout << "继续循环..." << endl;
                continue;  // 这里可能导致无限循环
            } else {
                cout << "其他错误，退出" << endl;
                break;
            }
        }
        
        left -= nread;
        buf += nread;
        count += nread;
    }
    
    cout << "总共读取: " << count << " 字节" << endl;
    close(fd);
}

int main() {
    test_blocking_read();
    test_readn_behavior();
    return 0;
}
