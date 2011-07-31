IF (BUILD_TESTING)
  INCLUDE_DIRECTORIES(extdep/cppunit/include)

  ADD_LIBRARY(cppunit-cellml STATIC
    extdep/cppunit/src/cppunit/AdditionalMessage.cpp
    extdep/cppunit/src/cppunit/Asserter.cpp
    extdep/cppunit/src/cppunit/BriefTestProgressListener.cpp
    extdep/cppunit/src/cppunit/CompilerOutputter.cpp
    extdep/cppunit/src/cppunit/DefaultProtector.cpp
    extdep/cppunit/src/cppunit/Exception.cpp
    extdep/cppunit/src/cppunit/Message.cpp
    extdep/cppunit/src/cppunit/ProtectorChain.cpp
    extdep/cppunit/src/cppunit/Protector.cpp
    extdep/cppunit/src/cppunit/RepeatedTest.cpp
    extdep/cppunit/src/cppunit/SourceLine.cpp
    extdep/cppunit/src/cppunit/StringTools.cpp
    extdep/cppunit/src/cppunit/SynchronizedObject.cpp
    extdep/cppunit/src/cppunit/TestAssert.cpp
    extdep/cppunit/src/cppunit/TestCase.cpp
    extdep/cppunit/src/cppunit/TestCaseDecorator.cpp
    extdep/cppunit/src/cppunit/TestComposite.cpp
    extdep/cppunit/src/cppunit/Test.cpp
    extdep/cppunit/src/cppunit/TestDecorator.cpp
    extdep/cppunit/src/cppunit/TestFactoryRegistry.cpp
    extdep/cppunit/src/cppunit/TestFailure.cpp
    extdep/cppunit/src/cppunit/TestLeaf.cpp
    extdep/cppunit/src/cppunit/TestNamer.cpp
    extdep/cppunit/src/cppunit/TestPath.cpp
    extdep/cppunit/src/cppunit/TestPlugInDefaultImpl.cpp
    extdep/cppunit/src/cppunit/TestResultCollector.cpp
    extdep/cppunit/src/cppunit/TestResult.cpp
    extdep/cppunit/src/cppunit/TestRunner.cpp
    extdep/cppunit/src/cppunit/TestSetUp.cpp
    extdep/cppunit/src/cppunit/TestSuccessListener.cpp
    extdep/cppunit/src/cppunit/TestSuiteBuilderContext.cpp
    extdep/cppunit/src/cppunit/TestSuite.cpp
    extdep/cppunit/src/cppunit/TextOutputter.cpp
    extdep/cppunit/src/cppunit/TextTestProgressListener.cpp
    extdep/cppunit/src/cppunit/TextTestResult.cpp
    extdep/cppunit/src/cppunit/TextTestRunner.cpp
    extdep/cppunit/src/cppunit/TypeInfoHelper.cpp)

  ADD_EXECUTABLE(RunTestBin tests/TestOutput.cpp ${CPPUNIT_SOURCES})
  TARGET_LINK_LIBRARIES(RunTestBin cppunit-cellml ${CMAKE_DL_LIBS} ${TEST_LIBS})
  ADD_TEST(RunTestBin ${BASH} RunTestBin)
ENDIF()
