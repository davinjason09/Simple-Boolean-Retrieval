# Simple Boolean Retrieval Model

A simple CLI-based Boolean Retrieval Model that processes boolean, positional and phrase queries using C++ and Python. This model is capable of handling queries with the following operators:
- AND / ``&``
- OR / ``|`` / ``/``
- NOT / ``~``
- ( )

This code supports the following query types:
```plaintext
w1
w1 AND w2
w1 OR w2
NOT w1
w1 AND/OR NOT w2
w1 AND/OR (w2 AND/OR w3)
```

and any combination of the above query types. This implementation currently doesn't support phrase queries.

You can combine the operators from the available operators above. Currently, the Python implementation only supports `AND`, `OR`, and `NOT` operators, and not their symbol counterparts.

The operators are evaluated in the following order:
```plaintext
() > NOT > AND > OR
```

## Usage

### C++ Implementation
You can run the C++ implementation using the following commands:
```bash
g++ -O3 -mtune=native -march=native BooleanRetrieval.cpp InvertedIndex.cpp Main.cpp -o Main -pthread
./Main  # Linux
./Main.exe  # Windows
```

or you can use the CMake build system:
```bash
soon
```

### Python Implementation
You can use the [ipynb](https://github.com/davinjason09/Simple-Boolean-Retrieval/blob/main/python/Boolean_Retrieval.ipynb) file to run the Python implementation.

## Speed Comparison

### Inverted Index Construction:
| C++ | Python |
| -------------- | --------------- |
| 2.015 s | 20.971 s |

### Query Retrieval:
Query retrieval times are dependent on the query and the number of documents in the collection. The C++ implementation is much more stable than the Python implementation, averaging around 65 ms per query.
