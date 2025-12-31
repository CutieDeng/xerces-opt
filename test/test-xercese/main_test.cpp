#include <iostream>
#include <cstring>
#include "xercese/util/PlatformUtils.hh"
#include "xercese/util/ValueVectorOf.hh"
#include "xercese/util/OutOfMemoryException.hh"

using namespace xercese;
using namespace std;

// 定义一个简单的测试对象，使用POD类型避免memset警告
struct TestObject {
    int value;
    int id;
    double score;

    // 使用聚合初始化，保持trivially copyable
    static TestObject create(int v, int i = 0, double s = 0.0) {
        TestObject obj;
        obj.value = v;
        obj.id = i;
        obj.score = s;
        return obj;
    }

    // 比较操作符
    bool operator==(const TestObject& other) const {
        return value == other.value && id == other.id && score == other.score;
    }

    bool operator!=(const TestObject& other) const {
        return !(*this == other);
    }

    // 用于调试和验证的方法
    void display() const {
        std::cout << "TestObject(value=" << value << ", id=" << id << ", score=" << score << ")";
    }
};

// 用于测试 fCallDestructor=true 的计数器类
struct DestructorCounter {
    static int destructor_count;
    int id;

    DestructorCounter() : id(0) {}
    DestructorCounter(int i) : id(i) {}
    DestructorCounter(const DestructorCounter& other) : id(other.id) {}
    DestructorCounter& operator=(const DestructorCounter& other) {
        id = other.id;
        return *this;
    }
    ~DestructorCounter() {
        destructor_count++;
    }

    bool operator==(const DestructorCounter& other) const {
        return id == other.id;
    }
};
int DestructorCounter::destructor_count = 0;

// ============================================================================
// 测试场景1：类与数组类之间的交互
// VectorProcessor - 一个处理 ValueVectorOf 的辅助类
// ============================================================================
class VectorProcessor {
private:
    MemoryManager* fMemoryManager;
    int* fProcessedResults;          // 自己管理的数组
    unsigned int fResultCount;
    unsigned int fResultCapacity;

public:
    VectorProcessor(MemoryManager* mgr, unsigned int initialCapacity = 16)
        : fMemoryManager(mgr)
        , fProcessedResults(nullptr)
        , fResultCount(0)
        , fResultCapacity(initialCapacity)
    {
        fProcessedResults = (int*) fMemoryManager->allocate(fResultCapacity * sizeof(int));
        memset(fProcessedResults, 0, fResultCapacity * sizeof(int));
    }

    ~VectorProcessor() {
        if (fProcessedResults) {
            fMemoryManager->deallocate(fProcessedResults);
        }
    }

    // 处理向量中的所有元素，计算并存储结果
    void processVector(const ValueVectorOf<int>& vec) {
        for (unsigned int i = 0; i < vec.size(); i++) {
            int processed = vec.elementAt(i) * 2 + 1;  // 简单变换
            addResult(processed);
        }
    }

    // 将两个向量合并处理
    void mergeAndProcess(const ValueVectorOf<int>& vec1, const ValueVectorOf<int>& vec2) {
        for (unsigned int i = 0; i < vec1.size(); i++) {
            addResult(vec1.elementAt(i));
        }
        for (unsigned int i = 0; i < vec2.size(); i++) {
            addResult(vec2.elementAt(i));
        }
    }

    // 将结果复制到目标向量
    void copyResultsTo(ValueVectorOf<int>& target) const {
        for (unsigned int i = 0; i < fResultCount; i++) {
            target.addElement(fProcessedResults[i]);
        }
    }

    // 根据过滤条件提取元素
    void filterVector(const ValueVectorOf<int>& src, ValueVectorOf<int>& dest, int threshold) {
        for (unsigned int i = 0; i < src.size(); i++) {
            if (src.elementAt(i) > threshold) {
                dest.addElement(src.elementAt(i));
            }
        }
    }

    unsigned int getResultCount() const { return fResultCount; }
    int getResult(unsigned int index) const {
        if (index < fResultCount) return fProcessedResults[index];
        return -1;
    }

private:
    void addResult(int value) {
        if (fResultCount >= fResultCapacity) {
            // 扩容
            unsigned int newCapacity = fResultCapacity + (fResultCapacity / 4) + 1;
            int* newResults = (int*) fMemoryManager->allocate(newCapacity * sizeof(int));
            memcpy(newResults, fProcessedResults, fResultCount * sizeof(int));
            fMemoryManager->deallocate(fProcessedResults);
            fProcessedResults = newResults;
            fResultCapacity = newCapacity;
        }
        fProcessedResults[fResultCount++] = value;
    }
};

// ============================================================================
// 测试场景2：封装数组类为字段的上层应用类型
// StringTable - 一个使用 ValueVectorOf 作为内部存储的字符串表
// ============================================================================
class StringTable {
private:
    ValueVectorOf<char*>* fStrings;     // 字符串指针数组
    ValueVectorOf<unsigned int>* fLengths;  // 对应的字符串长度
    MemoryManager* fMemoryManager;
    unsigned int fTotalChars;           // 总字符数统计

public:
    StringTable(MemoryManager* mgr, unsigned int initialCapacity = 16)
        : fStrings(nullptr)
        , fLengths(nullptr)
        , fMemoryManager(mgr)
        , fTotalChars(0)
    {
        fStrings = new ValueVectorOf<char*>(initialCapacity, mgr, false);
        fLengths = new ValueVectorOf<unsigned int>(initialCapacity, mgr, false);
    }

    ~StringTable() {
        // 先释放所有字符串
        for (unsigned int i = 0; i < fStrings->size(); i++) {
            char* str = fStrings->elementAt(i);
            if (str) {
                fMemoryManager->deallocate(str);
            }
        }
        delete fStrings;
        delete fLengths;
    }

    // 添加字符串（复制）
    void addString(const char* str) {
        if (!str) return;
        unsigned int len = strlen(str);
        char* copy = (char*) fMemoryManager->allocate(len + 1);
        strcpy(copy, str);
        fStrings->addElement(copy);
        fLengths->addElement(len);
        fTotalChars += len;
    }

    // 获取字符串
    const char* getString(unsigned int index) const {
        if (index < fStrings->size()) {
            return fStrings->elementAt(index);
        }
        return nullptr;
    }

    // 获取字符串长度
    unsigned int getLength(unsigned int index) const {
        if (index < fLengths->size()) {
            return fLengths->elementAt(index);
        }
        return 0;
    }

    // 查找字符串
    int findString(const char* str) const {
        if (!str) return -1;
        for (unsigned int i = 0; i < fStrings->size(); i++) {
            if (strcmp(fStrings->elementAt(i), str) == 0) {
                return (int)i;
            }
        }
        return -1;
    }

    // 连接所有字符串
    char* concatenateAll() const {
        if (fTotalChars == 0) return nullptr;

        char* result = (char*) fMemoryManager->allocate(fTotalChars + fStrings->size());
        char* ptr = result;

        for (unsigned int i = 0; i < fStrings->size(); i++) {
            const char* str = fStrings->elementAt(i);
            unsigned int len = fLengths->elementAt(i);
            memcpy(ptr, str, len);
            ptr += len;
            if (i < fStrings->size() - 1) {
                *ptr++ = ' ';  // 空格分隔
            }
        }
        *ptr = '\0';
        return result;
    }

    unsigned int size() const { return fStrings->size(); }
    unsigned int totalChars() const { return fTotalChars; }
};

// ============================================================================
// 测试场景3：更复杂的包装类 - 带有多个数组字段的业务对象
// StudentRegistry - 学生注册表，包含多个 ValueVectorOf 字段
// ============================================================================
struct StudentRecord {
    int id;
    int age;
    double gpa;

    bool operator==(const StudentRecord& other) const {
        return id == other.id;
    }
};

class StudentRegistry {
private:
    ValueVectorOf<StudentRecord>* fStudents;    // 学生记录
    ValueVectorOf<int>* fStudentIds;            // 学生ID索引（用于快速查找）
    ValueVectorOf<int>* fScoreHistory;          // 分数历史记录
    MemoryManager* fMemoryManager;
    int fNextId;

public:
    StudentRegistry(MemoryManager* mgr, unsigned int initialCapacity = 32)
        : fStudents(nullptr)
        , fStudentIds(nullptr)
        , fScoreHistory(nullptr)
        , fMemoryManager(mgr)
        , fNextId(1000)
    {
        fStudents = new ValueVectorOf<StudentRecord>(initialCapacity, mgr, false);
        fStudentIds = new ValueVectorOf<int>(initialCapacity, mgr, false);
        fScoreHistory = new ValueVectorOf<int>(initialCapacity * 4, mgr, false);
    }

    ~StudentRegistry() {
        delete fStudents;
        delete fStudentIds;
        delete fScoreHistory;
    }

    // 添加学生
    int addStudent(int age, double gpa) {
        StudentRecord record;
        record.id = fNextId++;
        record.age = age;
        record.gpa = gpa;

        fStudents->addElement(record);
        fStudentIds->addElement(record.id);

        return record.id;
    }

    // 根据ID查找学生
    const StudentRecord* findStudent(int id) const {
        for (unsigned int i = 0; i < fStudentIds->size(); i++) {
            if (fStudentIds->elementAt(i) == id) {
                return &fStudents->elementAt(i);
            }
        }
        return nullptr;
    }

    // 添加分数记录
    void addScore(int studentId, int score) {
        // 存储格式: studentId, score (交替存储)
        fScoreHistory->addElement(studentId);
        fScoreHistory->addElement(score);
    }

    // 获取学生的所有分数
    void getScores(int studentId, ValueVectorOf<int>& scores) const {
        for (unsigned int i = 0; i < fScoreHistory->size(); i += 2) {
            if (fScoreHistory->elementAt(i) == studentId) {
                scores.addElement(fScoreHistory->elementAt(i + 1));
            }
        }
    }

    // 计算学生平均分
    double getAverageScore(int studentId) const {
        int total = 0;
        int count = 0;
        for (unsigned int i = 0; i < fScoreHistory->size(); i += 2) {
            if (fScoreHistory->elementAt(i) == studentId) {
                total += fScoreHistory->elementAt(i + 1);
                count++;
            }
        }
        return count > 0 ? (double)total / count : 0.0;
    }

    // 获取所有GPA高于阈值的学生
    void getHighPerformers(double minGpa, ValueVectorOf<int>& result) const {
        for (unsigned int i = 0; i < fStudents->size(); i++) {
            if (fStudents->elementAt(i).gpa >= minGpa) {
                result.addElement(fStudents->elementAt(i).id);
            }
        }
    }

    unsigned int studentCount() const { return fStudents->size(); }
    unsigned int scoreCount() const { return fScoreHistory->size() / 2; }
};

// 测试 ensureExtraCapacity 的扩容逻辑
void test_capacity_growth(MemoryManager* mgr) {
    cout << "\n=== Testing ensureExtraCapacity (1.25x growth) ===" << endl;
    ValueVectorOf<int> vec(2, mgr, false);  // 初始容量 2
    cout << "Initial capacity: " << vec.curCapacity() << endl;

    unsigned prevCapacity = vec.curCapacity();
    for (int i = 0; i < 20; i++) {
        vec.addElement(i * 10);
        unsigned newCapacity = vec.curCapacity();
        if (newCapacity != prevCapacity) {
            cout << "Capacity grew from " << prevCapacity << " to " << newCapacity
                 << " (at size " << vec.size() << ")" << endl;
            prevCapacity = newCapacity;
        }
    }
    cout << "Final: size=" << vec.size() << ", capacity=" << vec.curCapacity() << endl;
}

// 测试 rawData() 方法
void test_raw_data(MemoryManager* mgr) {
    cout << "\n=== Testing rawData() ===" << endl;
    ValueVectorOf<int> vec(10, mgr, false);
    for (int i = 0; i < 5; i++) {
        vec.addElement(i * 100);
    }

    const int* raw = vec.rawData();
    cout << "rawData() contents: ";
    for (unsigned i = 0; i < vec.size(); i++) {
        cout << raw[i] << " ";
    }
    cout << endl;

    // 验证与 elementAt 一致
    bool consistent = true;
    for (unsigned i = 0; i < vec.size(); i++) {
        if (raw[i] != vec.elementAt(i)) {
            consistent = false;
            break;
        }
    }
    cout << "rawData() consistent with elementAt(): " << (consistent ? "yes" : "no") << endl;
}

// 测试 fCallDestructor=true
void test_destructor_calls(MemoryManager* mgr) {
    cout << "\n=== Testing fCallDestructor=true ===" << endl;
    DestructorCounter::destructor_count = 0;

    {
        ValueVectorOf<DestructorCounter> vec(5, mgr, true);  // toCallDestructor=true
        for (int i = 0; i < 3; i++) {
            vec.addElement(DestructorCounter(i));
        }
        cout << "Added 3 elements, destructor calls so far (from copies): "
             << DestructorCounter::destructor_count << endl;
        // vec 将在作用域结束时销毁
    }

    cout << "After vector destroyed, total destructor calls: "
         << DestructorCounter::destructor_count << endl;
}

// 测试边界条件
void test_edge_cases(MemoryManager* mgr) {
    cout << "\n=== Testing edge cases ===" << endl;

    // 空向量测试
    cout << "-- Empty vector tests --" << endl;
    ValueVectorOf<int> empty(1, mgr, false);
    cout << "Empty vector size: " << empty.size() << endl;
    cout << "Empty vector capacity: " << empty.curCapacity() << endl;
    cout << "containsElement(0) on empty: " << (empty.containsElement(0) ? "true" : "false") << endl;

    // 单元素测试
    cout << "-- Single element tests --" << endl;
    empty.addElement(42);
    cout << "After adding 42: size=" << empty.size() << endl;
    empty.removeElementAt(0);  // 删除唯一元素
    cout << "After removing: size=" << empty.size() << endl;

    // 位置 0 插入测试
    cout << "-- Insert at position 0 tests --" << endl;
    ValueVectorOf<int> vec(5, mgr, false);
    vec.addElement(100);
    vec.addElement(200);
    vec.addElement(300);
    cout << "Before insertAt(0): ";
    for (unsigned i = 0; i < vec.size(); i++) cout << vec.elementAt(i) << " ";
    cout << endl;

    vec.insertElementAt(50, 0);  // 在开头插入
    cout << "After insertAt(0, 50): ";
    for (unsigned i = 0; i < vec.size(); i++) cout << vec.elementAt(i) << " ";
    cout << endl;

    // 末尾插入（通过 insertElementAt）
    vec.insertElementAt(999, vec.size());  // 在末尾插入
    cout << "After insertAt(end, 999): ";
    for (unsigned i = 0; i < vec.size(); i++) cout << vec.elementAt(i) << " ";
    cout << endl;

    // 删除首元素
    vec.removeElementAt(0);
    cout << "After removeAt(0): ";
    for (unsigned i = 0; i < vec.size(); i++) cout << vec.elementAt(i) << " ";
    cout << endl;
}

// 测试 getMemoryManager()
void test_get_memory_manager(MemoryManager* mgr) {
    cout << "\n=== Testing getMemoryManager() ===" << endl;
    ValueVectorOf<int> vec(5, mgr, false);
    MemoryManager* retrieved = vec.getMemoryManager();
    cout << "getMemoryManager() returns same as passed: "
         << (retrieved == mgr ? "yes" : "no") << endl;
}

// ============================================================================
// 测试场景1：VectorProcessor 与 ValueVectorOf 的交互
// ============================================================================
void test_vector_processor(MemoryManager* mgr) {
    cout << "\n=== Testing VectorProcessor (class interaction) ===" << endl;

    // 创建 VectorProcessor
    VectorProcessor processor(mgr, 8);
    cout << "Created VectorProcessor with initial capacity 8" << endl;

    // 创建一些 ValueVectorOf 来测试交互
    ValueVectorOf<int> source1(10, mgr, false);
    ValueVectorOf<int> source2(10, mgr, false);
    ValueVectorOf<int> result(20, mgr, false);

    // 填充源向量
    for (int i = 0; i < 5; i++) {
        source1.addElement(i * 10);
        source2.addElement(i * 100);
    }
    cout << "Source1: ";
    for (unsigned i = 0; i < source1.size(); i++) cout << source1.elementAt(i) << " ";
    cout << endl;
    cout << "Source2: ";
    for (unsigned i = 0; i < source2.size(); i++) cout << source2.elementAt(i) << " ";
    cout << endl;

    // 测试 processVector
    cout << "Processing source1..." << endl;
    processor.processVector(source1);
    cout << "Result count after processing: " << processor.getResultCount() << endl;

    // 测试 mergeAndProcess
    VectorProcessor processor2(mgr, 16);
    processor2.mergeAndProcess(source1, source2);
    cout << "Merged result count: " << processor2.getResultCount() << endl;

    // 测试 copyResultsTo
    processor2.copyResultsTo(result);
    cout << "Copied to result vector, size: " << result.size() << endl;

    // 测试 filterVector
    ValueVectorOf<int> filtered(10, mgr, false);
    processor.filterVector(source2, filtered, 150);
    cout << "Filtered (>150): ";
    for (unsigned i = 0; i < filtered.size(); i++) cout << filtered.elementAt(i) << " ";
    cout << endl;

    cout << "VectorProcessor test completed" << endl;
}

// ============================================================================
// 测试场景2：StringTable 作为封装 ValueVectorOf 的包装类
// ============================================================================
void test_string_table(MemoryManager* mgr) {
    cout << "\n=== Testing StringTable (wrapper class) ===" << endl;

    StringTable table(mgr, 8);
    cout << "Created StringTable" << endl;

    // 添加字符串
    table.addString("Hello");
    table.addString("World");
    table.addString("Test");
    table.addString("String");
    cout << "Added 4 strings, size: " << table.size() << endl;
    cout << "Total characters: " << table.totalChars() << endl;

    // 获取字符串
    cout << "Strings in table:" << endl;
    for (unsigned i = 0; i < table.size(); i++) {
        cout << "  [" << i << "] \"" << table.getString(i) << "\" (len=" << table.getLength(i) << ")" << endl;
    }

    // 查找字符串
    int idx = table.findString("World");
    cout << "findString(\"World\"): " << idx << endl;
    idx = table.findString("NotFound");
    cout << "findString(\"NotFound\"): " << idx << endl;

    // 连接所有字符串
    char* concat = table.concatenateAll();
    if (concat) {
        cout << "Concatenated: \"" << concat << "\"" << endl;
        mgr->deallocate(concat);
    }

    cout << "StringTable test completed" << endl;
}

// ============================================================================
// 测试场景3：StudentRegistry 作为包含多个 ValueVectorOf 字段的复杂包装类
// ============================================================================
void test_student_registry(MemoryManager* mgr) {
    cout << "\n=== Testing StudentRegistry (complex wrapper) ===" << endl;

    StudentRegistry registry(mgr, 16);
    cout << "Created StudentRegistry" << endl;

    // 添加学生
    int id1 = registry.addStudent(20, 3.8);
    int id2 = registry.addStudent(21, 3.5);
    int id3 = registry.addStudent(19, 3.9);
    cout << "Added 3 students with IDs: " << id1 << ", " << id2 << ", " << id3 << endl;
    cout << "Student count: " << registry.studentCount() << endl;

    // 添加分数记录
    registry.addScore(id1, 85);
    registry.addScore(id1, 90);
    registry.addScore(id1, 88);
    registry.addScore(id2, 78);
    registry.addScore(id2, 82);
    registry.addScore(id3, 95);
    registry.addScore(id3, 92);
    cout << "Score count: " << registry.scoreCount() << endl;

    // 查找学生
    const StudentRecord* found = registry.findStudent(id2);
    if (found) {
        cout << "Found student " << id2 << ": age=" << found->age << ", gpa=" << found->gpa << endl;
    }

    // 获取学生分数
    ValueVectorOf<int> scores(10, mgr, false);
    registry.getScores(id1, scores);
    cout << "Scores for student " << id1 << ": ";
    for (unsigned i = 0; i < scores.size(); i++) cout << scores.elementAt(i) << " ";
    cout << endl;

    // 计算平均分
    cout << "Average score for student " << id1 << ": " << registry.getAverageScore(id1) << endl;
    cout << "Average score for student " << id3 << ": " << registry.getAverageScore(id3) << endl;

    // 获取高绩点学生
    ValueVectorOf<int> highPerformers(10, mgr, false);
    registry.getHighPerformers(3.7, highPerformers);
    cout << "High performers (GPA >= 3.7): ";
    for (unsigned i = 0; i < highPerformers.size(); i++) cout << highPerformers.elementAt(i) << " ";
    cout << endl;

    cout << "StudentRegistry test completed" << endl;
}

int main() {
    try {
        // 初始化XML平台工具
        cout << "Initializing XMLPlatformUtils..." << endl;
        XMLPlatformUtils::Initialize(nullptr);

        // 将所有向量操作放在一个块中，确保在Terminate前被销毁
        {
            // 测试1: 基本操作测试 - 整数类型
            cout << "\n=== Testing ValueVectorOf<int> ===" << endl;
            // 使用正确的构造函数参数
            ValueVectorOf<int> intVector(5, XMLPlatformUtils::fgMemoryManager, false);  // 最大容量5，不需要调用析构函数的基本类型

            // 测试添加元素
            cout << "Adding elements..." << endl;
            for (int i = 0; i < 10; i++) {
                intVector.addElement(i * 10);
            }

            // 测试大小和容量
            cout << "Size: " << intVector.size() << ", Capacity: " << intVector.curCapacity() << endl;

            // 测试访问元素
            cout << "Elements: ";
            for (unsigned i = 0; i < intVector.size(); i++) {
                cout << intVector.elementAt(i) << " ";
            }
            cout << endl;

            // 测试修改元素
            cout << "Setting element at index 5 to 999..." << endl;
            intVector.setElementAt(999, 5);
            cout << "Element at index 5: " << intVector.elementAt(5) << endl;

            // 测试插入元素
            cout << "Inserting 888 at index 3..." << endl;
            intVector.insertElementAt(888, 3);
            cout << "Size after insertion: " << intVector.size() << endl;
            cout << "Elements after insertion: ";
            for (unsigned i = 0; i < intVector.size(); i++) {
                cout << intVector.elementAt(i) << " ";
            }
            cout << endl;

            // 测试删除元素
            cout << "Removing element at index 7..." << endl;
            intVector.removeElementAt(7);
            cout << "Size after removal: " << intVector.size() << endl;
            cout << "Elements after removal: ";
            for (unsigned i = 0; i < intVector.size(); i++) {
                cout << intVector.elementAt(i) << " ";
            }
            cout << endl;

            // 测试containsElement
            bool contains = intVector.containsElement(888);
            cout << "Contains 888: " << (contains ? "true" : "false") << endl;
            contains = intVector.containsElement(1000);
            cout << "Contains 1000: " << (contains ? "true" : "false") << endl;

            // 测试2: 自定义对象类型测试
            cout << "\n=== Testing ValueVectorOf<TestObject> ===" << endl;
            // 使用正确的构造函数参数
            ValueVectorOf<TestObject> objVector(3, XMLPlatformUtils::fgMemoryManager, false);  // 最大容量3，POD类型无需调用析构函数

            // 添加自定义对象
            cout << "Adding TestObjects..." << endl;
            objVector.addElement(TestObject::create(1, 100, 95.5));
            objVector.addElement(TestObject::create(2, 200, 87.3));
            objVector.addElement(TestObject::create(3, 300, 92.8));

            // 显示对象
            cout << "TestObjects in vector: " << endl;
            for (unsigned i = 0; i < objVector.size(); i++) {
                const TestObject& obj = objVector.elementAt(i);
                cout << "Object " << i << ": ";
                obj.display();
                cout << endl;
            }

            // 测试拷贝构造函数
            cout << "\nTesting copy constructor..." << endl;
            ValueVectorOf<TestObject> objVectorCopy = objVector;
            cout << "Copied vector size: " << objVectorCopy.size() << endl;

            // 修改拷贝的元素，验证深拷贝
            cout << "Modifying element in copy..." << endl;
            TestObject& obj = objVectorCopy.elementAt(1);
            obj.value = 99;
            obj.id = 999;
            obj.score = 99.9;

            // 验证原向量和拷贝向量的元素是否不同
            cout << "Original vector element 1: ";
            objVector.elementAt(1).display();
            cout << endl;
            cout << "Copy vector element 1: ";
            objVectorCopy.elementAt(1).display();
            cout << endl;

            // 测试赋值操作符
            cout << "\nTesting assignment operator..." << endl;
            ValueVectorOf<TestObject> objVectorAssign(1, XMLPlatformUtils::fgMemoryManager, false);  // 最大容量1，POD类型无需调用析构函数
            objVectorAssign.addElement(TestObject::create(999, 888, 77.7));
            objVectorAssign = objVector;  // 赋值操作

            cout << "Assigned vector size: " << objVectorAssign.size() << endl;
            cout << "Assigned vector element 0: ";
            objVectorAssign.elementAt(0).display();
            cout << endl;

            // 测试removeAllElements
            cout << "\nTesting removeAllElements..." << endl;
            objVector.removeAllElements();
            cout << "Size after removeAllElements: " << objVector.size() << endl;

            // 测试内存管理
            cout << "\nTesting memory management..." << endl;
            // 这里不做显式测试，但确保所有对象都被正确销毁
        }

        // 新增测试：ensureExtraCapacity 扩容逻辑
        test_capacity_growth(XMLPlatformUtils::fgMemoryManager);

        // 新增测试：rawData() 方法
        test_raw_data(XMLPlatformUtils::fgMemoryManager);

        // 新增测试：fCallDestructor=true
        test_destructor_calls(XMLPlatformUtils::fgMemoryManager);

        // 新增测试：边界条件
        test_edge_cases(XMLPlatformUtils::fgMemoryManager);

        // 新增测试：getMemoryManager()
        test_get_memory_manager(XMLPlatformUtils::fgMemoryManager);

        // ================================================================
        // 新增测试：类与数组类之间的交互测试
        // ================================================================
        cout << "\n========================================" << endl;
        cout << "Testing class interactions with ValueVectorOf" << endl;
        cout << "========================================" << endl;

        // 测试 VectorProcessor（外部类与 ValueVectorOf 的交互）
        test_vector_processor(XMLPlatformUtils::fgMemoryManager);

        // 测试 StringTable（封装 ValueVectorOf 为字段的包装类）
        test_string_table(XMLPlatformUtils::fgMemoryManager);

        // 测试 StudentRegistry（包含多个 ValueVectorOf 字段的复杂包装类）
        test_student_registry(XMLPlatformUtils::fgMemoryManager);

        cout << "\n========================================" << endl;
        cout << "All interaction tests completed!" << endl;
        cout << "========================================" << endl;

        // 清理资源
        cout << "\nTerminating XMLPlatformUtils..." << endl;
        XMLPlatformUtils::Terminate();

        cout << "\nAll tests completed successfully!" << endl;
        return 0;
    } catch (const OutOfMemoryException& e) {
        cerr << "OutOfMemoryException caught!" << endl;
        return 1;
    } catch (const exception& e) {
        cerr << "Standard exception caught: " << e.what() << endl;
        return 1;
    } catch (...) {
        cerr << "Unknown exception caught!" << endl;
        return 1;
    }
}