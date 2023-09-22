> Hello!
This is anonymous repo for paper submission


# Preparing development environment
Follow below steps to prepare your environment by downloading or running scripts

### 1. Download and install the latest version of Windows 11 on a physical machine (~40min).
**NOTE: Use a new machine rather than regular machine for daily operations**
### 2. Enable test mode for faster/easier driver loading then reboot the machine (~2min).
```
bcdedit -set TESTSIGNING ON && bcdedit /set testsigning on && bcdedit /set nointegritychecks on && echo "Done! Check above messages" && pause
```
Make sure there is no error for above command. If there is, try yo resolve it.
### 3. Download and install Visual Studio Enterprise 2022 (~30min).
### 4. Select "Desktop Development with C++" and install it as your main workload (~30min).
### 5. Make sure you have the following components after VS installation:
*Use VS installer to install the following components*
 > * C# and Visual Basic Roslyn compilers
 > * MSBuild
 > * C# and Visual Basic
 > * NuGet package manager
 > * Developer Analytics tools
 > * Snapshot Debugger
 > * Python language support
 > * Python 3 64-bit (3.9.13) (out of support)
 > * MSVC v143 - VS 2022 C++ x64/x86 build tool...
 > * C++ ATL for latest v143 build tools (x86 & x64)
 > * C++ MFC for latest v143 build tools (x86 & x...
 > * C++ Clang Compiler for Windows (16.0.5)
 > * Windows 11 SDK (10.0.22621.382)
 > * USB Device Connectivity
 > * Intel Hardware Accelerated Execution Manag...
 > * NuGet targets and build tasks
 > * Help Viewer
 > * Git for Windows
 > * C++ ATL for latest v143 build tools with Spect...
 > * C++ MFC for latest v143 build tools with Spe...
 > * MSVC v143 - VS 2022 C++ x64/x86 Spectre-...
 > * C++ v14.37 (17.7) ATL for v143 build tools (x8...
 > * C++ v14.37 (17.7) ATL for v143 build tools wit...
 > * C++ v14.37 (17.7) MFC for v143 build tools (x...
 > * C++ v14.37 (17.7) MFC for v143 build tools wi...
 > * Follow instructions and [Install Windows 11, version 22H2 WDK](https://learn.microsoft.com/en-us/windows-hardware/drivers/download-the-wdk)
  *You may need to match the version of SDK and WDK. Both should be in same version*

# Pulling the code
You may pull pcosfe branch for getting a copy of source code.
```
  git clone -b pcosfe --single-branch https://github.com/osfeanonpc/pcosfe.git
  cd pcosfe
```

# Building
## Follow below steps for building the project
### 1. Select solution "OSFE.sln" and set building target to Debug | x64. All sub-repo requires the same setting.
### 2. Click on solution and select Build. You may see a success message.
### 3. If you get the following error:
2 > SIGNTASK : SignTool error : No certificates were found that met all the given criteria.
###   Then, You may need to generate a certificate on your own machine, thus:
#### 1. Choose OSFEDrv sub-repo.
#### 2. Go to "Driver signing"
#### 3. select "Test certificate"
#### 4. Select "create test certificate"
#### 5. Re-build the solution.
#### 6. Done.

Your output will be in ${SolutionDir}/Programs/OSFEDrv.sys

# Running the driver
You may use the following instructions to load the driver for capturing the traces:
## 1. Installing the driver
1. Copy OSFEDrv.sys into C:\Windows\System32\drivers\OSFEDrv.sys
2. Install a service for running this driver via cmd.exe (with admin right)
```
sc create OSFEDrv type=kernel start=demand error=ignore binpath=system32\DRIVERS\OSFEDrv.sys group=None tag=yes displayname=OSFEDrv
```
3. Run the following command to check the status of driver:
```
sc query OSFEDrv
```
## 2. Running the driver
4. Run the following command to capture the events:
```
sc start OSFEDrv
```
6. For unstalling the driver, run the following command:
```
sc delete OSFEDrv
```
7. Done.
**You cannot stop the driver since tear-down manager is exlcluded from this project.**
