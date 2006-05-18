#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>

#ifdef USE_MEMORY_DEBUGGER
#include <malloc.h>
#include <assert.h>

static void* (*old_malloc_hook)(size_t, const void*);
static void* (*old_realloc_hook)(void*, size_t, const void*);
static void* (*old_memalign_hook)(size_t, size_t, const void*);
static void (*old_free_hook)(void*, const void*);

static uint32_t total_allocated = 0;

static void*
tests_malloc_hook(size_t sz, const void* caller)
{
  uint32_t* ret;
  
  __malloc_hook = old_malloc_hook;
  ret = (uint32_t*)malloc(sz + 12);
  old_malloc_hook = __malloc_hook;
  __malloc_hook = tests_malloc_hook;

  ret[0] = 0x12345678;
  ret[1] = sz;
  ((uint32_t*)(((char*)ret) + sz + 8))[0] = 0x9ABCDEF0;
  total_allocated += sz;

  memset(ret + 2, 0xFF, sz);

  return ret + 2;
}

static void*
tests_realloc_hook(void* ptr, size_t sz, const void* address)
{
  uint32_t* realdata = ((uint32_t*)ptr) - 2;
  void* newdata = malloc(realdata[1]);
  memcpy(newdata, realdata + 2, realdata[1]);
  free(realdata + 2);

  return newdata;
}

static void*
tests_memalign_hook(size_t al, size_t sz, const void* caller)
{
  printf("WARNING: memalign called, not properly implemented!\n");

  return NULL;
}

static void
tests_free_hook(void* ptr, const void* caller)
{
  uint32_t* realdata = ((uint32_t*)ptr) - 2;

  __free_hook = old_free_hook;
  assert(realdata[0] == 0x12345678);
  assert(((uint32_t*)(((char*)ptr) + realdata[1]))[0] == 0x9ABCDEF0);
  total_allocated -= realdata[1];

  memset(realdata, 0xFF, realdata[1] + 12);

  free(realdata);
  old_free_hook = __free_hook;
  __free_hook = tests_free_hook;
}

static void
tests_init_hook (void)
{
  old_malloc_hook = __malloc_hook;
  old_realloc_hook = __realloc_hook;
  old_memalign_hook = __memalign_hook;
  old_free_hook = __free_hook;

  __malloc_hook = tests_malloc_hook;
  __free_hook = tests_free_hook;
  __memalign_hook = tests_memalign_hook;
  __realloc_hook = tests_realloc_hook;
}

void (*__malloc_initialize_hook)(void) = tests_init_hook;

#endif

int main(int argc, char* argv[])
{
#ifdef SHOW_MEMORY_BREAK
  void* initialbreak = sbrk(0);
#endif

  // Get the top level suite from the registry
  CppUnit::Test *suite = CppUnit::TestFactoryRegistry::getRegistry().makeTest();

  // Adds the test to the list of test to run
  CppUnit::TextUi::TestRunner runner;
  runner.addTest( suite );

  // Change the default outputter to a compiler error format outputter
  runner.setOutputter( new CppUnit::CompilerOutputter( &runner.result(),
                                                       std::cerr ) );
  // Run the tests.
  bool wasSucessful = runner.run();

#ifdef SHOW_MEMORY_BREAK
  void* finalbreak = sbrk(0);
  
  uint32_t deltabreak = ((uint32_t)finalbreak) - ((uint32_t)initialbreak);
  printf("Change in memory break = %u kB\n", deltabreak / 1024);
#ifdef USE_MEMORY_DEBUGGER
  printf("Memory allocated at exit: %u\n", total_allocated);
#endif
#endif

  // Return error code 1 if the one of test failed.
  return wasSucessful ? 0 : 1;
}
