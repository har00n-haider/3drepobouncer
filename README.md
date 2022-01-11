# License deactivation issue with cryptolens via C++ client

## The scenarios

We have a core C++ dll (3drepobouncer) that we call from two locations:

- An native C++ program (3drepobouncerClient.exe)
- A managed C# program (csharp_example.exe)

The licensing is implementd in the core C++ dll

## Issue

The problem is that the first scenario activates and deactivates fine, but the second scenario only activates, but does not seem to deactivate properly. I cannot tell why there is such a difference, seeing as they are both calling the same dll, with the same configuration.


## Replicating the scenario

1. Open the following files and update the paths at the top of each file to match the local setup:

bouncer_client.cmd
bouncer_build.cmd
csharp_example.cmd
csharp_example_build.cmd

2. Run bouncer_build.cmd & csharp_example_build.cmd to build the executables

3. Run bouncer_client.cmd it will activate/deactive properly

4. Run csharp_example.cmd it will activate fine but will not deactive properly

