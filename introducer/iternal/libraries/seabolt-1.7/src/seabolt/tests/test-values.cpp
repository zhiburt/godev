/*
 * Copyright (c) 2002-2019 "Neo4j,"
 * Neo4j Sweden AB [http://neo4j.com]
 *
 * This file is part of Neo4j.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cmath>
#include "integration.hpp"
#include "catch.hpp"

using Catch::Matchers::Equals;

#define REQUIRE_BOLT_NULL(value) { REQUIRE(BoltValue_type(value) == BOLT_NULL); }
#define REQUIRE_BOLT_BOOLEAN(value, x) { REQUIRE(BoltValue_type(value) == BOLT_BOOLEAN); REQUIRE(BoltBoolean_get(value) == (x)); }
#define REQUIRE_BOLT_INTEGER(value, x) { REQUIRE(BoltValue_type(value) == BOLT_INTEGER); REQUIRE(BoltInteger_get(value) == (x)); }
#define REQUIRE_BOLT_FLOAT(value, x) { REQUIRE(BoltValue_type(value) == BOLT_FLOAT); REQUIRE( BoltFloat_get(value) == (x)); }
#define REQUIRE_BOLT_STRING(value, x, size_) { REQUIRE(BoltValue_type(value) == BOLT_STRING); REQUIRE(strncmp(BoltString_get(value), x, size_) == 0); REQUIRE(BoltValue_size(value) == (size_)); }
#define REQUIRE_BOLT_DICTIONARY(value, size_) { REQUIRE(BoltValue_type(value) == BOLT_DICTIONARY); REQUIRE(BoltValue_size(value) == (size_)); }
#define REQUIRE_BOLT_LIST(value, size_) { REQUIRE(BoltValue_type(value) == BOLT_LIST); REQUIRE(BoltValue_size(value) == (size_)); }
#define REQUIRE_BOLT_BYTES(value, size_) { REQUIRE(BoltValue_type(value) == BOLT_BYTES); REQUIRE(BoltValue_size(value) == (size_)); }
#define REQUIRE_BOLT_STRUCTURE(value, code, size_) { REQUIRE(BoltValue_type(value) == BOLT_STRUCTURE); REQUIRE(BoltStructure_code(value) == (code)); REQUIRE(BoltValue_size(value) == (size_)); }
#define REQUIRE_BOLT_SUCCESS(connection) { REQUIRE(BoltConnection_summary_success(connection) == 1); }

#define RUN_PULL_SEND(connection, result)\
    BoltConnection_load_run_request(connection);\
    BoltConnection_load_pull_request(connection, -1);\
    BoltConnection_send(connection);\
    BoltRequest (result) = BoltConnection_last_request(connection);

SCENARIO("Test null parameter", "[integration][ipv6][secure]")
{
    GIVEN("an open and initialised connection") {
        struct BoltConnection* connection = bolt_open_init_default();
        WHEN("successfully executed Cypher") {
            const char* cypher = "RETURN $x";
            BoltConnection_set_run_cypher(connection, cypher, strlen(cypher), 1);
            BoltValue* x = BoltConnection_set_run_cypher_parameter(connection, 0, "x", 1);
            BoltValue_format_as_Null(x);
            RUN_PULL_SEND(connection, result);
            while (BoltConnection_fetch(connection, result)) {
                const struct BoltValue* field_values = BoltConnection_field_values(connection);
                struct BoltValue* value = BoltList_value(field_values, 0);
                REQUIRE_BOLT_NULL(value);
            }
            REQUIRE_BOLT_SUCCESS(connection);
        }
        bolt_close_and_destroy_b(connection);
    }
}

SCENARIO("Test boolean in boolean out", "[integration][ipv6][secure]")
{
    GIVEN("an open and initialised connection") {
        struct BoltConnection* connection = bolt_open_init_default();
        WHEN("successfully executed Cypher") {
            const char* cypher = "RETURN $x";
            BoltConnection_set_run_cypher(connection, cypher, strlen(cypher), 1);
            BoltValue* x = BoltConnection_set_run_cypher_parameter(connection, 0, "x", 1);
            BoltValue_format_as_Boolean(x, 1);
            RUN_PULL_SEND(connection, result);
            while (BoltConnection_fetch(connection, result)) {
                const struct BoltValue* field_values = BoltConnection_field_values(connection);
                struct BoltValue* value = BoltList_value(field_values, 0);
                REQUIRE_BOLT_BOOLEAN(value, 1);
            }
            REQUIRE_BOLT_SUCCESS(connection);
        }
        bolt_close_and_destroy_b(connection);
    }
}

SCENARIO("Test bytes in bytes out", "[integration][ipv6][secure]")
{
    GIVEN("an open and initialised connection") {
        struct BoltConnection* connection = bolt_open_init_default();
        WHEN("successfully executed Cypher") {
            const char* cypher = "RETURN $x";
            BoltConnection_set_run_cypher(connection, cypher, strlen(cypher), 1);
            BoltValue* x = BoltConnection_set_run_cypher_parameter(connection, 0, "x", 1);
            char array[5] = {33, 44, 55, 66, 77};
            BoltValue_format_as_Bytes(x, &array[0], sizeof(array));
            RUN_PULL_SEND(connection, result);
            while (BoltConnection_fetch(connection, result)) {
                const struct BoltValue* field_values = BoltConnection_field_values(connection);
                struct BoltValue* value = BoltList_value(field_values, 0);
                REQUIRE_BOLT_BYTES(value, 5);
            }
            REQUIRE_BOLT_SUCCESS(connection);
        }
        bolt_close_and_destroy_b(connection);
    }
}

SCENARIO("Test string in string out", "[integration][ipv6][secure]")
{
    GIVEN("an open and initialised connection") {
        struct BoltConnection* connection = bolt_open_init_default();
        WHEN("successfully executed Cypher") {
            const char* cypher = "RETURN $x";
            BoltConnection_set_run_cypher(connection, cypher, strlen(cypher), 1);
            BoltValue* x = BoltConnection_set_run_cypher_parameter(connection, 0, "x", 1);
            BoltValue_format_as_String(x, "hello, world", 12);
            RUN_PULL_SEND(connection, result);
            while (BoltConnection_fetch(connection, result)) {
                const struct BoltValue* field_values = BoltConnection_field_values(connection);
                struct BoltValue* value = BoltList_value(field_values, 0);
                REQUIRE_BOLT_STRING(value, "hello, world", 12);
            }
            REQUIRE_BOLT_SUCCESS(connection);
        }
        bolt_close_and_destroy_b(connection);
    }
}

SCENARIO("Test list in list out", "[integration][ipv6][secure]")
{
    GIVEN("an open and initialised connection") {
        struct BoltConnection* connection = bolt_open_init_default();
        WHEN("successfully executed Cypher") {
            const char* cypher = "RETURN $x";
            REQUIRE(BoltConnection_set_run_cypher(connection, cypher, strlen(cypher), 1)==0);
            BoltValue* x = BoltConnection_set_run_cypher_parameter(connection, 0, "x", 1);
            BoltValue_format_as_List(x, 3);
            BoltValue_format_as_Integer(BoltList_value(x, 0), 0);
            BoltValue_format_as_Integer(BoltList_value(x, 1), 1);
            BoltValue_format_as_Integer(BoltList_value(x, 2), 2);
            RUN_PULL_SEND(connection, result);
            while (BoltConnection_fetch(connection, result)) {
                BoltValue* field_values = BoltConnection_field_values(connection);
                BoltValue* value = BoltList_value(field_values, 0);
                REQUIRE_BOLT_LIST(value, 3);
                REQUIRE(BoltInteger_get(BoltList_value(value, 0))==0);
                REQUIRE(BoltInteger_get(BoltList_value(value, 1))==1);
                REQUIRE(BoltInteger_get(BoltList_value(value, 2))==2);
            }
            REQUIRE_BOLT_SUCCESS(connection);
        }
        bolt_close_and_destroy_b(connection);
    }
}

SCENARIO("Test empty list in empty list out", "[integration][ipv6][secure]")
{
    GIVEN("an open and initialised connection") {
        struct BoltConnection* connection = bolt_open_init_default();
        WHEN("successfully executed Cypher") {
            const char* cypher = "RETURN $x";
            REQUIRE(BoltConnection_set_run_cypher(connection, cypher, strlen(cypher), 1)==0);
            BoltValue* x = BoltConnection_set_run_cypher_parameter(connection, 0, "x", 1);
            BoltValue_format_as_List(x, 0);
            RUN_PULL_SEND(connection, result);
            while (BoltConnection_fetch(connection, result)) {
                BoltValue* field_values = BoltConnection_field_values(connection);
                BoltValue* value = BoltList_value(field_values, 0);
                REQUIRE_BOLT_LIST(value, 0);
            }
            REQUIRE_BOLT_SUCCESS(connection);
        }
        bolt_close_and_destroy_b(connection);
    }
}

SCENARIO("Test mixed list in mixed list out", "[integration][ipv6][secure]")
{
    GIVEN("an open and initialised connection") {
        struct BoltConnection* connection = bolt_open_init_default();
        WHEN("successfully executed Cypher") {
            const char* cypher = "RETURN $x";
            REQUIRE(BoltConnection_set_run_cypher(connection, cypher, strlen(cypher), 1)==0);
            BoltValue* x = BoltConnection_set_run_cypher_parameter(connection, 0, "x", 1);

            // list [42, "hello", false, 42.4242, {key1: "value1", key2: -424242}]
            BoltValue_format_as_List(x, 5);
            BoltValue_format_as_Integer(BoltList_value(x, 0), 42);
            BoltValue_format_as_String(BoltList_value(x, 1), "hello", 5);
            BoltValue_format_as_Boolean(BoltList_value(x, 2), 0);
            BoltValue_format_as_Float(BoltList_value(x, 3), 42.4242);

            BoltValue_format_as_Dictionary(BoltList_value(x, 4), 2);
            BoltDictionary_set_key(BoltList_value(x, 4), 0, "key1", 4);
            BoltValue_format_as_String(BoltDictionary_value(BoltList_value(x, 4), 0), "value1", 6);
            BoltDictionary_set_key(BoltList_value(x, 4), 1, "key2", 4);
            BoltValue_format_as_Integer(BoltDictionary_value(BoltList_value(x, 4), 1), -424242);

            RUN_PULL_SEND(connection, result);
            while (BoltConnection_fetch(connection, result)) {
                BoltValue* field_values = BoltConnection_field_values(connection);
                BoltValue* value = BoltList_value(field_values, 0);
                REQUIRE_BOLT_LIST(value, 5);
                REQUIRE(BoltInteger_get(BoltList_value(value, 0))==42);
                CHECK_THAT(BoltString_get(BoltList_value(value, 1)), Equals("hello"));
                REQUIRE(BoltBoolean_get(BoltList_value(value, 2))==0);
                REQUIRE(BoltFloat_get(BoltList_value(value, 3))==42.4242);

                BoltValue* dictionary = BoltList_value(value, 4);
                REQUIRE_BOLT_DICTIONARY(dictionary, 2);
                CHECK_THAT(BoltString_get(BoltDictionary_value_by_key(dictionary, "key1", 4)), Equals("value1"));
                REQUIRE(BoltInteger_get(BoltDictionary_value_by_key(dictionary, "key2", 4))==-424242);
            }
            REQUIRE_BOLT_SUCCESS(connection);
        }
        bolt_close_and_destroy_b(connection);
    }
}

SCENARIO("Test dictionary in dictionary out", "[integration][ipv6][secure]")
{
    GIVEN("an open and initialised connection") {
        struct BoltConnection* connection = bolt_open_init_default();
        WHEN("successfully executed Cypher") {
            const char* cypher = "RETURN $x";
            BoltConnection_set_run_cypher(connection, cypher, strlen(cypher), 1);
            BoltValue* x = BoltConnection_set_run_cypher_parameter(connection, 0, "x", 1);
            BoltValue_format_as_Dictionary(x, 2);
            BoltDictionary_set_key(x, 0, "name", 4);
            BoltValue_format_as_String(BoltDictionary_value(x, 0), "Alice", 5);
            BoltDictionary_set_key(x, 1, "age", 3);
            BoltValue_format_as_Integer(BoltDictionary_value(x, 1), 33);
            RUN_PULL_SEND(connection, result);
            while (BoltConnection_fetch(connection, result)) {
                const struct BoltValue* field_values = BoltConnection_field_values(connection);
                struct BoltValue* dict = BoltList_value(field_values, 0);
                REQUIRE_BOLT_DICTIONARY(dict, 2);
                int found = 0;
                for (int i = 0; i<dict->size; i++) {
                    const char* key = BoltDictionary_get_key(dict, i);
                    if (strcmp(key, "name")==0) {
                        REQUIRE_BOLT_STRING(BoltDictionary_value(dict, i), "Alice", 5);
                        found += 1;
                    }
                    else if (strcmp(key, "age")==0) {
                        REQUIRE_BOLT_INTEGER(BoltDictionary_value(dict, i), 33);
                        found += 1;
                    }
                    else {
                        FAIL();
                    }
                }
                REQUIRE(found==2);
            }
            REQUIRE_BOLT_SUCCESS(connection);
        }
        bolt_close_and_destroy_b(connection);
    }
}

SCENARIO("Test empty dictionary in empty dictionary out", "[integration][ipv6][secure]")
{
    GIVEN("an open and initialised connection") {
        struct BoltConnection* connection = bolt_open_init_default();
        WHEN("successfully executed Cypher") {
            const char* cypher = "RETURN $x";
            BoltConnection_set_run_cypher(connection, cypher, strlen(cypher), 1);
            BoltValue* x = BoltConnection_set_run_cypher_parameter(connection, 0, "x", 1);
            BoltValue_format_as_Dictionary(x, 0);
            RUN_PULL_SEND(connection, result);
            while (BoltConnection_fetch(connection, result)) {
                const struct BoltValue* field_values = BoltConnection_field_values(connection);
                struct BoltValue* value = BoltList_value(field_values, 0);
                REQUIRE_BOLT_DICTIONARY(value, 0);
            }
            REQUIRE_BOLT_SUCCESS(connection);
        }
        bolt_close_and_destroy_b(connection);
    }
}

SCENARIO("Test mixed dictionary in mixed dictionary out", "[integration][ipv6][secure]")
{
    GIVEN("an open and initialised connection") {
        struct BoltConnection* connection = bolt_open_init_default();
        WHEN("successfully executed Cypher") {
            const char* cypher = "RETURN $x";
            BoltConnection_set_run_cypher(connection, cypher, strlen(cypher), 1);
            BoltValue* x = BoltConnection_set_run_cypher_parameter(connection, 0, "x", 1);

            // dictionary {k1: "apa", key2: [1.9283, "hello world!"], TheKey3: true}
            BoltValue_format_as_Dictionary(x, 3);

            BoltDictionary_set_key(x, 0, "k1", 2);
            BoltValue_format_as_String(BoltDictionary_value(x, 0), "apa", 3);

            BoltDictionary_set_key(x, 1, "key2", 4);
            BoltValue_format_as_List(BoltDictionary_value(x, 1), 2);
            BoltValue_format_as_Float(BoltList_value(BoltDictionary_value(x, 1), 0), 1.9283);
            BoltValue_format_as_String(BoltList_value(BoltDictionary_value(x, 1), 1), "hello world!", 12);

            BoltDictionary_set_key(x, 2, "TheKey3", 7);
            BoltValue_format_as_Boolean(BoltDictionary_value(x, 2), 1);

            RUN_PULL_SEND(connection, result);
            while (BoltConnection_fetch(connection, result)) {
                const struct BoltValue* field_values = BoltConnection_field_values(connection);
                struct BoltValue* value = BoltList_value(field_values, 0);
                REQUIRE_BOLT_DICTIONARY(value, 3);
                CHECK_THAT(BoltString_get(BoltDictionary_value_by_key(value, "k1", 2)), Equals("apa"));

                BoltValue* list = BoltDictionary_value_by_key(value, "key2", 4);
                REQUIRE_BOLT_LIST(list, 2);
                REQUIRE(BoltFloat_get(BoltList_value(list, 0))==1.9283);
                CHECK_THAT(BoltString_get(BoltList_value(list, 1)), Equals("hello world!"));

                REQUIRE_BOLT_BOOLEAN(BoltDictionary_value_by_key(value, "TheKey3", 7), 1);
            }
            REQUIRE_BOLT_SUCCESS(connection);
        }
        bolt_close_and_destroy_b(connection);
    }
}

SCENARIO("Test integer in integer out", "[integration][ipv6][secure]")
{
    GIVEN("an open and initialised connection") {
        struct BoltConnection* connection = bolt_open_init_default();
        WHEN("successfully executed Cypher") {
            const char* cypher = "RETURN $x";
            BoltConnection_set_run_cypher(connection, cypher, strlen(cypher), 1);
            BoltValue* x = BoltConnection_set_run_cypher_parameter(connection, 0, "x", 1);
            BoltValue_format_as_Integer(x, 123456789);
            RUN_PULL_SEND(connection, result);
            while (BoltConnection_fetch(connection, result)) {
                const struct BoltValue* field_values = BoltConnection_field_values(connection);
                struct BoltValue* value = BoltList_value(field_values, 0);
                REQUIRE_BOLT_INTEGER(value, 123456789);
            }
            REQUIRE_BOLT_SUCCESS(connection);
        }
        bolt_close_and_destroy_b(connection);
    }
}

SCENARIO("Test max & min integer in max & min integer out", "[integration][ipv6][secure]")
{
    GIVEN("an open and initialised connection") {
        struct BoltConnection* connection = bolt_open_init_default();
        WHEN("successfully executed Cypher") {
            const char* cypher = "RETURN $x";
            BoltConnection_set_run_cypher(connection, cypher, strlen(cypher), 1);
            BoltValue* x = BoltConnection_set_run_cypher_parameter(connection, 0, "x", 1);

            BoltValue_format_as_Integer(x, INT64_MAX);
            RUN_PULL_SEND(connection, result1);
            while (BoltConnection_fetch(connection, result1)) {
                const struct BoltValue* field_values = BoltConnection_field_values(connection);
                struct BoltValue* value = BoltList_value(field_values, 0);
                REQUIRE_BOLT_INTEGER(value, INT64_MAX);
            }

            BoltValue_format_as_Integer(x, INT64_MIN);
            RUN_PULL_SEND(connection, result2);
            while (BoltConnection_fetch(connection, result2)) {
                const struct BoltValue* field_values = BoltConnection_field_values(connection);
                struct BoltValue* value = BoltList_value(field_values, 0);
                REQUIRE_BOLT_INTEGER(value, INT64_MIN);
            }

            REQUIRE_BOLT_SUCCESS(connection);
        }
        bolt_close_and_destroy_b(connection);
    }
}

SCENARIO("Test float in float out", "[integration][ipv6][secure]")
{
    GIVEN("an open and initialised connection") {
        struct BoltConnection* connection = bolt_open_init_default();
        WHEN("successfully executed Cypher") {
            const char* cypher = "RETURN $x";
            BoltConnection_set_run_cypher(connection, cypher, strlen(cypher), 1);
            BoltValue* x = BoltConnection_set_run_cypher_parameter(connection, 0, "x", 1);
            BoltValue_format_as_Float(x, 6.283185307179);
            RUN_PULL_SEND(connection, result);
            while (BoltConnection_fetch(connection, result)) {
                const struct BoltValue* field_values = BoltConnection_field_values(connection);
                struct BoltValue* value = BoltList_value(field_values, 0);
                REQUIRE_BOLT_FLOAT(value, 6.283185307179);
            }
            REQUIRE_BOLT_SUCCESS(connection);
        }
        bolt_close_and_destroy_b(connection);
    }
}

SCENARIO("Test max & min float in max & min float out", "[integration][ipv6][secure]")
{
    GIVEN("an open and initialised connection") {
        struct BoltConnection* connection = bolt_open_init_default();
        WHEN("successfully executed Cypher") {
            const char* cypher = "RETURN $x";
            BoltConnection_set_run_cypher(connection, cypher, strlen(cypher), 1);
            BoltValue* x = BoltConnection_set_run_cypher_parameter(connection, 0, "x", 1);

            BoltValue_format_as_Float(x, 1.7976931348623157E308);
            RUN_PULL_SEND(connection, result1);
            while (BoltConnection_fetch(connection, result1)) {
                const struct BoltValue* field_values = BoltConnection_field_values(connection);
                struct BoltValue* value = BoltList_value(field_values, 0);
                REQUIRE_BOLT_FLOAT(value, 1.7976931348623157E308);
            }

            BoltValue_format_as_Float(x, 4.9E-324);
            RUN_PULL_SEND(connection, result2);
            while (BoltConnection_fetch(connection, result2)) {
                const struct BoltValue* field_values = BoltConnection_field_values(connection);
                struct BoltValue* value = BoltList_value(field_values, 0);
                REQUIRE_BOLT_FLOAT(value, 4.9E-324);
            }

            REQUIRE_BOLT_SUCCESS(connection);
        }
        bolt_close_and_destroy_b(connection);
    }
}

SCENARIO("Test structure in result", "[integration][ipv6][secure]")
{
    GIVEN("an open and initialised connection") {
        struct BoltConnection* connection = bolt_open_init_default();
        WHEN("successfully executed Cypher") {
            BoltConnection_load_begin_request(connection);
            const char* cypher = "CREATE (a:Person {name:'Alice'}) RETURN a";
            BoltConnection_set_run_cypher(connection, cypher, strlen(cypher), 0);
            BoltConnection_load_run_request(connection);
            BoltConnection_load_pull_request(connection, -1);
            BoltRequest result = BoltConnection_last_request(connection);
            BoltConnection_load_rollback_request(connection);
            BoltConnection_send(connection);
            BoltRequest last = BoltConnection_last_request(connection);
            while (BoltConnection_fetch(connection, result)) {
                const struct BoltValue* field_values = BoltConnection_field_values(connection);
                struct BoltValue* node = BoltList_value(field_values, 0);
                REQUIRE_BOLT_STRUCTURE(node, 'N', 3);
                BoltValue* id = BoltStructure_value(node, 0);
                BoltValue* labels = BoltStructure_value(node, 1);
                BoltValue* properties = BoltStructure_value(node, 2);
                REQUIRE(BoltValue_type(id)==BOLT_INTEGER);
                REQUIRE_BOLT_LIST(labels, 1);
                REQUIRE_BOLT_STRING(BoltList_value(labels, 0), "Person", 6);
                REQUIRE_BOLT_DICTIONARY(properties, 1);
                REQUIRE(strcmp(BoltDictionary_get_key(properties, 0), "name")==0);
                REQUIRE_BOLT_STRING(BoltDictionary_value(properties, 0), "Alice", 5);
            }
            BoltConnection_fetch_summary(connection, last);
            REQUIRE_BOLT_SUCCESS(connection);
        }
        bolt_close_and_destroy_b(connection);
    }
}

TEST_CASE("BoltValue")
{
    SECTION("BoltValue_to_string") {
        // assume an un-initialized buffer
        const int buffer_size = 128000;
        char buffer[buffer_size+1];
        for (int i = 0; i<buffer_size; i++) {
            buffer[i] = 'a';
        }

        BoltValue* value = BoltValue_create();

        SECTION("Null") {
            BoltValue_format_as_Null(value);

            REQUIRE(BoltValue_to_string(value, buffer, buffer_size, nullptr)==4);
            REQUIRE(strlen(buffer)==4);
            REQUIRE(strcmp(buffer, "null")==0);
        }

        SECTION("Boolean") {
            SECTION("True") {
                BoltValue_format_as_Boolean(value, 1);

                REQUIRE(BoltValue_to_string(value, buffer, buffer_size, nullptr)==4);
                REQUIRE(strlen(buffer)==4);
                REQUIRE(strcmp(buffer, "true")==0);
            }

            SECTION("False") {
                BoltValue_format_as_Boolean(value, 0);

                REQUIRE(BoltValue_to_string(value, buffer, buffer_size, nullptr)==5);
                REQUIRE(strlen(buffer)==5);
                REQUIRE(strcmp(buffer, "false")==0);
            }
        }

        SECTION("Integer") {
            SECTION("-9223372036854775807") {
                BoltValue_format_as_Integer(value, -9223372036854775807);

                REQUIRE(BoltValue_to_string(value, buffer, buffer_size, nullptr)==20);
                REQUIRE(strlen(buffer)==20);
                REQUIRE(strcmp(buffer, "-9223372036854775807")==0);
            }

            SECTION("-100050") {
                BoltValue_format_as_Integer(value, -100050);

                REQUIRE(BoltValue_to_string(value, buffer, buffer_size, nullptr)==7);
                REQUIRE(strlen(buffer)==7);
                REQUIRE(strcmp(buffer, "-100050")==0);
            }

            SECTION("0") {
                BoltValue_format_as_Integer(value, 0);

                REQUIRE(BoltValue_to_string(value, buffer, buffer_size, nullptr)==1);
                REQUIRE(strlen(buffer)==1);
                REQUIRE(strcmp(buffer, "0")==0);
            }

            SECTION("100") {
                BoltValue_format_as_Integer(value, 100);

                REQUIRE(BoltValue_to_string(value, buffer, buffer_size, nullptr)==3);
                REQUIRE(strlen(buffer)==3);
                REQUIRE(strcmp(buffer, "100")==0);
            }

            SECTION("9223372036854775807") {
                BoltValue_format_as_Integer(value, 9223372036854775807);

                REQUIRE(BoltValue_to_string(value, buffer, buffer_size, nullptr)==19);
                REQUIRE(strlen(buffer)==19);
                REQUIRE(strcmp(buffer, "9223372036854775807")==0);
            }
        }

        SECTION("Float") {
            SECTION("-100513.14574789") {
                BoltValue_format_as_Float(value, -100513.14574789);

                REQUIRE(BoltValue_to_string(value, buffer, buffer_size, nullptr)==14);
                REQUIRE(strlen(buffer)==14);
                REQUIRE(strcmp(buffer, "-100513.145748")==0);
            }

            SECTION("0") {
                BoltValue_format_as_Float(value, 0);

                REQUIRE(BoltValue_to_string(value, buffer, buffer_size, nullptr)==8);
                REQUIRE(strlen(buffer)==8);
                REQUIRE(strcmp(buffer, "0.000000")==0);
            }

            SECTION("3.14") {
                BoltValue_format_as_Float(value, 3.14);

                REQUIRE(BoltValue_to_string(value, buffer, buffer_size, nullptr)==8);
                REQUIRE(strlen(buffer)==8);
                REQUIRE(strcmp(buffer, "3.140000")==0);
            }

            SECTION("100513.145747") {
                BoltValue_format_as_Float(value, 100513.145747);

                REQUIRE(BoltValue_to_string(value, buffer, buffer_size, nullptr)==13);
                REQUIRE(strlen(buffer)==13);
                REQUIRE(strcmp(buffer, "100513.145747")==0);
            }

            SECTION("100513.14574789") {
                BoltValue_format_as_Float(value, 100513.14574789);

                REQUIRE(BoltValue_to_string(value, buffer, buffer_size, nullptr)==13);
                REQUIRE(strlen(buffer)==13);
                REQUIRE(strcmp(buffer, "100513.145748")==0);
            }
        }

        SECTION("String") {
            SECTION("null") {
                BoltValue_format_as_String(value, nullptr, 0);

                REQUIRE(BoltValue_to_string(value, buffer, buffer_size, nullptr)==0);
                REQUIRE(strlen(buffer)==0);
                REQUIRE(strcmp(buffer, "")==0);
            }

            SECTION("empty string") {
                BoltValue_format_as_String(value, "", 0);

                REQUIRE(BoltValue_to_string(value, buffer, buffer_size, nullptr)==0);
                REQUIRE(strlen(buffer)==0);
                REQUIRE(strcmp(buffer, "")==0);
            }

            SECTION("abcdefg hijkl") {
                BoltValue_format_as_String(value, "abcdefg hijkl", 13);

                REQUIRE(BoltValue_to_string(value, buffer, buffer_size, nullptr)==13);
                REQUIRE(strlen(buffer)==13);
                REQUIRE(strcmp(buffer, "abcdefg hijkl")==0);
            }
        }

        SECTION("Bytes") {
            SECTION("null") {
                BoltValue_format_as_Bytes(value, nullptr, 0);

                REQUIRE(BoltValue_to_string(value, buffer, buffer_size, nullptr)==1);
                REQUIRE(strlen(buffer)==1);
                REQUIRE(strcmp(buffer, "#")==0);
            }

            SECTION("empty array") {
                char empty[1];
                BoltValue_format_as_Bytes(value, empty, 0);

                REQUIRE(BoltValue_to_string(value, buffer, buffer_size, nullptr)==1);
                REQUIRE(strlen(buffer)==1);
                REQUIRE(strcmp(buffer, "#")==0);
            }

            SECTION("byte sequence") {
                char empty[11] = "abcdefghij";
                BoltValue_format_as_Bytes(value, empty, 10);

                REQUIRE(BoltValue_to_string(value, buffer, buffer_size, nullptr)==21);
                REQUIRE(strlen(buffer)==21);
                REQUIRE(strcmp(buffer, "#6162636465666768696A")==0);
            }
        }

        SECTION("List") {
            SECTION("empty list") {
                BoltValue_format_as_List(value, 0);

                REQUIRE(BoltValue_to_string(value, buffer, buffer_size, nullptr)==2);
                REQUIRE(strlen(buffer)==2);
                REQUIRE(strcmp(buffer, "[]")==0);
            }

            SECTION("a list of basic types") {
                BoltValue_format_as_List(value, 5);
                BoltValue_format_as_Null(BoltList_value(value, 0));
                BoltValue_format_as_Boolean(BoltList_value(value, 1), 1);
                BoltValue_format_as_Integer(BoltList_value(value, 2), 2467);
                BoltValue_format_as_Float(BoltList_value(value, 3), 3.14);
                BoltValue_format_as_String(BoltList_value(value, 4), "test string", 11);

                REQUIRE(BoltValue_to_string(value, buffer, buffer_size, nullptr)==41);
                REQUIRE(strlen(buffer)==41);
                REQUIRE(strcmp(buffer, "[null, true, 2467, 3.140000, test string]")==0);
            }
        }

        SECTION("Dictionary") {
            SECTION("empty dictionary") {
                BoltValue_format_as_Dictionary(value, 0);

                REQUIRE(BoltValue_to_string(value, buffer, buffer_size, nullptr)==2);
                REQUIRE(strlen(buffer)==2);
                REQUIRE(strcmp(buffer, "{}")==0);
            }

            SECTION("a dictionary of basic types") {
                BoltValue_format_as_Dictionary(value, 5);
                BoltDictionary_set_key(value, 0, "a", 1);
                BoltValue_format_as_Null(BoltDictionary_value(value, 0));
                BoltDictionary_set_key(value, 1, "b", 1);
                BoltValue_format_as_Boolean(BoltDictionary_value(value, 1), 1);
                BoltDictionary_set_key(value, 2, "c", 1);
                BoltValue_format_as_Integer(BoltDictionary_value(value, 2), 2467);
                BoltDictionary_set_key(value, 3, "d", 1);
                BoltValue_format_as_Float(BoltDictionary_value(value, 3), 3.14);
                BoltDictionary_set_key(value, 4, "e", 1);
                BoltValue_format_as_String(BoltDictionary_value(value, 4), "test string", 11);

                REQUIRE(BoltValue_to_string(value, buffer, buffer_size, nullptr)==56);
                REQUIRE(strlen(buffer)==56);
                REQUIRE(strcmp(buffer, "{a: null, b: true, c: 2467, d: 3.140000, e: test string}")==0);
            }
        }

        SECTION("Structure") {
            SECTION("code only") {
                BoltValue_format_as_Structure(value, 65, 0);

                REQUIRE(BoltValue_to_string(value, buffer, buffer_size, nullptr)==8);
                REQUIRE(strlen(buffer)==8);
                REQUIRE(strcmp(buffer, "$#0041()")==0);
            }

            SECTION("with simple fields") {
                BoltValue_format_as_Structure(value, 95, 5);
                BoltValue_format_as_Null(BoltStructure_value(value, 0));
                BoltValue_format_as_Boolean(BoltStructure_value(value, 1), 0);
                BoltValue_format_as_Integer(BoltStructure_value(value, 2), 2467);
                BoltValue_format_as_Float(BoltStructure_value(value, 3), 3.14);
                BoltValue_format_as_String(BoltStructure_value(value, 4), "test string", 11);

                REQUIRE(BoltValue_to_string(value, buffer, buffer_size, nullptr)==48);
                REQUIRE(strlen(buffer)==48);
                REQUIRE(strcmp(buffer, "$#005F(null, false, 2467, 3.140000, test string)")==0);
            }
        }

        BoltValue_destroy(value);
    }
}