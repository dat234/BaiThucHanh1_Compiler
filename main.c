#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// --- CẤU HÌNH ---
#define MAX_WORDS 6000      
#define MAX_LEN 50
#define MAX_LINES 2000      
#define MAX_STOPW 100

typedef struct {
    char word[MAX_LEN];
    int *lines;             
    int lineCount;
    int lineCapacity;       
} WordItem;

char stopWords[MAX_STOPW][MAX_LEN];
int stopWordCount = 0;

WordItem *dictionary;       
int dictCount = 0;

// Hàm chuyển chữ thường
void toLowerCase(char *str) {
    for(int i = 0; str[i]; i++){
        str[i] = tolower(str[i]);
    }
}

// Đọc stopw.txt
void loadStopWords(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) { printf("Loi: Khong tim thay file %s\n", filename); return; }
    char buffer[MAX_LEN];
    while (fscanf(f, "%s", buffer) != EOF) {
        toLowerCase(buffer);
        strcpy(stopWords[stopWordCount++], buffer);
    }
    fclose(f);
}

// Kiểm tra stopword
int isStopWord(char *word) {
    for (int i = 0; i < stopWordCount; i++) {
        if (strcmp(word, stopWords[i]) == 0) return 1;
    }
    return 0;
}

// Thêm từ vào từ điển
void addToDictionary(char *word, int lineNumber) {
    for (int i = 0; i < dictCount; i++) {
        if (strcmp(dictionary[i].word, word) == 0) {
            if (dictionary[i].lineCount > 0 && 
                dictionary[i].lines[dictionary[i].lineCount - 1] == lineNumber) {
                return;
            }
            if (dictionary[i].lineCount >= dictionary[i].lineCapacity) {
                if (dictionary[i].lineCapacity < MAX_LINES) { 
                     dictionary[i].lineCapacity += 100; 
                     dictionary[i].lines = realloc(dictionary[i].lines, dictionary[i].lineCapacity * sizeof(int));
                } else {
                    return; 
                }
            }
            dictionary[i].lines[dictionary[i].lineCount++] = lineNumber;
            return;
        }
    }

    if (dictCount < MAX_WORDS) {
        strcpy(dictionary[dictCount].word, word);
        dictionary[dictCount].lineCapacity = 50;
        dictionary[dictCount].lines = (int*)malloc(dictionary[dictCount].lineCapacity * sizeof(int));
        dictionary[dictCount].lines[0] = lineNumber;
        dictionary[dictCount].lineCount = 1;
        dictCount++;
    }
}

// Xử lý văn bản
void processText(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) { 
        printf("Loi: Khong the mo file '%s'. Kiem tra lai ten file!\n", filename); 
        exit(1); 
    }

    char ch, buffer[MAX_LEN];
    int len = 0;
    int currentLine = 1;
    int isStartOfSentence = 1;

    dictionary = (WordItem*)malloc(MAX_WORDS * sizeof(WordItem));

    while ((ch = fgetc(f)) != EOF) {
        if (isalpha(ch)) {
            if (len < MAX_LEN - 1) buffer[len++] = ch;
        } else {
            if (len > 0) {
                buffer[len] = '\0';
                if (!(isupper(buffer[0]) && !isStartOfSentence)) {
                    toLowerCase(buffer);
                    if (!isStopWord(buffer)) {
                        addToDictionary(buffer, currentLine);
                    }
                }
                len = 0;
                isStartOfSentence = 0;
            }
            if (ch == '.' || ch == '?' || ch == '!') isStartOfSentence = 1;
            if (ch == '\n') currentLine++;
        }
    }
    fclose(f);
}

int compareWords(const void *a, const void *b) {
    return strcmp(((WordItem*)a)->word, ((WordItem*)b)->word);
}

// --- HÀM MAIN MỚI ---
int main(int argc, char *argv[]) {
    // Kiểm tra xem người dùng có nhập tên file không
    if (argc < 2) {
        printf("Cach su dung:\n");
        printf("   Windows: .\\indexer.exe <ten_file_can_doc>\n");
        printf("   Vi du:   .\\indexer.exe alice30.txt\n");
        return 1;
    }

    loadStopWords("stopw.txt");
    
    // argv[1] chính là cái tên file bạn gõ sau lệnh ./indexer
    printf("Dang xu ly file: %s ...\n", argv[1]);
    processText(argv[1]);
    
    qsort(dictionary, dictCount, sizeof(WordItem), compareWords);

    printf("Ket qua:\n");
    printf("--------------------------------------\n");
    for (int i = 0; i < dictCount; i++) {
        printf("%-15s", dictionary[i].word);
        for (int j = 0; j < dictionary[i].lineCount; j++) {
            printf("%d", dictionary[i].lines[j]);
            if (j < dictionary[i].lineCount - 1) printf(", ");
        }
        printf("\n");
    }
    
    for(int i=0; i<dictCount; i++) free(dictionary[i].lines);
    free(dictionary);

    return 0;
}