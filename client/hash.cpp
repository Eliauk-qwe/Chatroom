#include "client.h"

#include <unistd.h>
#include <fcntl.h>
#include <openssl/sha.h>
#include <string>
#include <vector>
#include <stdexcept>
#include <cstdio>

// 函数：计算文件描述符对应文件的SHA-256哈希值
// 参数：fd - 已打开的文件描述符
// 返回：文件的SHA-256哈希值（十六进制字符串）
std::string calculateHashFromDescriptor(int fd) {
    // 保存当前文件位置
    off_t original_pos = lseek(fd, 0, SEEK_CUR);
    if (original_pos == (off_t)-1) {
        throw std::runtime_error("无法获取当前文件位置");
    }

    // 重置到文件开头
    if (lseek(fd, 0, SEEK_SET) == (off_t)-1) {
        throw std::runtime_error("无法重置文件指针");
    }

    SHA256_CTX sha_context;
    if (!SHA256_Init(&sha_context)) {
        throw std::runtime_error("SHA256初始化失败");
    }

    const size_t buffer_size = 4096;
    std::vector<unsigned char> buffer(buffer_size);

    ssize_t bytes_read;
    while ((bytes_read = read(fd, buffer.data(), buffer_size)) > 0) {
        if (!SHA256_Update(&sha_context, buffer.data(), bytes_read)) {
            throw std::runtime_error("SHA256更新失败");
        }
    }

    // 检查读取是否出错
    if (bytes_read == -1) {
        throw std::runtime_error("文件读取错误");
    }

    // 计算最终哈希值
    unsigned char hash[SHA256_DIGEST_LENGTH];
    if (!SHA256_Final(hash, &sha_context)) {
        throw std::runtime_error("SHA256最终计算失败");
    }

    // 恢复原始文件位置
    if (lseek(fd, original_pos, SEEK_SET) == (off_t)-1) {
        throw std::runtime_error("无法恢复文件位置");
    }

    // 转换为十六进制字符串
    char hex_hash[2 * SHA256_DIGEST_LENGTH + 1];
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        sprintf(hex_hash + 2 * i, "%02x", hash[i]);
    }
    hex_hash[2 * SHA256_DIGEST_LENGTH] = '\0';

    return std::string(hex_hash);
}

// 使用示例
int main() {
    const char* filename = "test.txt";
    
    // 打开文件获取描述符
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("打开文件失败");
        return 1;
    }

    try {
        // 计算文件哈希
        std::string hash = calculateHashFromDescriptor(fd);
        printf("文件 '%s' 的SHA-256哈希值: %s\n", filename, hash.c_str());
        
        // 这里可以继续使用文件描述符进行其他操作...
        
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
    }

    // 关闭文件描述符
    close(fd);
    return 0;
}