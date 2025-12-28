#!/bin/bash

# Loop starting with 1024 (2^10) and multiplying by 2 until 2^20 is reached
for i in $(seq 10 14); do
  # Calculate 2 to the power of i
  l1_size=$((2**i))
    for j in $(seq 0 3); do
    assoc=$((2**j))
    # Print the current value to the console
    echo "Running with l1_size: $l1_size assoc: $assoc"
    
    # Run the command with the current value
    ./sim 16 $l1_size $assoc 0 0 0 0 streams_trace.txt
    done
  
done

