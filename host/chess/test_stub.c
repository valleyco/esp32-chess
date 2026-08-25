#include "../test_assert.h"

/* Placeholder until Step 4 (chess_api red tests). */
static void test_host_harness_runs(void)
{
    ASSERT_EQ_INT(1, 1);
}

int main(void)
{
    test_host_harness_runs();
    return test_report();
}
