#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>
#include <sys/select.h>
#include <pthread.h>

#define MAX_BUF_SIZE 65536
#define TIMEOUT_SEC 30
#define GB_TIMEOUT_SEC 2
#define MAX_LINE_LENGTH 256



int fd;
char command[2] = {0};

int getCartType() {
  unsigned char CartType[9];
  char buf[MAX_BUF_SIZE];

  printf("Getting Cartridge Type...\n");
  strcpy(command, "6");
  write(fd, command, 1);
  int bytesRead = read(fd, buf, MAX_BUF_SIZE);
  printf("[DEBUG]read %d buffer\n", bytesRead);

  for (int i = 0; i < 8; i++) {
    unsigned char byte = (unsigned char)buf[i];
    CartType[i] = byte;
  }
  CartType[8] = '\0';
  memset(buf, 0, sizeof(buf));
  printf("[mcu] %s\n", CartType);

  if(memcmp(CartType, "GBA", 3) == 0) {
    return 1;
  }
  if(memcmp(CartType, "GBC", 3) == 0) {
    return 0;
  }
}

// GBA
int search(char codeID[5]) {
    FILE *file = fopen("gba.txt", "r");
    if (file == NULL) {
        perror("Error opening file. No file: gba.txt");
        return 1;
    }

    int gba_size = 0;
    char code_search[6];
    code_search[0] = codeID[0];
    code_search[1] = codeID[1];
    code_search[2] = codeID[2];
    code_search[3] = codeID[3];
    code_search[4] = ',';
    code_search[5] = '\0';
    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file)) {
      char *found = strstr(line, code_search);
      if (found != NULL) {
        // found points to the start of "B8KJ,"
        found += strlen(code_search); // move past "B8KJ,"
        if (strlen(found) >= 2) {
          gba_size = ((int)found[0] - 48) * 10 + ((int)found[1] - 48);
            printf("Cartridge Size: %d Mb\n", gba_size);
            break;
        } else {
          gba_size = (int)found[0] - 48;
            printf("Cartridge Size: %d Mb\n", gba_size);
            break;
        }
    }
  }
  fclose(file);
  return gba_size;
}

int getCodeID(char name[12], char codeid[4]) {
    FILE *file = fopen("romname.lst.txt", "r");
    if (file == NULL) {
        printf("Error opening file\n");
        return 1;
    }

    char line[256];
    int found = 0;
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(name, line, strlen(name)) == 0) {
            strncpy(codeid, &line[12], 4);
            found = 1;
            break;
        }
    }

    fclose(file);
    if (!found) {
      printf("Cannot find CODEID from Name\n");
      return 1;
    }
    return found;
}

int dumpGBA() {
  char buf[MAX_BUF_SIZE];
  int bufPos = 0; // buffer position
  char gameboyFile[MAX_BUF_SIZE];
  char gameboyRAM[MAX_BUF_SIZE];
  unsigned char header[18];

  printf("Getting Cartridge Information...\n\n");
  FILE *gamesListFile = fopen("GameList.txt", "a");
  if (!gamesListFile) {
      perror("fopen");
      close(fd);
      return 1;
  }

  printf("Reading header...\n");
  strcpy(command, "4");
  write(fd, command, 1);
  int bytesRead = read(fd, buf, 17);
  printf("[DEBUG]read %d buffer\n", bytesRead);

  for (int i = 0; i < 17; i++) {
    unsigned char byte = (unsigned char)buf[i];
    header[i] = byte;
  }
  header[17] = '\0';
  memset(buf, 0, sizeof(buf));



  printf("[DEBUG] header[17]: %s\n", header);
  if (memcmp(header, "ErrorCart", 9) == 0) {
    printf("Error GBA cartridge, check power voltage or fix this cartridge!\n");
    return 1;
  }
  else {
    char gba_name[13];
    char gba_code[5];

    strncpy(gba_name, header, 12);
    gba_name[12] = '\0';
    printf("GBA Game Name = %s\n", gba_name);
    gba_code[0] = header[13];
    gba_code[1] = header[14];
    gba_code[2] = header[15];
    gba_code[3] = header[16];
    gba_code[4] = '\0';


    printf("GBA Game Code = %s\n", gba_code);
    int cartSize = search(gba_code);
    if (cartSize == 0){
      getCodeID(gba_name, gba_code);
      printf("Found CodeID by Cart Name\nGBA Game Code = %s\nResearching Cartridge size!\n", gba_code);
      cartSize = search(gba_code);
    }

    fputs(header, gamesListFile);
    fputs("\n", gamesListFile);
    fclose(gamesListFile);
    strcpy(gameboyFile, header);
    strcat(gameboyFile, ".GBA");
    memset(header, 0, sizeof(header));

    FILE *gameFile = fopen(gameboyFile, "wb");
    if (!gameFile) {
        perror("fopen");
        close(fd);
        return 1;
    }

    printf("Sending size to mcu..\n");
    // 发送以0开头的字符串，mcu认0为标识
    // buf[1]代表容量，1 = 1M， 2 = 4M， 3 = 8M， 4 = 16M， 5 = 32M
    buf[0] = '0';
    if (cartSize == 1){
      buf[1] = '1';
    }
    else if(cartSize == 4) {
      buf[1] = '2';
    }
    else if(cartSize == 8) {
      buf[1] = '3';
    }
    else if(cartSize == 16) {
      buf[1] = '4';
    }
    else if(cartSize == 32) {
      buf[1] = '5';
    }
    /*
    else {
      printf("Cartridge size not supported, or it's not official cart.\n");
      return 1;
    }
    */
    else {
      printf("Cartridge size not supported, or it's not official cart.\n");
      printf("But we still read anyway.\n");
      cartSize = 32;
      buf[0] = '0';
      buf[1] = '5';
      memset(gameboyFile, 0, sizeof(gameboyFile));
      strcpy(gameboyFile, "ROM.GBA");
      fclose(gameFile);
      FILE *gameFile = fopen(gameboyFile, "wb");
      if (!gameFile) {
          perror("fopen");
          close(fd);
          return 1;
      }
     }
    printf("sent mcu %s\n", buf);
    write(fd, buf, strlen(buf));
    //sleep(2);
    // 检查mcu收到的字节是否正确
    char mcu_back[21];
    while(1) {
      int bytesRead = read(fd, buf, MAX_BUF_SIZE);
      if (bytesRead > 0) {
        for (int i = 0; i < 20; i++) {
             unsigned char byte = (unsigned char)buf[i];
             mcu_back[i] = byte;
        }
        mcu_back[20] = '\0';
        break;
      }
    }
    if(memcmp(mcu_back, "success", 7) != 0){
      printf("Setting mcu gba cartridge size error!\n");
      return 1;
    } else {
      printf("[MCU] %s\n", mcu_back);
        // gba_dump
      printf("\nDumping GBA ROM...\n");
      strcpy(command, "5");
      write(fd, command, strlen(command));
      int count = cartSize * 1024 * 1024;
      int count_t = 0;
      while(1) {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(fd, &read_fds);
        struct timeval timeout;
        timeout.tv_sec = TIMEOUT_SEC;
        timeout.tv_usec = 0;
        int result = select(fd + 1, &read_fds, NULL, NULL, &timeout);

        if (result == -1) {
            perror("select");
            fclose(gameFile);
            close(fd);
            return 1;
        } else if (result == 0) {
            //printf("ROM saved in %s.\n", gameboyFile);
            printf("count = %d, count_t = %d\n", count, count_t);
            printf("Broken Cartridge\n");
            printf("Broken Cartridge1\n");
            break;
        } else {
          int bytesRead = read(fd, buf, MAX_BUF_SIZE);
          if (bytesRead > 0) {
            bufPos += bytesRead;
            for (int i = 0; i < bytesRead; i++) {
              unsigned char byte = (unsigned char)buf[i];
              fwrite(&byte, 1, 1, gameFile);
              count_t++;
              //printf("%c\n", byte);
            }
            if (bufPos >= MAX_BUF_SIZE) {
              bufPos = 0;
              double getSize = (double)count_t / 1024 /1024;
              double persent = (double)count_t / count * 100;
              printf("\rProcessing: %.2lf %%, get %.2lf Mb", persent, getSize);
              fflush(stdout);
            }
            if(count_t >= count) { //dump到对应大小
              printf("\n[LOG]Received %d Bytes. %d Mb data write.\n", count_t, count_t/1024/1024);
              printf("ROM saved in %s. \n", gameboyFile, count, count_t);
              count_t = 0;
              break;
            }
         } else { //超时
                printf("count = %d, count_t = %d\n", count, count_t);
                printf("Broken Cartridge\n");
                fclose(gameFile);
                close(fd);
                return 1;
              }
            }
          }
          fclose(gameFile);
        }
      }
}
// END GBA DUMP

// GB/GBC DUMP
int dumpGB() {
  char buf[MAX_BUF_SIZE];
  int bufPos = 0; // buffer position
  char gameboyFile[MAX_BUF_SIZE];

  printf("Getting Name...\n");
  FILE *gamesListFile = fopen("GameList.txt", "a");
  if (!gamesListFile) {
      perror("fopen");
      close(fd);
      return 1;
  }
  strcpy(buf, "1\n");
  write(fd, buf, strlen(buf));

  fd_set read_fds;
  FD_ZERO(&read_fds);
  FD_SET(fd, &read_fds);

  struct timeval timeout;
  timeout.tv_sec = GB_TIMEOUT_SEC;
  timeout.tv_usec = 0;

  int result = select(fd + 1, &read_fds, NULL, NULL, &timeout);
  if (result == -1) {
      perror("select");
      close(fd);
      return 1;
  } else if (result == 0) {
      printf("Timeout occurred. No data available.\n");
      fclose(gamesListFile);
      close(fd);
      return 1;
  } else {
      int bytesRead = read(fd, buf, MAX_BUF_SIZE - 1);
      if (bytesRead > 0) {
          buf[bytesRead] = '\0';
          fputs(buf, gamesListFile);
          fputs("\n", gamesListFile);
      } else {
          perror("read");
          fclose(gamesListFile);
          close(fd);
          return 1;
      }
  }
  fclose(gamesListFile);

  strcpy(gameboyFile, buf);
  strcat(gameboyFile, ".GB");

  printf("Gameboy Cartridge Name = %s\n", buf);
  memset(buf, 0 , sizeof(buf));

  printf("Dumping ROM...\n");
  FILE *gameFile = fopen(gameboyFile, "wb");
  if (!gameFile) {
      perror("fopen");
      close(fd);
      return 1;
  }

  strcpy(buf, "2\n");
  write(fd, buf, strlen(buf));
  int count_t = 0;
  while (1) {
      FD_ZERO(&read_fds);
      FD_SET(fd, &read_fds);

      timeout.tv_sec = GB_TIMEOUT_SEC;
      timeout.tv_usec = 0;

      int result = select(fd + 1, &read_fds, NULL, NULL, &timeout);
      if (result == -1) {
          perror("select");
          fclose(gameFile);
          close(fd);
          return 1;
      } else if (result == 0) {
          printf("ROM saved in %s.\n", gameboyFile);
          break;
      } else {
          int bytesRead = read(fd, buf, MAX_BUF_SIZE);
          if (bytesRead > 0) {
            bufPos += bytesRead; // update buffer position
            for (int i = 0; i < bytesRead; i++) {
                  unsigned char byte = (unsigned char)buf[i];
                  fwrite(&byte, 1, 1, gameFile);
                  count_t++;
            }
            // check buffer position
          if(bufPos >= MAX_BUF_SIZE) {
              bufPos = 0; // reset buffer position
              double getSize = (double)count_t / 1024 /1024;
              printf("\r[Transmitting] Get %.2lf Mb", getSize);
              fflush(stdout);
            }
          } else {
              printf("ROM saved in %s.\n", gameboyFile);
              break;
          }
      }
  }

  fclose(gameFile);
}


int main() {
    fd = open("/dev/ttyACM0", O_RDWR);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    struct termios tty;
    memset(&tty, 0, sizeof(tty));
    if (tcgetattr(fd, &tty) != 0) {
        perror("tcgetattr");
        close(fd);
        return 1;
    }
    cfsetospeed(&tty, B115200);
    cfsetispeed(&tty, B115200);
    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;
    tty.c_cflag |= CREAD;
    tty.c_cflag |= CLOCAL;
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_lflag = 0;
    tty.c_cc[VMIN] = 1;
    tty.c_cc[VTIME] = 5;
    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        perror("tcsetattr");
        close(fd);
        return 1;
    }

    // get Cartridge Type
    int Mode = getCartType();
    printf("[DEBUG] Mode set %d\n", Mode);

    // Real dump
    if(Mode) {
      printf("\nDumping GBA...\n");
      dumpGBA();
    }
    if(!Mode) {
      printf("\nDumping GB/GBC...\n");
      dumpGB();
    }

  close(fd);
  return 0;
}
