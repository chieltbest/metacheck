# Metacheck

Metacheck is a parameterized testing suite for template metaprograms, and aims to provide similar
functionality as rapidcheck or quickcheck.

As TMP gets faster and algorithms grow in size unit testing of TMP grows in importance. The fact that the user, and in many case the author, have trouble understanding and reporting bugs also adds to the importance of good testing tools.

In order to run easily in any CI system or testing suite metacheck is implemented purely using C++ template metaprogramming. The idea is to use the time macro as a random seed and provide TMP based generators which can generate test cases much the way rapidcheck or quickcheck do. 

For example a viable test for sort would be to take random input of different lengths and compare a sort of the input to a sort of a rotated version of the input. Another sanity check would be to verify that the count of any element is the same before and after sorting or to make a fold verifying that the elements are in ascending order. 

Another interesting capability is to select random types from a list of error prone nasty types like void, int&&, pointers to members, nullptr_t etc. and run them through algorithms to verify that they came out the other side in the same form.

The output of metacheck is currently based on google test, with planned support for JUnit XML 
test reports.

## Writing tests



See the main.cpp file for example usage of metacheck.

## Todo
- improve this readme
- create more generators
- support for more command line options
- custom value output
