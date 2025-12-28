#!/bin/bash

# Loop starting with 1024 (2^10) and multiplying by 2 until 2^20 is reached
for i in $(seq 0 4); do
  # Calculate 2 to the power of i
  #value=$((2**i))
  
  # Use arithmetic expansion to perform integer division
  #assoc=$(($value/32))
  
  # Print the current value to the console
  echo "Running ./sim value: $i"
  
  # Run the command with the current value
  ./sim 16 1024 1 0 0 $i 4 streams_trace.txt | grep "L1 miss rate"
done
