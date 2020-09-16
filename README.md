# Sailor-db | 세일러 DB
in memory database library
인메모리 데이터베이스 라이브러리

## How to use | 사용법

### CREATE TABLE
```cpp
#include <string>
#include "db.hpp"

class MyDatabase : sl::Database {
public:
    struct MyRecord : sl::Model {
        // CREATE TABLE        
        int id;
        std::string name;
        std::string password;

        MyRecord(std::string _name, std::string _pass) {
            // DEFAULT INITIALIZER
            static int PRIMARY_KEY = 0;
            name = _name;
            password = _pass;
            id = PRIMARY_KEY;
            PRIMARY_KEY++;
        }
    }

    sl::Table<MyRecord> myTable;
} myDatabase;
```

### INSERT
```cpp
// class MyTable's constructer runs with these parameters
myDatabase.myTable.insert(
    "some name",
    Hash("some password") // Use third party hashing library
);

myDatabase.myTable.insertRaw(
    MyDatabase::MyRecord(
        "some name",
        Hash("some password")
    )
);
```

### WHERE
```cpp
myDatabase.myTable.where([](auto e) {
    return e.name == "some name";
})
```
Where method returns new table of pointer of records in old table

### UPDATE
```cpp
myDatabase.myTable.where([](auto e) {
    return e.name == "some name";
}).first()->name = "some another name";

for (auto record : myDatabase.myTable.where([](auto e) {
    return e.name == "some name";
})) {
    record->name = "some another name";
}
```

### convert record to string
```cpp
auto record = myDatabase.myTable.first();

std::istringstream out;

sl::Datbase::write(
    out,
    record.id,
    record.name,
    record.password
);

auto result = out.str();
```

### convert string to record
```cpp
MyDatabase::MyRecord output;

sl::Datbase::read(
    inputString,
    output.id,
    output.name,
    output.password
);
```