#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>

#define PORT 8080
#define BUFFER_SIZE 4096

// 忽略SIGPIPE信号，防止写关闭的socket导致程序退出
void setup_signal_handler() {
    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGPIPE, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file-to-send>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *filename = argv[1];
    setup_signal_handler();

    // 创建TCP套接字
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    // 设置SO_REUSEADDR选项
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // 绑定套接字到端口
    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // 开始监听
    if (listen(server_fd, 5) < 0) {
        perror("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d. Waiting for connections...\n", PORT);

    while (1) {
        // 接受客户端连接
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_addr_len);
        
        if (client_fd < 0) {
            perror("accept failed");
            continue;
        }

        printf("Client connected: %s:%d\n", 
               inet_ntoa(client_addr.sin_addr), 
               ntohs(client_addr.sin_port));

        // 打开要发送的文件
        int file_fd = open(filename, O_RDONLY);
        if (file_fd < 0) {
            perror("file open failed");
            close(client_fd);
            continue;
        }

        // 获取文件大小
        struct stat file_stat;
        if (fstat(file_fd, &file_stat) < 0) {
            perror("fstat failed");
            close(file_fd);
            close(client_fd);
            continue;
        }

        off_t file_size = file_stat.st_size;
        off_t offset = 0;
        ssize_t sent_bytes;
        ssize_t total_sent = 0;

        printf("Sending file: %s (Size: %ld bytes)\n", filename, file_size);

        // 使用sendfile发送文件
        while (total_sent < file_size) {
            sent_bytes = sendfile(client_fd, file_fd, &offset, file_size - total_sent);
            
            if (sent_bytes <= 0) {
                if (errno == EAGAIN || errno == EINTR) {
                    continue; // 临时错误，重试
                }
                perror("sendfile error");
                break;
            }
            
            total_sent += sent_bytes;
            printf("Sent %zd bytes (%.2f%%)\n", 
                   sent_bytes, 
                   (double)total_sent / file_size * 100);
        }

        if (total_sent == file_size) {
            printf("File sent successfully! Total bytes: %zd\n", total_sent);
        } else {
            printf("File transfer incomplete. Sent %zd/%ld bytes\n", total_sent, file_size);
        }

        // 清理资源
        close(file_fd);
        close(client_fd);
        printf("Connection closed\n\n");
    }

    close(server_fd);
    return 0;
}



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <time.h>
#include <sys/stat.h>
#include <errno.h>

#define PORT 8080
#define BUFFER_SIZE 4096
#define MAX_FILENAME_LEN 256

// 显示传输进度
void show_progress(off_t received, off_t total, double elapsed_time) {
    double percentage = (double)received / total * 100;
    double speed = (received / 1024.0 / 1024.0) / elapsed_time; // MB/s
    
    printf("\rProgress: [");
    int pos = (int)(percentage / 2);
    for (int i = 0; i < 50; i++) {
        if (i < pos) printf("=");
        else if (i == pos) printf(">");
        else printf(" ");
    }
    
    printf("] %.2f%% | %.2f/%.2f MB | %.2f MB/s", 
           percentage, 
           (double)received / (1024 * 1024),
           (double)total / (1024 * 1024),
           speed);
    fflush(stdout);
}

// 接收文件头（文件名和文件大小）
int receive_file_header(int client_socket, char *filename, off_t *file_size) {
    // 接收文件名长度
    uint8_t filename_len;
    if (recv(client_socket, &filename_len, sizeof(filename_len), 0) != sizeof(filename_len)) {
        perror("Failed to receive filename length");
        return -1;
    }
    
    // 接收文件名
    if (recv(client_socket, filename, filename_len, 0) != filename_len) {
        perror("Failed to receive filename");
        return -1;
    }
    filename[filename_len] = '\0';
    
    // 接收文件大小
    if (recv(client_socket, file_size, sizeof(off_t), 0) != sizeof(off_t)) {
        perror("Failed to receive file size");
        return -1;
    }
    
    // 转换为主机字节序
    *file_size = be64toh(*file_size);
    
    return 0;
}

int main() {
    int server_fd, client_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    
    // 创建TCP套接字
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    
    // 配置服务器地址
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    // 绑定套接字到端口
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }
    
    // 开始监听
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
    
    printf("Server listening on port %d\n", PORT);
    printf("Waiting for client to send a file...\n");
    
    // 接受客户端连接
    if ((client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        perror("Accept failed");
        exit(EXIT_FAILURE);
    }
    
    printf("Client connected. Waiting for file information...\n");
    
    // 接收文件头信息
    char filename[MAX_FILENAME_LEN];
    off_t file_size;
    if (receive_file_header(client_socket, filename, &file_size) != 0) {
        close(client_socket);
        exit(EXIT_FAILURE);
    }
    
    printf("Receiving file: %s (Size: %.2f MB)\n", 
           filename, (double)file_size / (1024 * 1024));
    
    // 创建文件
    int file_fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (file_fd < 0) {
        perror("Failed to create file");
        close(client_socket);
        exit(EXIT_FAILURE);
    }
    
    // 开始接收文件
    char buffer[BUFFER_SIZE];
    off_t total_received = 0;
    ssize_t bytes_received;
    time_t start_time = time(NULL);
    
    printf("Starting file transfer...\n");
    
    while (total_received < file_size) {
        // 接收数据
        bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
        
        if (bytes_received < 0) {
            perror("Error receiving data");
            break;
        }
        
        if (bytes_received == 0) {
            printf("\nClient disconnected unexpectedly\n");
            break;
        }
        
        // 写入文件
        ssize_t bytes_written = write(file_fd, buffer, bytes_received);
        if (bytes_written < 0) {
            perror("Error writing to file");
            break;
        }
        
        total_received += bytes_received;
        
        // 显示进度
        time_t current_time = time(NULL);
        double elapsed = difftime(current_time, start_time);
        show_progress(total_received, file_size, elapsed);
    }
    
    // 完成传输
    close(file_fd);
    close(client_socket);
    
    printf("\n\nFile transfer completed with status: ");
    
    if (total_received == file_size) {
        printf("SUCCESS!\n");
        printf("Received %s (%.2f MB) in %.2f seconds\n", 
               filename, 
               (double)file_size / (1024 * 1024),
               difftime(time(NULL), start_time));
    } else {
        printf("INCOMPLETE!\n");
        printf("Received %zd/%zd bytes (%.2f%%)\n", 
               total_received, file_size,
               (double)total_received / file_size * 100);
        
        // 删除不完整的文件
        if (remove(filename) == 0) {
            printf("Deleted incomplete file: %s\n", filename);
        } else {
            perror("Failed to delete incomplete file");
        }
    }
    
    close(server_fd);
    return 0;
}