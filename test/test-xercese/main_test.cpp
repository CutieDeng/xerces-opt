#include <iostream>
#include <cstring>
#include "xercese/util/PlatformUtils.hh"
#include "xercese/util/ValueVectorOf.hh"
#include "xercese/util/OutOfMemoryException.hh"

using namespace xercese;
using namespace std;

// 定义一个简单的测试类用于测试ValueVectorOf
class TestObject {
private:
    int m_value;
    char* m_name;

public:
    TestObject(int value = 0, const char* name = "default") : m_value(value) {
        if (name) {
            m_name = new char[strlen(name) + 1];
            strcpy(m_name, name);
        } else {
            m_name = new char[1];
            m_name[0] = '\0';
        }
    }

    TestObject(const TestObject& other) : m_value(other.m_value) {
        m_name = new char[strlen(other.m_name) + 1];
        strcpy(m_name, other.m_name);
    }

    ~TestObject() {
        delete[] m_name;
    }

    TestObject& operator=(const TestObject& other) {
        if (this != &other) {
            m_value = other.m_value;
            delete[] m_name;
            m_name = new char[strlen(other.m_name) + 1];
            strcpy(m_name, other.m_name);
        }
        return *this;
    }

    bool operator==(const TestObject& other) const {
        return m_value == other.m_value && strcmp(m_name, other.m_name) == 0;
    }

    int getValue() const { return m_value; }
    const char* getName() const { return m_name; }

    void setValue(int value) { m_value = value; }
    void setName(const char* name) {
        delete[] m_name;
        m_name = new char[strlen(name) + 1];
        strcpy(m_name, name);
    }
};

int main() {
    try {
        // 初始化XML平台工具
        cout << "Initializing XMLPlatformUtils..." << endl;
        XMLPlatformUtils::Initialize(nullptr);

        // 测试1: 基本操作测试 - 整数类型
        cout << "\n=== Testing ValueVectorOf<int> ===" << endl;
        // 使用正确的构造函数参数
        ValueVectorOf<int> intVector(5, nullptr, false);  // 最大容量5，不需要调用析构函数的基本类型

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
        ValueVectorOf<TestObject> objVector(3, nullptr, true);  // 最大容量3，需要调用析构函数的对象类型

        // 添加自定义对象
        cout << "Adding TestObjects..." << endl;
        objVector.addElement(TestObject(1, "Object1"));
        objVector.addElement(TestObject(2, "Object2"));
        objVector.addElement(TestObject(3, "Object3"));

        // 显示对象
        cout << "TestObjects in vector: " << endl;
        for (unsigned i = 0; i < objVector.size(); i++) {
            const TestObject& obj = objVector.elementAt(i);
            cout << "Object " << i << ": value=" << obj.getValue() << ", name=" << obj.getName() << endl;
        }

        // 测试拷贝构造函数
        cout << "\nTesting copy constructor..." << endl;
        ValueVectorOf<TestObject> objVectorCopy = objVector;
        cout << "Copied vector size: " << objVectorCopy.size() << endl;

        // 修改拷贝的元素，验证深拷贝
        cout << "Modifying element in copy..." << endl;
        TestObject& obj = objVectorCopy.elementAt(1);
        obj.setValue(99);
        obj.setName("ModifiedObject");

        // 验证原向量和拷贝向量的元素是否不同
        cout << "Original vector element 1: value=" << objVector.elementAt(1).getValue() << ", name=" << objVector.elementAt(1).getName() << endl;
        cout << "Copy vector element 1: value=" << objVectorCopy.elementAt(1).getValue() << ", name=" << objVectorCopy.elementAt(1).getName() << endl;

        // 测试赋值操作符
        cout << "\nTesting assignment operator..." << endl;
        ValueVectorOf<TestObject> objVectorAssign(1, nullptr, true);  // 最大容量1
        objVectorAssign.addElement(TestObject(999, "TempObject"));
        objVectorAssign = objVector;  // 赋值操作

        cout << "Assigned vector size: " << objVectorAssign.size() << endl;
        cout << "Assigned vector element 0: value=" << objVectorAssign.elementAt(0).getValue() << ", name=" << objVectorAssign.elementAt(0).getName() << endl;

        // 测试removeAllElements
        cout << "\nTesting removeAllElements..." << endl;
        objVector.removeAllElements();
        cout << "Size after removeAllElements: " << objVector.size() << endl;

        // 测试内存管理
        cout << "\nTesting memory management..." << endl;
        // 这里不做显式测试，但确保所有对象都被正确销毁

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