#include <gtest/gtest.h>
#include "ProtocolParser.h"
#include "StorageManager.h"
#include "KVStore.h"
#include <cstdio> // For std::remove

// =================================================================
// TEST 1: The Protocol Parser (Pipelining)
// =================================================================
TEST(ParserTest, HandlesPipelinedCommands) {
    // Two commands glued together: SET mykey 1, then GET mykey
    std::string buffer = "*3\r\n$3\r\nSET\r\n$5\r\nmykey\r\n$1\r\n1\r\n*2\r\n$3\r\nGET\r\n$5\r\nmykey\r\n";

    // Parse the first command
    auto [tokens1, consumed1] = parse_resp(buffer);
    
    ASSERT_EQ(tokens1.size(), 3);
    EXPECT_EQ(tokens1[0], "SET");
    EXPECT_EQ(tokens1[1], "mykey");
    EXPECT_EQ(tokens1[2], "1");

    // Simulate the server erasing the consumed bytes
    buffer.erase(0, consumed1);

    // Parse the second command
    auto [tokens2, consumed2] = parse_resp(buffer);
    
    ASSERT_EQ(tokens2.size(), 2);
    EXPECT_EQ(tokens2[0], "GET");
    EXPECT_EQ(tokens2[1], "mykey");

    // Buffer should now be empty
    buffer.erase(0, consumed2);
    EXPECT_TRUE(buffer.empty());
}

// =================================================================
// TEST 2: Storage Manager (AOF Persistence)
// =================================================================
TEST(StorageTest, AppendsAndRecoversData) {
    const std::string test_file = "test_appendonly.aof";
    
    // Setup: Ensure we start with a clean slate by deleting any old test file
    std::remove(test_file.c_str());

    // Step 1: Create a database and write some data
    {
        KVStore db1(100);
        StorageManager storage(test_file);

        // Simulate a client sending SET player_name Mario
        std::vector<std::string> cmd1 = {"SET", "player_name", "Mario"};
        db1.set(cmd1[1], cmd1[2]);
        storage.append_command(cmd1);

        // Check it's in DB1
        EXPECT_EQ(db1.get("player_name").value(), "Mario");
        
        // As this scope ends, db1 and storage are destroyed, closing the file.
    }

    // Step 2: Create a BRAND NEW database, and recover from the file
    {
        KVStore db2(100);
        StorageManager storage2(test_file);

        // Before loading, db2 should be empty
        EXPECT_FALSE(db2.get("player_name").has_value());

        // Recover!
        storage2.load_aof(db2);

        // The moment of truth: Does db2 have the data from db1?
        ASSERT_TRUE(db2.get("player_name").has_value());
        EXPECT_EQ(db2.get("player_name").value(), "Mario");
    }

    // Cleanup
    std::remove(test_file.c_str());
}