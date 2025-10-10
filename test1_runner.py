####################################################################################################################################
# This file is used to quantify the uncertainty of testing something that the scheduler controls since it runs the test 1000 times #
# See chapter 3.2.1 in our report for details.                                                                                     #
####################################################################################################################################

import subprocess

# Configuration
n = 1000  # Number of test runs
binary = './test'  # Path to your test binary

success_count = 0
failure_count = 0

for _ in range(n):
    result = subprocess.run([binary], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    if result.returncode == 0:
        success_count += 1
    elif result.returncode == -6:  # Typical code for assertion failure (SIGABRT)
        failure_count += 1
    else:
        print(f"Run exited with unexpected code: {result.returncode}")

print(f"Successes: {success_count}")
print(f"Assertion failures: {failure_count}")
