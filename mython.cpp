#include "object.h"
#include "object_holder.h"
#include "statement.h"
#include "lexer.h"
#include "parse.h"

#include "test_runner.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

void TestAll();

void RunMythonProgram(istream& input, ostream& output) {
  Ast::Print::SetOutputStream(output);

  Parse::Lexer lexer(input);
  auto program = ParseProgram(lexer);

  Runtime::Closure closure;
  program->Execute(closure);
}

int main() {
    TestAll();

    try {
        RunMythonProgram(cin, cout);
    }
    catch (exception& ex) {
        cerr << ex.what();
    }
    catch (ObjectHolder& holder) {
        cerr << "Returned value was not catched";
    }
    catch (...) {
        cerr << "Unknown exception";
    }
    return 0;
}

void TestSimplePrints() {
  istringstream input(R"(
print 57
print 10, 24, -8
print 'hello'
print "world"
print True, False
print
print None
)");

  ostringstream output;
  RunMythonProgram(input, output);

  ASSERT_EQUAL(output.str(), "57\n10 24 -8\nhello\nworld\nTrue False\n\nNone\n");
}

void TestAssignments() {
  istringstream input(R"(
x = 57
print x
x = 'C++ black belt'
print x
y = False
x = y
print x
x = None
print x, y
)");

  ostringstream output;
  RunMythonProgram(input, output);

  ASSERT_EQUAL(output.str(), "57\nC++ black belt\nFalse\nNone False\n");
}

void TestArithmetics() {
  istringstream input(
    "print 1+2+3+4+5, 1*2*3*4*5, 1-2-3-4-5, 36/4/3, 2*5+10/2"
  );

  ostringstream output;
  RunMythonProgram(input, output);

  ASSERT_EQUAL(output.str(), "15 120 -13 3 15\n");
}

void TestVariablesArePointers() {
  istringstream input(R"(
class Counter:
  def __init__():
    self.value = 0

  def add():
    self.value = self.value + 1

class Dummy:
  def do_add(counter):
    counter.add()

x = Counter()
y = x

x.add()
y.add()

print x.value

d = Dummy()
d.do_add(x)

print y.value
)");

  ostringstream output;
  RunMythonProgram(input, output);

  ASSERT_EQUAL(output.str(), "2\n3\n");
}

void TestAll() {
    TestRunner tr;
    cerr << "LEXER" << endl;
    Parse::RunLexerTests(tr);
    cerr << "OBJECT HOLDER" << endl;
    Runtime::RunObjectHolderTests(tr);
    cerr << "OBJECTS" << endl;
    Runtime::RunObjectsTests(tr);
    cerr << "PROGRAM INTERPRETED TESTS" << endl;
    RUN_TEST(tr, TestSimplePrints);
    RUN_TEST(tr, TestAssignments);
    RUN_TEST(tr, TestArithmetics);
    RUN_TEST(tr, TestVariablesArePointers);
    cerr << "STATEMENT UNIT TESTS" << endl;
    Ast::RunUnitTests(tr);
    cerr << "PARSE PROGRAM" << endl;
    TestParseProgram(tr);
}
