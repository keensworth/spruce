#include <vector>
#include "gtest/gtest.h"
#include "../src/core/memory/Handle.h"
#include "../src/core/memory/Pool.h"

typedef struct {
    int a;
    float b;
    int c;
    int d;
} TestStruct;

using namespace spr;

TEST(PoolHandleTest, ResizeTest) { 
    Pool<TestStruct> pool = Pool<TestStruct>(2);

    TestStruct test1{.a=1,.b=0.3f,.c=1001,.d=99991};
    TestStruct test2{.a=2,.b=0.34f,.c=1002,.d=99992};
    TestStruct test3{.a=3,.b=0.345f,.c=1003,.d=99993};
    TestStruct test4{.a=4,.b=0.3456f,.c=1004,.d=99994};
    TestStruct test5{.a=5,.b=0.34567f,.c=1005,.d=99995};

    Handle<TestStruct> handle1 = pool.insert(test1);
    Handle<TestStruct> handle2 = pool.insert(test2);
    Handle<TestStruct> handle3 = pool.insert(test3);
    Handle<TestStruct> handle4 = pool.insert(test4);
    Handle<TestStruct> handle5 = pool.insert(test5);

    TestStruct* result1 = pool.get(handle1);
    EXPECT_EQ(result1->a, test1.a);
    EXPECT_EQ(result1->b, test1.b);
    EXPECT_EQ(result1->c, test1.c);
    EXPECT_EQ(result1->d, test1.d);

    TestStruct* result2 = pool.get(handle2);
    EXPECT_EQ(result2->a, test2.a);
    EXPECT_EQ(result2->b, test2.b);
    EXPECT_EQ(result2->c, test2.c);
    EXPECT_EQ(result2->d, test2.d);

    TestStruct* result3 = pool.get(handle3);
    EXPECT_EQ(result3->a, test3.a);
    EXPECT_EQ(result3->b, test3.b);
    EXPECT_EQ(result3->c, test3.c);
    EXPECT_EQ(result3->d, test3.d);

    TestStruct* result4 = pool.get(handle4);
    EXPECT_EQ(result4->a, test4.a);
    EXPECT_EQ(result4->b, test4.b);
    EXPECT_EQ(result4->c, test4.c);
    EXPECT_EQ(result4->d, test4.d);

    TestStruct* result5 = pool.get(handle5);
    EXPECT_EQ(result5->a, test5.a);
    EXPECT_EQ(result5->b, test5.b);
    EXPECT_EQ(result5->c, test5.c);
    EXPECT_EQ(result5->d, test5.d);
}

TEST(PoolHandleTest, RemoveOneNullTest) { 
    Pool<TestStruct> pool = Pool<TestStruct>(64);

    TestStruct test1{.a=1,.b=0.3f,.c=1001,.d=99991};
    TestStruct test2{.a=2,.b=0.34f,.c=1002,.d=99992};

    Handle<TestStruct> handle1 = pool.insert(test1);
    Handle<TestStruct> handle2 = pool.insert(test2);

    pool.remove(handle2);

    TestStruct* result = pool.get(handle2);
    //std::cout << handle2.isValid() << std::endl;
    EXPECT_TRUE(result == nullptr);
}

TEST(PoolHandleTest, RemoveNullReplaceTest) { 
    Pool<TestStruct> pool = Pool<TestStruct>(64);

    TestStruct test1{.a=1,.b=0.3f,.c=1001,.d=99991};
    TestStruct test2{.a=2,.b=0.34f,.c=1002,.d=99992};
    TestStruct test3{.a=3,.b=0.345f,.c=1003,.d=99993};

    Handle<TestStruct> handle1 = pool.insert(test1);
    Handle<TestStruct> handle2 = pool.insert(test2);

    pool.remove(handle2);

    TestStruct* result1 = pool.get(handle2);
    EXPECT_TRUE(result1 == nullptr);

    Handle<TestStruct> handle3 = pool.insert(test3);
    TestStruct* result2 = pool.get(handle3);
    EXPECT_TRUE(handle3.isValid());
    EXPECT_TRUE(result2 != nullptr);
    EXPECT_EQ(result2->a, test3.a);
}

TEST(PoolHandleTest, RemoveMany) { 
    Pool<TestStruct> pool = Pool<TestStruct>(64);

    TestStruct test1{.a=1,.b=0.3f,.c=1001,.d=99991};
    TestStruct test2{.a=2,.b=0.34f,.c=1002,.d=99992};
    TestStruct test3{.a=3,.b=0.345f,.c=1003,.d=99993};

    Handle<TestStruct> handle1 = pool.insert(test1);
    Handle<TestStruct> handle2 = pool.insert(test2);

    pool.remove(handle2);
    pool.remove(handle1);
    pool.remove(handle1);
    pool.remove(handle1);

    Handle<TestStruct> handle3 = pool.insert(test3);
    TestStruct* result2 = pool.get(handle3);
    EXPECT_TRUE(handle3.isValid());
    EXPECT_TRUE(result2 != nullptr);
    EXPECT_EQ(result2->a, test3.a);
}

TEST(PoolHandleTest, ResizeRemoveManyAddMany) { 
    Pool<TestStruct> pool = Pool<TestStruct>(2);

    TestStruct test1{.a=1,.b=0.3f,.c=1001,.d=99991};
    TestStruct test2{.a=2,.b=0.34f,.c=1002,.d=99992};
    TestStruct test3{.a=3,.b=0.345f,.c=1003,.d=99993};
    TestStruct test4{.a=4,.b=0.3456f,.c=1004,.d=99994};
    TestStruct test5{.a=5,.b=0.34567f,.c=1005,.d=99995};

    Handle<TestStruct> handle1 = pool.insert(test1);
    Handle<TestStruct> handle2 = pool.insert(test2);
    TestStruct* result1 = pool.get(handle1);
    EXPECT_EQ(result1->a, test1.a);
    EXPECT_EQ(result1->b, test1.b);
    EXPECT_EQ(result1->c, test1.c);
    EXPECT_EQ(result1->d, test1.d);
    pool.remove(handle2);
    result1 = pool.get(handle1);
    EXPECT_EQ(result1->a, test1.a);
    EXPECT_EQ(result1->b, test1.b);
    EXPECT_EQ(result1->c, test1.c);
    EXPECT_EQ(result1->d, test1.d);
    Handle<TestStruct> handle3 = pool.insert(test3);
    Handle<TestStruct> handle4 = pool.insert(test4);
    Handle<TestStruct> handle5 = pool.insert(test5);
    result1 = pool.get(handle1);
    EXPECT_EQ(result1->a, test1.a);
    EXPECT_EQ(result1->b, test1.b);
    EXPECT_EQ(result1->c, test1.c);
    EXPECT_EQ(result1->d, test1.d);
    pool.remove(handle4);
    result1 = pool.get(handle1);
    EXPECT_EQ(result1->a, test1.a);
    EXPECT_EQ(result1->b, test1.b);
    EXPECT_EQ(result1->c, test1.c);
    EXPECT_EQ(result1->d, test1.d);
    TestStruct* result2 = pool.get(handle3);
    EXPECT_EQ(result2->a, test3.a);
    EXPECT_EQ(result2->b, test3.b);
    EXPECT_EQ(result2->c, test3.c);
    EXPECT_EQ(result2->d, test3.d);
}

TEST(PoolHandleTest, OverflowTest) { 
    Pool<TestStruct> pool = Pool<TestStruct>(1024);

    TestStruct test1{.a=1,.b=0.3f,.c=1001,.d=99991};
    TestStruct test2{.a=2,.b=0.34f,.c=1002,.d=99992};
    std::vector<Handle<TestStruct>> handles;

    for (int i = 0; i < 1000000; i++){
      pool.insert(i%2==0?test1:test2);
      // handles.push_back(pool.insert(i%2==0?test1:test2));
      // if (i%17 == 0){
      //   pool.remove(handles.at(i/17));
      // }
    }
    EXPECT_TRUE(1);
}


int main() {
::testing::InitGoogleTest();
  return RUN_ALL_TESTS();
}