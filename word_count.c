//
// Created by mooki on 11/21/21.
//

/* 单线程单词计数程序，对照组 */

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

/* 预设参数 */
#define MOD 10007  // hash值计算模数
#define WORDLENMAX 128  // 单词最大长度

/* 全局数据区 */
uint32_t fileNum = 0;
uint32_t wordNum = 0;

/* 单词链表 */
// 节点
typedef struct WordNodeTag{
    char *word;
    struct WordNodeTag *next;
    uint32_t num;
}WordNode;

// 新建节点
WordNode *newWordNode(char* word){
    WordNode *wordNodeP = malloc(sizeof(WordNode));
    wordNodeP->word = malloc(strlen(word) + 1);
    strcpy(wordNodeP->word, word);
    wordNodeP->next = NULL;
    wordNodeP->num = 1;

    return wordNodeP;
}

/* hash */
// hash表
WordNode *wordNodeHashTab[MOD];

// 初始化hash表
void initWordNodeHashTab(){
    for(uint32_t i = 0; i < MOD; ++i){
        wordNodeHashTab[i] = NULL;
    }
}

// hash值计算函数
uint32_t getHash(const char *s)
{
    uint32_t h = MOD ^ ((uint32_t)*s++ << 2);
    uint32_t len = 0;

    while (*s) {
        len++;
        h ^= (((uint32_t)*s) << (len % 3)) +
             ((uint32_t)*(s - 1) << ((len % 3 + 7)));
        s++;
    }
    h ^= len;

    return h % MOD;
}

/* 单词统计 */
// 计数单词
void countWord(char* word) {
    uint32_t hashVal = getHash(word);

    WordNode *wordNodeP = wordNodeHashTab[hashVal];
    if(wordNodeP != NULL){  // 之前出现过此hashVal
        for(; wordNodeP != NULL; wordNodeP = wordNodeP->next){
            if(strcmp(wordNodeP->word, word) == 0){  // 之前出现过此单词
                ++wordNodeP->num;

                return;
            }
        }

        // 之前未出现过此单词
        WordNode *newWordNodeP = newWordNode(word);

        // 将新单词加到链表头(单向链表加到链表尾较为困难)
        newWordNodeP->next = wordNodeHashTab[hashVal];
        wordNodeHashTab[hashVal] = newWordNodeP;
    }
    else{  // 之前未出现过此hashVal
        WordNode *newWordNodeP = newWordNode(word);

        // 将新单词加到哈希表中
        wordNodeHashTab[hashVal] = newWordNodeP;
    }
}

// 从文本文件中分离单词
void* getWords(void* filePath){
    int cha;

    char word[WORDLENMAX];
    char* wordP = NULL;

    FILE *fd;
    if((fd = fopen(filePath, "r")) == NULL){
        fprintf(stderr, "Open %s failed. \n", (char *)filePath);
    }

    while((cha = getc(fd)) != EOF){
        if(isalpha(cha)){
            if(wordP == NULL){  // 一个新的单词
                wordP = word;
            }
            *wordP++ = (char)tolower(cha);
            ++wordNum;
        }
        else if(wordP != NULL){  // 一个单词结束了
            *wordP = '\0';
            wordP = NULL;
            countWord(word);
        }
    }

    // 对文件最后一个单词空格、换行符、回车符的情况进行特殊处理
    if(wordP != NULL){
        *wordP = '\0';
        countWord(word);
    }

    return (void*)0;
}

/* 遍历目录项 */
void traverseAndCount(char* dirName, DIR* dir){
    struct dirent *dirEntP;
    while((dirEntP = readdir(dir)) != NULL){
        // 跳过"."和".."
        if((strcmp(dirEntP->d_name, ".") == 0) || (strcmp(dirEntP->d_name, "..") == 0)){
            continue;
        }

        // 得到新目录项路径
        char path[128] = {0};
        strcpy(path, dirName);
        strcat(path, "/");
        strcat(path, dirEntP->d_name);

        // 读取目录项
        struct stat statP;
        if(stat(path, &statP) != 0){
            fprintf(stderr, "Read stat struct of %s failed. \n", path);
            exit(1);
        }
        if(S_ISDIR(statP.st_mode)){  // 是目录，继续递归
            DIR* childDir;
            if((childDir = opendir(path)) == NULL){
                fprintf(stderr, "Opendir %s failed. \n", path);
                exit(1);
            }
            traverseAndCount(path, childDir);
        }
        else{  // 是文件，进行处理
            ++fileNum;

            getWords(path);
        }
    }
}

/* 输出结果 */
void printResult(){
    printf("%10s %-20s %-10s %-10s\n", "IDX", "WORD", "NUM", "PERCENT(%)");
    uint32_t idx = 0;

    WordNode *wordNodeP;
    for(uint32_t i = 0; i < MOD; ++i){
        for(wordNodeP = wordNodeHashTab[i]; wordNodeP != NULL ; wordNodeP = wordNodeP->next) {
            printf("%10u %-20s %-10u %-10.2f\n", idx, wordNodeP->word, wordNodeP->num,
                   (float)wordNodeP->num / (float)wordNum * 100);
            ++idx;
        }
    }
}

int main(int argc, char* argv[]){
    if(argc != 2){
        fprintf(stderr, "Usage: ./word_count_mt dirName\n");
        return 0;
    }

    // 初始化单词节点哈希表
    initWordNodeHashTab();

    // 遍历目录树并处理文本文件
    DIR* dir;
    if((dir = opendir(argv[1])) == NULL){
        if(errno == ENOTDIR){
            printf("%s is not a dir. \n", argv[1]);
        }
        return 0;
    }
    traverseAndCount(argv[1], dir);

    // 输出结果
    printResult();

    // 打印提示信息
    printf("Total %d files processed. \n", fileNum);

    return 0;
}