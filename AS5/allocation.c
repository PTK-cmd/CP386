#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_MEMORY 1048576 // 默认内存大小 1 MB

typedef struct Block {
    int start;           // 起始地址
    int size;            // 内存块大小
    int is_free;         // 是否空闲，1 表示空闲，0 表示已分配
    char process[10];    // 进程名
    struct Block *next;  // 下一个内存块
} Block;

Block *head = NULL;

// 初始化内存
void init_memory(int max_memory) {
    head = (Block *)malloc(sizeof(Block));
    head->start = 0;
    head->size = max_memory;
    head->is_free = 1;
    strcpy(head->process, "");
    head->next = NULL;
    printf("Here, the Best Fit approach has been implemented and the allocated %d bytes of memory.\n", max_memory);
}

// 打印内存状态
void print_status() {
    Block *current = head;
    int allocated_memory = 0, free_memory = 0;

    printf("Partitions [Allocated memory = ");
    while (current) {
        if (!current->is_free) {
            allocated_memory += current->size;
        }
        current = current->next;
    }
    printf("%d]:\n", allocated_memory);

    current = head;
    while (current) {
        if (!current->is_free) {
            printf("Address [%d:%d] Process %s\n", current->start, current->start + current->size - 1, current->process);
        }
        current = current->next;
    }

    printf("Holes [Free memory = ");
    current = head;
    while (current) {
        if (current->is_free) {
            free_memory += current->size;
        }
        current = current->next;
    }
    printf("%d]:\n", free_memory);

    current = head;
    while (current) {
        if (current->is_free) {
            printf("Address [%d:%d] len = %d\n", current->start, current->start + current->size - 1, current->size);
        }
        current = current->next;
    }
}

// 分配内存
void allocate_memory(char *process, int size, char strategy) {
    Block *current = head, *best_fit = NULL;
    int best_size = MAX_MEMORY + 1;

    while (current) {
        if (current->is_free && current->size >= size) {
            if (strategy == 'F') { // First Fit
                best_fit = current;
                break;
            } else if (strategy == 'B') { // Best Fit
                if (current->size < best_size) {
                    best_fit = current;
                    best_size = current->size;
                }
            } else if (strategy == 'W') { // Worst Fit
                if (current->size > best_size) {
                    best_fit = current;
                    best_size = current->size;
                }
            }
        }
        current = current->next;
    }

    if (best_fit) {
        if (best_fit->size > size) {
            Block *new_block = (Block *)malloc(sizeof(Block));
            new_block->start = best_fit->start + size;
            new_block->size = best_fit->size - size;
            new_block->is_free = 1;
            strcpy(new_block->process, "");
            new_block->next = best_fit->next;
            best_fit->next = new_block;
        }
        best_fit->size = size;
        best_fit->is_free = 0;
        strcpy(best_fit->process, process);
        printf("Successfully allocated %d to process %s\n", size, process);
    } else {
        printf("No sufficient memory to allocate %d to process %s\n", size, process);
    }
}

// 释放内存
void release_memory(char *process) {
    Block *current = head;

    while (current) {
        if (!current->is_free && strcmp(current->process, process) == 0) {
            current->is_free = 1;
            strcpy(current->process, "");
            printf("Successfully released memory for process %s\n", process);

            // 合并下一个空闲块
            if (current->next && current->next->is_free) {
                Block *temp = current->next;
                current->size += temp->size;
                current->next = temp->next;
                free(temp);
            }

            // 合并上一个空闲块
            Block *prev = head;
            while (prev && prev->next != current) {
                prev = prev->next;
            }
            if (prev && prev->is_free) {
                prev->size += current->size;
                prev->next = current->next;
                free(current);
            }
            return;
        }
        current = current->next;
    }

    printf("No memory allocated to process %s\n", process);
}

// 内存压缩
void compact_memory() {
    Block *current = head, *prev_free = NULL;
    int new_start = 0;

    while (current) {
        if (!current->is_free) {
            if (current->start != new_start) {
                current->start = new_start;
            }
            new_start += current->size;
        } else if (prev_free) {
            prev_free->size += current->size;
            prev_free->next = current->next;
            free(current);
            current = prev_free;
        } else {
            prev_free = current;
        }
        current = current->next;
    }

    printf("Compaction process is successful\n");
}

// 主程序
int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: ./allocation <memory_size>\n");
        return 1;
    }

    int max_memory = atoi(argv[1]);
    init_memory(max_memory);

    char command[100], process[10], strategy;
    int size;

    while (1) {
        printf("allocator> ");
        scanf("%s", command);

        if (strcmp(command, "RQ") == 0) {
            scanf("%s %d %c", process, &size, &strategy);
            allocate_memory(process, size, strategy);
        } else if (strcmp(command, "RL") == 0) {
            scanf("%s", process);
            release_memory(process);
        } else if (strcmp(command, "C") == 0) {
            compact_memory();
        } else if (strcmp(command, "Status") == 0) {
            print_status();
        } else if (strcmp(command, "Exit") == 0) {
            printf("Exiting program.\n");
            break;
        } else {
            printf("Invalid command.\n");
        }
    }

    return 0;
}
