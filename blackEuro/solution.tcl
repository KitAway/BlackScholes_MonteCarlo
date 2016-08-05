# SDAccel creates a solution of the application per invocation of the tool
# Set the name of the solution to be used throughut the current script
set solution "solution"

# This script will copy the results of the experiment to a user defined Location
set solution_results "results"
# Create a solution
create_solution -name $solution -force

# Define the target devices for the current solution
add_device -vbnv xilinx:adm-pcie-7v3:1ddr:2.1

# Host Source Files
add_files "../common/main.cpp"
# Host Compiler Flags
set_property -name host_cflags -value "-g -Wall" -objects [current_solution]


# Define a kernel to be compiled by SDAccel
# The kernel definition name must match the kernel name in the source code
# The kernel type is given by the source code type for the kernel
create_kernel -type c blackEuro

# Adding files for a kernel works in the same way as adding files for host code.
# User must associate source files to specific kernels using the -kernel option to the add_files command
add_files -kernel [get_kernels blackEuro] "blackEuro.cpp"
add_files -kernel [get_kernels blackEuro] "blackScholes.cpp"
add_files -kernel [get_kernels blackEuro] "../common/RNG.cpp"
add_files -kernel [get_kernels blackEuro] "../common/stockData.cpp"

# Create a binary container. Every SDAccel application has at least 1 binary container to hold the FPGA binary.
create_opencl_binary blackScholes
# Depending on configuration, a target device may have 1 or more areas reserved for kernels compiled by SDAccel
# The user must tell SDAccel which area to target on the device. This sets the compilation parameters for the kernel.
set_property region OCL_REGION_0 [get_opencl_binary blackScholes]
# Kernels are compiled into compute units. There is at least 1 compute unit per kernel in an FPGA binary.
create_compute_unit -opencl_binary [get_opencl_binary blackScholes] -kernel [get_kernels blackEuro] -name K1

# Compile the design for CPU based emulation
# Currently it does not work due to a bug in SDAccel.
#compile_emulation -flow cpu -opencl_binary [get_opencl_binary blackScholes]

# Run the design in CPU emulation mode
# Currently it does not work due to a bug in SDAccel.
#run_emulation -flow cpu -args "-a blackScholes.xclbin -n blackEuro -s 100 -k 110 -r 0.05 -v 0.2 -t 1 -c 6.04 -p 10.65"

# Compute the resource estimate for the application
report_estimate

# Compile the design for RTL simulation
compile_emulation -flow hardware -opencl_binary [get_opencl_binary blackScholes]

# Run the RTL simulation of the application
run_emulation -flow hardware -args "-a blackScholes.xclbin -n blackEuro -s 100 -k 110 -r 0.05 -v 0.2 -t 1 -c 6.04 -p 10.65"

#Compile the application to run on an FPGA
#build_system

#Package_system
#package_system

# Run the application in hardware
#run_system -args "-a blackScholes.xclbin -n blackEuro -s 100 -k 110 -r 0.05 -v 0.2 -t 1 -c 6.04 -p 10.65"



