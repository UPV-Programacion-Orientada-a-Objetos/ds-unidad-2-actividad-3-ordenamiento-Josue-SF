#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>

class DataSource {
public:
    virtual int getNext() = 0;
    virtual bool hasMoreData() = 0;
    virtual ~DataSource() {}
};

class FileSource : public DataSource {
private:
    FILE* f;
    bool hasMore;
    int current;
    char buf[64];
public:
    FileSource(const char* filename) {
        f = fopen(filename, "r");
        if (!f) {
            hasMore = false;
            current = 0;
            return;
        }
        if (fgets(buf, sizeof(buf), f) != NULL) {
            char* endptr;
            long v = strtol(buf, &endptr, 10);
            if (endptr == buf) {
                hasMore = false;
            } else {
                current = (int)v;
                hasMore = true;
            }
        } else {
            hasMore = false;
        }
    }
    virtual int getNext() {
        int ret = current;
        if (!f) return ret;
        if (fgets(buf, sizeof(buf), f) != NULL) {
            char* endptr;
            long v = strtol(buf, &endptr, 10);
            if (endptr == buf) {
                hasMore = false;
            } else {
                current = (int)v;
                hasMore = true;
            }
        } else {
            hasMore = false;
        }
        return ret;
    }
    virtual bool hasMoreData() {
        return hasMore;
    }
    virtual ~FileSource() {
        if (f) fclose(f);
    }
};

class SerialSource : public DataSource {
private:
    FILE* f;
    bool hasMore;
    int current;
    char buf[64];
public:
    SerialSource(const char* portOrFile, bool simulateFile) {
        if (simulateFile) {
            f = fopen(portOrFile, "r");
        } else {
            f = fopen(portOrFile, "r");
        }
        if (!f) {
            hasMore = false;
            current = 0;
            return;
        }
        if (fgets(buf, sizeof(buf), f) != NULL) {
            char* endptr;
            long v = strtol(buf, &endptr, 10);
            if (endptr == buf) {
                hasMore = false;
            } else {
                current = (int)v;
                hasMore = true;
            }
        } else {
            hasMore = false;
        }
    }
    virtual int getNext() {
        int ret = current;
        if (!f) return ret;
        if (fgets(buf, sizeof(buf), f) != NULL) {
            char* endptr;
            long v = strtol(buf, &endptr, 10);
            if (endptr == buf) {
                hasMore = false;
            } else {
                current = (int)v;
                hasMore = true;
            }
        } else {
            hasMore = false;
        }
        return ret;
    }
    virtual bool hasMoreData() {
        return hasMore;
    }
    virtual ~SerialSource() {
        if (f) fclose(f);
    }
};

struct Node {
    int value;
    Node* prev;
    Node* next;
};

class CircularBuffer {
private:
    Node* head;
    size_t capacity;
    size_t count;
public:
    CircularBuffer(size_t cap) {
        head = NULL;
        capacity = cap;
        count = 0;
    }
    bool push(int v) {
        if (count >= capacity) return false;
        Node* n = new Node();
        n->value = v;
        if (!head) {
            n->next = n;
            n->prev = n;
            head = n;
        } else {
            Node* tail = head->prev;
            tail->next = n;
            n->prev = tail;
            n->next = head;
            head->prev = n;
        }
        count++;
        return true;
    }
    bool isFull() { return count >= capacity; }
    bool isEmpty() { return count == 0; }
    size_t size() { return count; }
    Node* begin() { return head; }
    void clear() {
        if (!head) return;
        Node* cur = head;
        size_t removed = 0;
        while (removed < count) {
            Node* next = cur->next;
            delete cur;
            cur = next;
            removed++;
        }
        head = NULL;
        count = 0;
    }
    void setHead(Node* h, size_t cnt) {
        head = h;
        count = cnt;
    }
    ~CircularBuffer() {
        clear();
    }
};

static Node* splitList(Node* head) {
    if (!head || head->next == head) return NULL;
    Node* slow = head;
    Node* fast = head;
    while (fast->next != head && fast->next->next != head) {
        slow = slow->next;
        fast = fast->next->next;
    }
    Node* second = slow->next;
    slow->next = head;
    head->prev = slow;
    Node* tail2 = second->prev;
    tail2->next = second;
    second->prev = tail2;
    return second;
}

static Node* mergeSorted(Node* a, Node* b) {
    if (!a) return b;
    if (!b) return a;
    Node dummy;
    Node* tail = &dummy;
    Node* pa = a;
    Node* pb = b;
    while (pa && pb) {
        if (pa->value <= pb->value) {
            tail->next = pa;
            pa->prev = tail;
            pa = pa->next;
        } else {
            tail->next = pb;
            pb->prev = tail;
            pb = pb->next;
        }
        tail = tail->next;
        tail->next = NULL;
    }
    if (pa) {
        tail->next = pa;
        pa->prev = tail;
    }
    if (pb) {
        tail->next = pb;
        pb->prev = tail;
    }
    Node* res = dummy.next;
    if (!res) return NULL;
    Node* cur = res;
    Node* last = NULL;
    while (cur) {
        last = cur;
        cur = cur->next;
    }
    res->prev = last;
    last->next = res;
    return res;
}

static Node* listToLinear(Node* head, size_t count) {
    if (!head) return NULL;
    Node* cur = head;
    size_t i = 0;
    Node* tail = head->prev;
    tail->next = NULL;
    head->prev = NULL;
    return head;
}

static Node* linearToCircular(Node* head) {
    if (!head) return NULL;
    Node* cur = head;
    Node* last = NULL;
    while (cur) {
        last = cur;
        cur = cur->next;
    }
    head->prev = last;
    last->next = head;
    return head;
}

static Node* mergeSortLinear(Node* head) {
    if (!head || !head->next) return head;
    Node* slow = head;
    Node* fast = head->next;
    while (fast && fast->next) {
        slow = slow->next;
        fast = fast->next->next;
    }
    Node* mid = slow->next;
    slow->next = NULL;
    if (mid) mid->prev = NULL;
    Node* left = mergeSortLinear(head);
    Node* right = mergeSortLinear(mid);
    Node dummy;
    Node* tail = &dummy;
    while (left && right) {
        if (left->value <= right->value) {
            tail->next = left;
            left->prev = tail;
            left = left->next;
        } else {
            tail->next = right;
            right->prev = tail;
            right = right->next;
        }
        tail = tail->next;
        tail->next = NULL;
    }
    if (left) tail->next = left;
    if (right) tail->next = right;
    Node* res = dummy.next;
    if (res) res->prev = NULL;
    Node* cur = res;
    Node* last = NULL;
    while (cur) { last = cur; cur = cur->next; }
    if (res && last) {
        res->prev = last;
        last->next = res;
    }
    return res;
}

static Node* circularToLinear(Node* head) {
    if (!head) return NULL;
    Node* tail = head->prev;
    tail->next = NULL;
    head->prev = NULL;
    return head;
}

class HeapEntry { public: int value; int src; };

class MinHeap {
private:
    HeapEntry* arr;
    int capacity;
    int sizeh;
public:
    MinHeap(int cap) {
        capacity = cap + 5;
        arr = (HeapEntry*)malloc(sizeof(HeapEntry) * (capacity + 1));
        sizeh = 0;
    }
    void push(int value, int src) {
        if (sizeh + 1 > capacity) return;
        sizeh++;
        int i = sizeh;
        arr[i].value = value;
        arr[i].src = src;
        while (i > 1) {
            int p = i / 2;
            if (arr[p].value <= arr[i].value) break;
            HeapEntry tmp = arr[p]; arr[p] = arr[i]; arr[i] = tmp;
            i = p;
        }
    }
    bool empty() { return sizeh == 0; }
    HeapEntry popMin() {
        HeapEntry res = arr[1];
        arr[1] = arr[sizeh];
        sizeh--;
        int i = 1;
        while (true) {
            int l = i * 2;
            int r = l + 1;
            int smallest = i;
            if (l <= sizeh && arr[l].value < arr[smallest].value) smallest = l;
            if (r <= sizeh && arr[r].value < arr[smallest].value) smallest = r;
            if (smallest == i) break;
            HeapEntry tmp = arr[i]; arr[i] = arr[smallest]; arr[smallest] = tmp;
            i = smallest;
        }
        return res;
    }
    ~MinHeap() { free(arr); }
};

static void writeChunk(Node* head, size_t count, int chunkIdx) {
    char fname[64];
    snprintf(fname, sizeof(fname), "chunk_%d.tmp", chunkIdx);
    FILE* f = fopen(fname, "w");
    if (!f) return;
    if (head) {
        Node* cur = head;
        size_t written = 0;
        do {
            fprintf(f, "%d\n", cur->value);
            cur = cur->next;
            written++;
        } while (cur != head && written < count);
    }
    fclose(f);
}

int main(int argc, char** argv) {
    const char* inputFile = "serial_input.txt";
    size_t bufferSize = 1000;
    if (argc >= 2) inputFile = argv[1];
    if (argc >= 3) bufferSize = (size_t)atoi(argv[2]);
    SerialSource serial(inputFile, true);
    CircularBuffer buffer(bufferSize);
    int chunkCount = 0;
    while (serial.hasMoreData()) {
        int v = serial.getNext();
        if (!buffer.push(v)) {
            Node* linear = circularToLinear(buffer.begin());
            Node* sorted = mergeSortLinear(linear);
            sorted = linearToCircular(sorted);
            writeChunk(sorted, buffer.size(), chunkCount);
            buffer.clear();
            chunkCount++;
            buffer.push(v);
        }
    }
    if (!buffer.isEmpty()) {
        Node* linear = circularToLinear(buffer.begin());
        Node* sorted = mergeSortLinear(linear);
        sorted = linearToCircular(sorted);
        writeChunk(sorted, buffer.size(), chunkCount);
        buffer.clear();
        chunkCount++;
    }
    if (chunkCount == 0) return 0;
    FileSource** sources = (FileSource**)malloc(sizeof(FileSource*) * chunkCount);
    for (int i = 0; i < chunkCount; ++i) {
        char fname[64];
        snprintf(fname, sizeof(fname), "chunk_%d.tmp", i);
        sources[i] = new FileSource(fname);
    }
    MinHeap heap(chunkCount);
    bool* active = (bool*)malloc(sizeof(bool) * chunkCount);
    for (int i = 0; i < chunkCount; ++i) {
        active[i] = false;
        if (sources[i]->hasMoreData()) {
            int val = sources[i]->getNext();
            heap.push(val, i);
            active[i] = true;
        }
    }
    FILE* out = fopen("output.sorted.txt", "w");
    if (!out) return 0;
    while (!heap.empty()) {
        HeapEntry e = heap.popMin();
        fprintf(out, "%d\n", e.value);
        int si = e.src;
        if (sources[si]->hasMoreData()) {
            int nv = sources[si]->getNext();
            heap.push(nv, si);
        }
    }
    fclose(out);
    for (int i = 0; i < chunkCount; ++i) delete sources[i];
    free(sources);
    free(active);
    return 0;
}
