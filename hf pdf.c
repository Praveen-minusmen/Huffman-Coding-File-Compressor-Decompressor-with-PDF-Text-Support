#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_BUFFER_SIZE 1048576 // 1 MB

// Tree node structure for Huffman coding
typedef struct Tree {
    int frequency;
    unsigned char character;
    struct Tree *left;
    struct Tree *right;
} Tree;

// Priority queue structure for Huffman coding
typedef struct {
    Tree **data;
    int size;
    int capacity;
} PriorityQueue;

// Function prototypes
PriorityQueue* createPriorityQueue(int capacity);
void swap(Tree **a, Tree **b);
void heapifyUp(PriorityQueue *pq, int index);
void heapifyDown(PriorityQueue *pq, int index);
void insertPriorityQueue(PriorityQueue *pq, Tree *node);
Tree* extractMinPriorityQueue(PriorityQueue *pq);
Tree* buildHuffmanTree(int freqtable[][2], int size);
void toBinary(unsigned char a, char* output);
void traverseHuffmanTree(Tree *root, char *prev, char toAppend, char codes[256][256], int level);
unsigned char *readFileIntoBuffer(const char *path, int *sz);
void writeFileFromBuffer(const char *path, unsigned char *buffer, int sz, int flag);
void buildFrequencyTable(unsigned char *buffer, int sz, int freqtable[][2], int *uniqueCharCount);
void writeHeader(const char *path, char codes[256][256], int paddedBits);
void compressFile(const char *path, const char *output_path, char codes[256][256]);
unsigned char *readHeader(unsigned char *buffer, char codes[256][256], int *paddedBits, int *sz);
void decompressFile(const char *inputPath, const char *outputPath);
char *convertToText(const char *path, int *size);
void freeTree(Tree *root);

// Create a priority queue with a given capacity
PriorityQueue* createPriorityQueue(int capacity) {
    PriorityQueue *pq = (PriorityQueue *) malloc(sizeof(PriorityQueue));
    pq->data = (Tree **) malloc(capacity * sizeof(Tree *));
    pq->size = 0;
    pq->capacity = capacity;
    return pq;
}

// Swap two tree nodes
void swap(Tree **a, Tree **b) {
    Tree *temp = *a;
    *a = *b;
    *b = temp;
}

// Heapify up operation for priority queue
void heapifyUp(PriorityQueue *pq, int index) {
    while (index > 0 && pq->data[(index - 1) / 2]->frequency > pq->data[index]->frequency) {
        swap(&pq->data[(index - 1) / 2], &pq->data[index]);
        index = (index - 1) / 2;
    }
}

// Heapify down operation for priority queue
void heapifyDown(PriorityQueue *pq, int index) {
    int left = 2 * index + 1;
    int right = 2 * index + 2;
    int smallest = index;

    if (left < pq->size && pq->data[left]->frequency < pq->data[smallest]->frequency) {
        smallest = left;
    }
    if (right < pq->size && pq->data[right]->frequency < pq->data[smallest]->frequency) {
        smallest = right;
    }
    if (smallest != index) {
        swap(&pq->data[index], &pq->data[smallest]);
        heapifyDown(pq, smallest);
    }
}

// Insert a node into the priority queue
void insertPriorityQueue(PriorityQueue *pq, Tree *node) {
    if (pq->size == pq->capacity) {
        pq->capacity *= 2;
        pq->data = (Tree **) realloc(pq->data, pq->capacity * sizeof(Tree *));
    }
    pq->data[pq->size] = node;
    heapifyUp(pq, pq->size);
    pq->size++;
}

// Extract the minimum node from the priority queue
Tree* extractMinPriorityQueue(PriorityQueue *pq) {
    if (pq->size == 0) {
        return NULL;
    }
    Tree *minNode = pq->data[0];
    pq->data[0] = pq->data[pq->size - 1];
    pq->size--;
    heapifyDown(pq, 0);
    return minNode;
}

// Build the Huffman tree from the frequency table
Tree* buildHuffmanTree(int freqtable[][2], int size) {
    PriorityQueue *pq = createPriorityQueue(size);
    for (int i = 0; i < size; i++) {
        Tree *node = (Tree *) malloc(sizeof(Tree));
        node->frequency = freqtable[i][1];
        node->character = freqtable[i][0];
        node->left = node->right = NULL;
        insertPriorityQueue(pq, node);
    }
    while (pq->size > 1) {
        Tree *a = extractMinPriorityQueue(pq);
        Tree *b = extractMinPriorityQueue(pq);
        Tree *c = (Tree *) malloc(sizeof(Tree));
        c->frequency = a->frequency + b->frequency;
        c->left = a;
        c->right = b;
        insertPriorityQueue(pq, c);
    }
    Tree *root = extractMinPriorityQueue(pq);
    free(pq->data);
    free(pq);
    return root;
}

// Convert a character to its binary representation
void toBinary(unsigned char a, char* output) {
    for (int i = 7; i >= 0; i--) {
        output[7 - i] = ((a >> i) & 1) ? '1' : '0';
    }
    output[8] = '\0';
}

// Traverse the Huffman tree to generate Huffman codes
void traverseHuffmanTree(Tree *root, char *prev, char toAppend, char codes[256][256], int level) {
    if (root == NULL) return;

    prev[level] = toAppend;
    if (root->left == NULL && root->right == NULL) {
        prev[level + 1] = '\0';
        strcpy(codes[root->character], prev);
    } else {
        traverseHuffmanTree(root->left, prev, '0', codes, level + 1);
        traverseHuffmanTree(root->right, prev, '1', codes, level + 1);
    }
}

// Read entire file content into buffer and return its size
unsigned char *readFileIntoBuffer(const char *path, int *sz) {
    FILE *fp = fopen(path, "rb");
    if (fp == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
    fseek(fp, 0, SEEK_END);
    *sz = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    unsigned char *buffer = (unsigned char *) malloc(*sz);
    fread(buffer, 1, *sz, fp);
    fclose(fp);
    return buffer;
}

// Write buffer content to a file
void writeFileFromBuffer(const char *path, unsigned char *buffer, int sz, int flag) {
    FILE *fp = flag == 0 ? fopen(path, "wb") : fopen(path, "ab");
    if (fp == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
    fwrite(buffer, 1, sz, fp);
    fclose(fp);
}

// Build frequency table for characters in the input buffer
void buildFrequencyTable(unsigned char *buffer, int sz, int freqtable[][2], int *uniqueCharCount) {
    int freq[256] = {0};
    for (int i = 0; i < sz; i++) {
        freq[buffer[i]]++;
    }
    *uniqueCharCount = 0;
    for (int i = 0; i < 256; i++) {
        if (freq[i] > 0) {
            freqtable[*uniqueCharCount][0] = i;
            freqtable[*uniqueCharCount][1] = freq[i];
            (*uniqueCharCount)++;
        }
    }
}

// Write header containing Huffman codes and padded bits to the output file
void writeHeader(const char *path, char codes[256][256], int paddedBits) {
    int size = 0;
    for (int i = 0; i < 256; i++) {
        if (codes[i][0] != '\0') {
            size++;
        }
    }

    writeFileFromBuffer(path, (unsigned char *) &paddedBits, sizeof(int), 0);
    writeFileFromBuffer(path, (unsigned char *) &size, sizeof(int), 1);

    for (int i = 0; i < 256; i++) {
        if (codes[i][0] != '\0') {
            unsigned char key = i;
            int len = strlen(codes[i]);
            writeFileFromBuffer(path, &key, 1, 1);
            writeFileFromBuffer(path, (unsigned char *) &len, sizeof(int), 1);
            writeFileFromBuffer(path, (unsigned char *) codes[i], len, 1);
        }
    }
}

// Compress a file using Huffman coding
void compressFile(const char *path, const char *output_path, char codes[256][256]) {
    int sz = 0;
    int paddedBits = 0;
    int freqtable[256][2];
    int uniqueCharCount;
    unsigned char *buffer = readFileIntoBuffer(path, &sz);

    buildFrequencyTable(buffer, sz, freqtable, &uniqueCharCount);

    Tree *root = buildHuffmanTree(freqtable, uniqueCharCount);

    char prev[256] = "";
    traverseHuffmanTree(root, prev, '\0', codes, -1);

    char *outputString = (char *) malloc(sz * 8 + 1);
    outputString[0] = '\0';
    for (int i = 0; i < sz; i++) {
        strcat(outputString, codes[buffer[i]]);
    }

    int outputStringLength = strlen(outputString);
    if (outputStringLength % 8 != 0) {
        paddedBits = 8 - (outputStringLength % 8);
        for (int i = 0; i < paddedBits; i++) {
            strcat(outputString, "0");
        }
    }

    sz = outputStringLength + paddedBits;
    unsigned char *outputBuffer = (unsigned char *) malloc(sz / 8);
    for (int i = 0; i < sz / 8; i++) {
        char byteString[9];
        strncpy(byteString, &outputString[i * 8], 8);
        byteString[8] = '\0';
        outputBuffer[i] = strtol(byteString, NULL, 2);
    }

    writeHeader(output_path, codes, paddedBits);
    writeFileFromBuffer(output_path, outputBuffer, sz / 8, 1);

    free(buffer);
    free(outputString);
    free(outputBuffer);
    freeTree(root);
}

// Read header information from compressed file
unsigned char *readHeader(unsigned char *buffer, char codes[256][256], int *paddedBits, int *sz) {
    *paddedBits = *((int *) buffer);
    buffer += sizeof(int);
    *sz -= sizeof(int);

    int size = *((int *) buffer);
    buffer += sizeof(int);
    *sz -= sizeof(int);

    for (int i = 0; i < size; i++) {
        unsigned char key = buffer[0];
        buffer++;
        (*sz)--;

        int len = *((int *) buffer);
        buffer += sizeof(int);
        *sz -= sizeof(int);

        strncpy(codes[key], (char *) buffer, len);
        codes[key][len] = '\0';

        buffer += len;
        *sz -= len;
    }
    return buffer;
}

// Decompress a compressed file
void decompressFile(const char *inputPath, const char *outputPath) {
    int sz = 0;
    char codes[256][256] = {0};
    int paddedBits = 0;
    unsigned char *fileBuffer = readFileIntoBuffer(inputPath, &sz);
    fileBuffer = readHeader(fileBuffer, codes, &paddedBits, &sz);

    int originalSize = sz * 8 - paddedBits;

    char *fileBitString = (char *) malloc(originalSize + 1);
    fileBitString[0] = '\0';

    for (int i = 0; i < sz; i++) {
        char byteString[9];
        toBinary(fileBuffer[i], byteString);
        if (i == sz - 1) {
            byteString[8 - paddedBits] = '\0';
        }
        strcat(fileBitString, byteString);
    }

    unsigned char *outputBuffer = (unsigned char *) malloc(originalSize);
    int outputSize = 0;
    char temp[256] = "";

    for (int i = 0; i < strlen(fileBitString); i++) {
        strncat(temp, &fileBitString[i], 1);
        for (int j = 0; j < 256; j++) {
            if (strcmp(temp, codes[j]) == 0) {
                outputBuffer[outputSize++] = (unsigned char) j;
                temp[0] = '\0';
                break;
            }
        }
    }

    writeFileFromBuffer(outputPath, outputBuffer, outputSize, 0);

    free(fileBuffer);
    free(fileBitString);
    free(outputBuffer);
}

// Convert various file types (PDF, Word) to text
char *convertToText(const char *path, int *size) {
    char command[512];
    char *buffer = (char *) malloc(MAX_BUFFER_SIZE);

    // Determine file type
    const char *ext = strrchr(path, '.');
    if (!ext) {
        fprintf(stderr, "File extension not found\n");
        exit(EXIT_FAILURE);
    }

    // Convert based on file type
    if (strcmp(ext, ".pdf") == 0) {
        snprintf(command, sizeof(command), "pdftotext -q %s -", path);
    } else if (strcmp(ext, ".doc") == 0 || strcmp(ext, ".docx") == 0) {
        snprintf(command, sizeof(command), "libreoffice --headless --convert-to txt:Text --outdir /tmp %s && cat /tmp/%s.txt", path, strrchr(path, '/') + 1);
    } else {
        fprintf(stderr, "Unsupported file type: %s\n", ext);
        exit(EXIT_FAILURE);
    }

    // Execute command to convert file to text
    FILE *fp = popen(command, "r");
    if (!fp) {
        perror("popen failed");
        exit(EXIT_FAILURE);
    }

    // Read the output into buffer
    *size = fread(buffer, 1, MAX_BUFFER_SIZE, fp);
    buffer[*size] = '\0';
    pclose(fp);

    return buffer;
}

// Free memory allocated for Huffman tree
void freeTree(Tree *root) {
    if (root == NULL) return;
    freeTree(root->left);
    freeTree(root->right);
    free(root);
}

// Main function to interact with user and choose compression or decompression
int main() {
    int choice;
    char inputFile[256];

    printf("Choose an option:\n");
    printf("1. Compress and Decompress\n");
    printf("2. Compress Only\n");
    printf("3. Decompress Only\n");
    printf("Enter your choice (1/2/3): ");
    scanf("%d", &choice);

    if (choice == 1 || choice == 2) {
        printf("Enter the input file name: ");
        scanf("%255s", inputFile);
    } else if (choice == 3) {
        printf("Enter the compressed file name: ");
        scanf("%255s", inputFile);
    } else {
        printf("Invalid choice!\n");
        return 1;
    }

    char compressedFile[256];
    char decompressedFile[256];
    sprintf(compressedFile, "%s_compressed", inputFile);
    sprintf(decompressedFile, "%s_decompressed", inputFile);

    char codes[256][256] = {0};

    switch (choice) {
        case 1:
        case 2: {
            int textSize;
            char *text = convertToText(inputFile, &textSize);
            char textFile[256];
            sprintf(textFile, "%s.txt", inputFile);
            writeFileFromBuffer(textFile, (unsigned char *)text, textSize, 0);
            compressFile(textFile, compressedFile, codes);
            if (choice == 1) {
                decompressFile(compressedFile, decompressedFile);
                printf("Compression and decompression completed successfully!\n");
            } else {
                printf("Compression completed successfully!\n");
            }
            free(text);
            break;
        }
        case 3:
            decompressFile(inputFile, decompressedFile);
            printf("Decompression completed successfully!\n");
            break;
        default:
            printf("Invalid choice!\n");
            return 1;
    }

    return 0;
}
