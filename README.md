# csvf
*csvf* is a C++ library for csv file (especially large csv file) processing. Functionalities such as csv parsing, reading, writing, splitting, combining etc, are implemented as C++ classes, functions, and standalone executable programs. Various csv format can be automatically detected and parsed, including different separator, end of line, quote rules etc.

## Installation
1. Prerequisites
   * CMake
   * C++ compiler
   * Boost
2. Download source code
   ```bash
   git clone https://github.com/gongliyu/csvf
   ```
3. Build and install
   ```bash
   mkdir csvf-build
   cd csvf-build
   cmake -DCMAKE_INSTALL_PREFIX=/path/to/install /path/to/csvf/source
   make
   make install
   ```

## Usage

### Parse csv file

``` C++
#include <iostream>
#include <iomanip>
#include <csvf/csvf.hpp>

int main(int argn, const char** argv)
{
    csvf::reader reader("abc.csv");
    while (reader) {
        auto record = reader.read_record();
        for (auto& field : record) {
            std::cout<<field<<"\t";
        }
        std::cout<<std::endl;
    }
}
```
The member function of  *csvf::reader::read_record()* will return a record as  *std::vector<std::string_view>* (if C++ compiler supports) or *std::vector<boost::string_view>*.

### Write csv file

``` C++
#include <iostream>
#include <iomanip>
#include <csvf/csvf.hpp>

int main(int argn, const char** argv)
{
    csvf::reader writer("abc.csv");
    std::vector<std::string> record1{"hello", "hi", "nihao"};
    std::list<std::string> record2{"1","2","3"};
    writer.write_record(record1).write_record(record2);
    writer.close();
}
```
The member function *csvf::writer::write_record* could accept any container with element type *std::string* or *std::string_view*.

### Split csv file

``` C++
std::string infname("abc.csv");
std::vector<std::string> outfnames{"abc1.csv", "abc2.csv", "abc3.csv"};
csvf::split_by_record(infname, outfnames);
```
will split *abc.csv* by record, i.e. each result small csv file will contain different rows of the original file. 

On the other hand,

``` C++
csvf::split_by_field(infname, outfnames);
```
will split *abc.csv* by field, i.e. *abc1.csv*, *abc2.csv* and *abc3.csv* will contain different columns of the original file *abc.csv*.

There is also a standalone executable program *csvf_split* could be used to do the same job

``` bash
csvf_split --input abc.csv --output abc1.csv abc2.csv abc3.csv --by record
```


### Combine csv files
Just as split files, there are functions and standalone program to combine multiple csv files to a single csv file

``` C++
csvf::combine_by_record(infnames, outfname);
csvf::combine_by_field(infnames, outfname);
```

``` bash
csvf_combine --input abc1.csv abc2.csv abc3.csv --output abc.csv --by record
```

