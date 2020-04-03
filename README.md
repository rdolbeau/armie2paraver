Arm Instruction Emulator traces to Paraver traces translator.

# What, How, ...

This is a prototype translator to convert a trace generated by the [Arm Instruction Emulator](https://developer.arm.com/tools-and-software/server-and-hpc/compile/arm-instruction-emulator) into a trace suitable for use with the [Paraver tool](https://tools.bsc.es/paraver/).

Using ArmIE version 20.0, you need to generate the trace with the `libmeminstrace_emulated.so` tool, such as this:

```
armie -msve-vector-bits=256 -i libmeminstrace_emulated.so -- ./myprogram
```
As of version 20.0, ArmIE doesn't include the memory information for the SVE load/store, hopefully this will be improved in an upcoming version.

Then you can convert the trace with e.g.:

```
./armietraceparser meminstrace.myprogram.12345.0000.log myprogram_paraver
```

which will produce the paraver trace in myprogram_paraver.prv, myprogram_paraver.pcf


Plaese not that you will need a recent LLVM installation to compile this, at this code uses LLVM's MCDisassembler internally to decode the instructions.


# Acknowledgements

This work has partly been done as part of the European Processor Initiative project.

The European Processor Initiative (EPI) (FPA: 800928) has received funding from the European Union's Horizon 2020 research and innovation programme under grant agreement EPI-SGA1: 826647
