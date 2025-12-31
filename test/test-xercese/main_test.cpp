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