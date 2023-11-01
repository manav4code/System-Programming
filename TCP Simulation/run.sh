#!/bin/bash
make target

# Check if the number of iterations is provided as a command line argument
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <number_of_iterations>"
    exit 1
fi

# Store the number of iterations from the command line argument
n="$1"

# Loop n times and run gnome-terminal command within each iteration
for ((i=1; i<=n; i++)); do
    gnome-terminal 
done
