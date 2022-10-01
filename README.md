# Still Water

The challenge binary is a Vulkan program that takes in a command line argument (the "flag") and displays a mesh grid being pertubed by water waves. The way the waves are generated depends on the argument passed in, and the mesh goes still when the correct flag is supplied.

## Control Flow Description

The Vulkan program, when executed, loads two shaders to be executed on the Vulkan instance. This is typically a GPU, but it is possble to run this challege binary using a CPU Vulkan instance (i.e. a "software renderer"). The binary has been tested in a VM with a 

The mesh grid is dynamically generated (i.e. not from a model file) in the main Vulkan program and passed into the shaders via a vertex buffer.

At program start, the supplied command line argument is transformed (TBC how this is done).

At every frame (iterations of the main loop in the Vulkan program), vertex buffer is sent to the GPU together with push constants containing the transformed command line argument.

The vertex shader perturbs the vertices according to some wave algorithm (TBC, probably Gerstner). The coefficients of the wave function will be some fixed values defined in the shader, from which the push constant inputs are subtracted.

If the correct flag is specified in the command line argument, the transformed values passed into the shader via the push constant subtract the coefficients exactly, causing the wavefunction to go to zero.

## Exploitation Steps

The participant is expected to identify from the binary that Vulkan is being used to display the image on the screen. This may be easily identified from the many `vkXXX` calls made at various points.

## Flag

`STF22{k33P-{<@>5T%ill??><@<-Th3rE!--<>}`
