# Project Title: Print Tokens Program Testing
Team members:
1. Shaheen Nijamudheen, 1002101057
2. Rumaysa Jafer, 1001898468
   
### Description

This project contains the implementation and JUnit test cases for the PrintTokens program. The project includes:
- right_printtokens.java: The main program file.
- RightPrinttokensMainTest.java: JUnit test cases for the main functionality.
- RightPrinttokensNonMainTest.java: JUnit test cases for additional functionalities.
- testfile.txt: A sample input file for testing.

### Prerequisites

To run the JUnit tests and measure code coverage with JaCoCo, we ensure the following are installed:
- Java Development Kit (JDK) 8 or higher
- Eclipse IDE for Java Developers
- Eclipse Plugin for JaCoCo

### Setup Instructions

Step 1: Import the Project into Eclipse
- Open Eclipse:
- launch Eclipse IDE.

Create a New Java Project:
- Go to File > New > Java Project.
- Name the project (e.g., PrintTokensProject).
- Click Finish.

Add the Project Files:
- Copy right_printtokens.java into the src folder of our project.
- Copy RightPrinttokensMainTest.java and RightPrinttokensNonMainTest.java into the src folder of our project.
- Copy testfile.txt to the root directory of our project.

Step 2: Configure JUnit in Eclipse
- Right-click on our project and select Properties.
- Go to Java Build Path > Libraries > Add Library.
- Select JUnit and then JUnit 5.
- Click Finish.

Step 3: Install JaCoCo Plugin
- Go to Help > Eclipse Marketplace.
- Search for JaCoCo and install the EclEmma Java Code Coverage plugin.
- Restart Eclipse if prompted.

Step 4: Running JUnit Tests with Code Coverage
- Right-click on our test files (RightPrinttokensMainTest.java or RightPrinttokensNonMainTest.java).
- Select Run As > JUnit Test.

Run with Code Coverage:
- Right-click on our test files or our project.
- Select Coverage As > JUnit Test.
- The JaCoCo plugin will execute the tests and display a coverage report.

Step 5: Viewing Code Coverage
- After running our tests with coverage, we can view the results in the Coverage view.
- Go to Window > Show View > Other > Coverage > Coverage.

### Creating the CFGs:

- We mapped out each method's control flow using a directed graph, where each node represents a significant step or decision point in the function, and each edge represents the transition between these steps based on certain conditions.
- Key decision points in the function, such as checks for spaces, new lines, special symbols, and end-of-file conditions, were identified and included in the CFG.
- We represented loops and branches explicitly to capture all possible paths the function could take during execution.
- For each method in right_printtokens.java, we created a Control Flow Graph (CFG), excluding code in catch clauses.
- Each node in the CFG represents a basic block, and edges represent the flow of control between these blocks.
- The CFGs provide a visual representation of the logical flow of the program, making it easier to identify all possible paths for thorough testing.

### Generating Test Paths:

- With the CFG in place, we identified distinct test paths that cover all the possible branches and loops within the function. Each path ensures that every conditional and decision point is tested.
- The goal was to achieve comprehensive coverage, ensuring that every aspect of the function’s behavior is validated.
- For unit testing, we selected test paths to achieve edge coverage for each method's CFG. This ensures that every possible edge in the CFG is executed at least once during testing.
- For program-level testing, we selected test paths to achieve edge coverage for the entire program, encompassing all CFG edges. This ensures comprehensive testing of the program's overall logic.

### Test Case Development:

We developed test cases based on the selected test paths. Each test case includes:
- The specific path to be tested.
- Corresponding test data needed to execute the path.
- The expected output to verify correct program behavior.
- Infeasible test paths (paths that cannot be executed due to logical constraints) were identified and replaced with alternative feasible paths to ensure complete edge coverage.

### Creating Test Cases:

- For each identified test path, we designed specific test cases with appropriate input data that would traverse that path. - This involved creating various BufferedReader inputs to simulate different scenarios the function might encounter.
- The expected output for each test case was determined based on the function’s logic and the specific path taken through the CFG.

### Documenting Test Paths and Expected Outputs:

- Each test case was documented in a table format, listing the test path, input data, and the expected output. This ensures clarity and ease of understanding when performing the tests

### JUnit Implementation:

- We implemented the developed test cases using JUnit, a popular framework for writing and running tests in Java.
- Each test case was executed to verify the program's functionality and detect any seeded faults. The focus was on ensuring that the program behaves as expected under various conditions and inputs.

### Coverage Reports:

- We generated separate coverage reports for unit tests (individual methods) and program-level tests (main method and overall program).
- The reports provide detailed information on the coverage achieved, highlighting areas that are well-tested and areas that may require additional tests.
- The coverage reports help in identifying uncovered code, allowing for targeted improvement of the test suite.

### Analyze the Source Code:

- We carefully examined each method in right_printtokens.java.
- Identified the basic blocks, which are sequences of consecutive statements with a single entry and exit point.

### Conclusion

By following these detailed instructions and understanding the implementation of each file, we can effectively test and analyze the PrintTokens program, ensuring thorough testing and comprehensive code coverage. This process ensures that the program is thoroughly tested, with detailed coverage reports providing insights into the quality and completeness of the tests.
