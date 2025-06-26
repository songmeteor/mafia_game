#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "network.h"
#include "utils.h"

#define SERVER_IP "10.10.16.219" //라즈베리파이 서버 IP 주소
#define PORT      12345
#define BUF_SIZE  512

static int sockfd;

// 
void* recv_handler(void* arg) {
    char buf[BUF_SIZE];
    int len;
    while ((len = recv(sockfd, buf, BUF_SIZE-1, 0)) > 0) {
        buf[len] = '\0';
        // 안내/상태 메시지 강조
        if (strstr(buf, "[안내]") || strstr(buf, "밤이 되었습니다") || strstr(buf, "낮이 되었습니다") || strstr(buf, "투표") || strstr(buf, "행동")) {
            printf("\033[1;33m%s\033[0m", buf);
        } else {
            printf("%s", buf);
        }
        fflush(stdout);
    }
    printf("Disconnected from server.\n");
    exit(0);
    return NULL;
}

int main() {
    char buf[BUF_SIZE];
    sockfd = create_client_socket(SERVER_IP, PORT); // 클라이언트 소켓 생성
    if (sockfd < 0) return 1;
    int len = recv(sockfd, buf, BUF_SIZE-1, 0);  // 서버에서 " Enter your nickname: " send 할떄 까지 블록 상태 
    if (len <= 0) {
        fprintf(stderr, "Failed to receive nickname prompt.\n");
        close(sockfd);
        return 1;
    }
    buf[len] = '\0';
    printf("%s", buf);  // 서버로 부터 recv 한 Enter your nickname: 클라이언트에 출력
    if (fgets(buf, BUF_SIZE, stdin) == NULL)   // 클라이언트에서 닉네임 입력
        close(sockfd);
        return 0;
    }                                  
    buf[strcspn(buf, "\r\n")] = '\0';
    send(sockfd, buf, strlen(buf), 0);        //서버로 닉네임 send

    pthread_t tid;                             // 클라이언트 쓰레드 생성 (recv_handler 실행)
    if (pthread_create(&tid, NULL, recv_handler, NULL) != 0) {
        perror("pthread_create");
        close(sockfd);
        return 1;
    }
    pthread_detach(tid);

    // 입력 루프, while 안에서 서버로 send 하는 역할만 함
    while (fgets(buf, BUF_SIZE, stdin) != NULL) {
        if (send(sockfd, buf, strlen(buf), 0) < 0) {
            perror("send");
            break;
        }
    }
    close(sockfd);
    return 0;
}
