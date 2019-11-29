**How to Test CLOOK IO Scheduler**

Running the Read and Write Tester Program

In this folder, run the following command:

make

This will add a new executible named clooktest. Run it three times then run
the following command (assuming root access is enabled):

dmesg

This will show the instructions (read or write) and sectors
of the IO requests added and dispatched by the CLOOK IO scheduler.
