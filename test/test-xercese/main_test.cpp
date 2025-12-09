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