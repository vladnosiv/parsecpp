# Parsec for C++


I have worked with Parsec library in Haskell before, and I really liked 
laconic usage, so I tried to implement some of the functionality 
of parser combinators in C++.

Here is a small example. Below is code in Haskell that allows you to parse 
sequence of letters or numbers with any number of spaces in between.
spaces in between, or empty is written instead of the sequence. With Haskell Parsec
it can be done in the following way:
```cpp
many (spaces >> alphaNum) <|> string "empty"
```

With the functionality I have implemented, the same thing can be written in C++ like this:
```cpp
many(spaces() >> alphaNum()) | prefix_parser("empty")
```
