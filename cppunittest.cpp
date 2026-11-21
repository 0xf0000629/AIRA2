#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/ui/text/TestRunner.h>
#include "arrangement.h"

class MyTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(MyTest);
    CPPUNIT_TEST(test_creation);
    CPPUNIT_TEST(test_creationwithvalue);
    CPPUNIT_TEST(test_multidimensional);
    CPPUNIT_TEST(test_size);
    CPPUNIT_TEST(test_assignment);
    CPPUNIT_TEST(test_forwarditerator);
    CPPUNIT_TEST(test_backwarditerator);
    CPPUNIT_TEST(test_insert);
    CPPUNIT_TEST(test_remove);
    CPPUNIT_TEST(test_growth);
    CPPUNIT_TEST_SUITE_END();

public:
    void test_creation() {
        arrangement<int> a(10);
        CPPUNIT_ASSERT_EQUAL(size_t(0), a.size());
        CPPUNIT_ASSERT_EQUAL(size_t(10), a.capacity());
    }

    void test_creationwithvalue() {
        arrangement<int> a(5, 123456789);
        CPPUNIT_ASSERT_EQUAL(size_t(5), a.size());
        CPPUNIT_ASSERT_EQUAL(size_t(5), a.capacity());
        for (size_t i=0; i<a.size(); i++)
            CPPUNIT_ASSERT_EQUAL(123456789, a[i]);
    }

    void test_multidimensional() {
        arrangement<arrangement<int>> a(3);

        for (int i=0; i<3; i++) {
            arrangement<int> q(3);
            for (int j=0; j<3; j++)
                q.insert(j, i*10+j);
            a.insert(i, q);
        }

        CPPUNIT_ASSERT_EQUAL(size_t(3), a.size());

        for (int i=0; i<3; i++) {
            CPPUNIT_ASSERT_EQUAL(size_t(3), a[i].size());
            for (int j=0; j<3; j++)
                CPPUNIT_ASSERT_EQUAL(i*10+j, a[i][j]);
        }

        a[1].insert(3, -1);
        CPPUNIT_ASSERT_EQUAL(size_t(4), a[1].size());
        CPPUNIT_ASSERT_EQUAL(-1, a[1][3]);
        CPPUNIT_ASSERT_EQUAL(size_t(3), a[0].size());
    }
    void test_size() {
        arrangement<int> a(2);

        CPPUNIT_ASSERT_EQUAL(size_t(0), a.size());
        CPPUNIT_ASSERT_EQUAL(size_t(2), a.capacity());

        a.insert(0, 1);
        a.insert(1, 2);

        CPPUNIT_ASSERT_EQUAL(size_t(2), a.size());

        a.insert(2, 3);

        CPPUNIT_ASSERT_EQUAL(size_t(3), a.size());
        CPPUNIT_ASSERT(a.capacity() >= 3);
    }
    void test_assignment() {
        arrangement<int> a(3);

        a.insert(0, 1);
        a.insert(1, 2);
        a.insert(2, 3);

        arrangement<int> b(1);
        b=a;

        CPPUNIT_ASSERT_EQUAL(a.size(), b.size());

        for (size_t i=0; i<a.size(); i++)
            CPPUNIT_ASSERT_EQUAL(a[i], b[i]);

        b[0]=-1;

        CPPUNIT_ASSERT_EQUAL(1, a[0]);
        CPPUNIT_ASSERT_EQUAL(-1, b[0]);

        arrangement<int> c(a);

        CPPUNIT_ASSERT_EQUAL(a.size(), c.size());
        CPPUNIT_ASSERT_EQUAL(2, c[1]);
    }
    void test_forwarditerator() {
        arrangement<int> a(3, 0);

        a[0]=1;
        a[1]=2;
        a[2]=3;

        auto it=a.iterator();
        CPPUNIT_ASSERT(it.hasNext());
        CPPUNIT_ASSERT_EQUAL(1, it.get());
        it.next();
        CPPUNIT_ASSERT_EQUAL(2, it.get());
        it.next();
        CPPUNIT_ASSERT_EQUAL(3, it.get());
        it.next();
        CPPUNIT_ASSERT(!it.hasNext());
    }
    void test_backwarditerator() {
        arrangement<int> a(3, 0);

        a[0]=4;
        a[1]=5;
        a[2]=6;

        auto it=a.reverseiterator();

        CPPUNIT_ASSERT(it.hasNext());
        CPPUNIT_ASSERT_EQUAL(6, it.get());
        it.next();
        CPPUNIT_ASSERT_EQUAL(5, it.get());
        it.next();
        CPPUNIT_ASSERT_EQUAL(4, it.get());
        it.next();
        CPPUNIT_ASSERT(!it.hasNext());
    }
    void test_insert() {
        arrangement<std::string> a(1, "Hello");

        std::string second="World";
        a.insert(1, std::move(second));

        std::string third="!";
        a.insert(2, &third);

        CPPUNIT_ASSERT_EQUAL(size_t(3), a.size());
        CPPUNIT_ASSERT_EQUAL(std::string("Hello"), a[0]);
        CPPUNIT_ASSERT_EQUAL(std::string("World"), a[1]);
        CPPUNIT_ASSERT_EQUAL(std::string("!"), a[2]);
    }
    void test_remove() {
        arrangement<int> a(5);

        for (int i=0; i<5; i++)
            a.insert(i, i+1);

        a.remove(2);
        CPPUNIT_ASSERT_EQUAL(size_t(4), a.size());
        CPPUNIT_ASSERT_EQUAL(1, a[0]);
        CPPUNIT_ASSERT_EQUAL(2, a[1]);
        CPPUNIT_ASSERT_EQUAL(4, a[2]);
        CPPUNIT_ASSERT_EQUAL(5, a[3]);

        a.remove(0);
        CPPUNIT_ASSERT_EQUAL(size_t(3), a.size());
        CPPUNIT_ASSERT_EQUAL(2, a[0]);

        a.remove(a.size() - 1);
        CPPUNIT_ASSERT_EQUAL(size_t(2), a.size());
        CPPUNIT_ASSERT_EQUAL(4, a[1]);
    }
    void test_growth() {
        arrangement<int> a(1);
        for (int i=0; i<100; i++)
            a.insert(a.size(), i);

        CPPUNIT_ASSERT_EQUAL(size_t(100), a.size());
        CPPUNIT_ASSERT(a.capacity() >= 100);

        for (int i=0; i<100; i++)
            CPPUNIT_ASSERT_EQUAL(i, a[i]);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(MyTest);

int main()
{
    CppUnit::TextTestRunner runner;
    runner.addTest(
        CppUnit::TestFactoryRegistry::getRegistry().makeTest()
    );

    return runner.run() ? 0 : 1;
}
