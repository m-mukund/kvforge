#include <gtest/gtest.h>
#include "../include/KVStore.h"

TEST(KVStoreTest, BasicSetAndGet) {
    KVStore db(100);
    db.set("name", "Mukund");
    
    auto result = db.get("name");
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), "Mukund");
}

TEST(KVStoreTest, GetMissingKey) {
    KVStore db(100);
    EXPECT_FALSE(db.get("missing").has_value());
}

TEST(KVStoreTest, DeleteKey) {
    KVStore db(100);
    db.set("language", "cpp");
    
    EXPECT_TRUE(db.del("language")); // Should return true
    EXPECT_FALSE(db.get("language").has_value()); // Should be gone
    EXPECT_FALSE(db.del("language")); // Deleting again should return false
}

TEST(KVStoreTest, LRUEviction) {
    KVStore db(3); 
    
    db.set("A", "1");
    db.set("B", "2");
    db.set("C", "3");
    
    db.set("D", "4");
    
    EXPECT_FALSE(db.get("A").has_value()); 
    EXPECT_TRUE(db.get("B").has_value());
    EXPECT_TRUE(db.get("C").has_value());
    EXPECT_TRUE(db.get("D").has_value());
    
    db.get("B");
    db.set("E", "5");
    
    EXPECT_FALSE(db.get("C").has_value()); 
    EXPECT_TRUE(db.get("B").has_value());  
}