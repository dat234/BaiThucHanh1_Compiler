#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// --- CẤU HÌNH ---
#define MAX_WORDS 6000      
#define MAX_LEN 50
#define MAX_LINES 2000      
#define MAX_STOPW 100

// Cấu trúc từ điển cập nhật
typedef struct {
    char word[MAX_LEN];
    int *lines;             
    int lineCount;          // Số lượng dòng ĐÃ LƯU (không tính trùng lặp dòng liên tiếp)
    int lineCapacity;       
    int totalCount;         // Tổng số lần từ xuất hiện (tính cả lặp lại trên cùng 1 dòng)
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
            // TỪ ĐÃ TỒN TẠI:
            dictionary[i].totalCount++; // Tăng tổng số lần xuất hiện lên
            
            // Logic lưu dòng: Chỉ lưu nếu dòng này khác dòng vừa lưu gần nhất (tránh 1, 1, 1)
            if (dictionary[i].lineCount > 0 && 
                dictionary[i].lines[dictionary[i].lineCount - 1] == lineNumber) {
                return; // Đã có dòng này rồi thì thôi không lưu số dòng nữa
            }

            // Mở rộng bộ nhớ nếu đầy
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

    // TỪ MỚI (CHƯA CÓ):
    if (dictCount < MAX_WORDS) {
        strcpy(dictionary[dictCount].word, word);
        dictionary[dictCount].totalCount = 1; // Khởi tạo số lần xuất hiện là 1
        
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

// --- HÀM MAIN ---
int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Cach su dung:\n");
        printf("   .\\indexer.exe <ten_file_can_doc>\n");
        return 1;
    }

    loadStopWords("stopw.txt");
    processText(argv[1]);
    qsort(dictionary, dictCount, sizeof(WordItem), compareWords);

    // IN KẾT QUẢ THEO ĐỊNH DẠNG MỚI
    // Mẫu: answer 7, 8, 12, 15
    // (Word) (Count), (Line), (Line)...
    
    printf("\nKET QUA:\n");
    for (int i = 0; i < dictCount; i++) {
        // In Từ và Tổng số lần xuất hiện trước
        printf("%-15s %d", dictionary[i].word, dictionary[i].totalCount);
        
        // In danh sách các dòng
        if (dictionary[i].lineCount > 0) {
            printf(", "); // Dấu phẩy ngăn cách giữa Count và Line đầu tiên
        }
        
        for (int j = 0; j < dictionary[i].lineCount; j++) {
            printf("%d", dictionary[i].lines[j]);
            if (j < dictionary[i].lineCount - 1) printf(", ");
        }
        printf("\n");
    }
    
    // Giải phóng bộ nhớ
    for(int i=0; i<dictCount; i++) free(dictionary[i].lines);
    free(dictionary);

    return 0;
}