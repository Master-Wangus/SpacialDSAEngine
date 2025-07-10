## Projects
1. Project 1 demonstrates collision tests between AABBs, Bounding Spheres, Rays and Planes.
2. Project 2 demonstrates bounding volumes construction and fustrum intersection/culling.
3. Project 3 demonstrates fast Bounding Volume Hiereachy construction from top down to bottom up with AABB, Bounding Spheres and OBBs.
4. Project 4 demonstrates fast Kd trees and Octrees. (WIP)

## Framework Additions
On top of the sample framework provided by Digipen, I have added:
1. An ECS architecture built with EnTT, a must-have for any modern rendering engines.
2. An event system to enable clean communication between systems.
3. A picking system for models to demonstrate model transforms as well as QOL.
4. An interface system built with ImGui, no need to touch code to see any outputs for debugging.
5. A simple resource system to hold references to all resources in use.
6. Pre-compiled headers for a faster startup. Also I like having to see my std includes in one place. 
7. Enforced coding conventions, code in every project has a part to play and no stray code or includes. 
   Header files should only show functions to be used and all implementation is done in .cpps. (except for a spare few).
8. A detailed and frequent commit history.

## 📦 Instructions provided by sample framework

Before building the sample-project complete the following installations:

1. Install $\text{CMake}$ for Windows
   1. $\text{CMake}$ is a configuration tool that simplifies the setting up of a programming environment for writing OpenGL-based applications using an integrated programming environment such as Microsoft's Visual Studio 2022.

   2. Begin by downloading the latest version of $\text{CMake}$ Windows x64 Installer file [with extension $\color{red}\text{msi}$] from [here](https://cmake.org/download/).

   3. Run the downloaded installer. It is important that you add $\text{CMake}$ to the system $\text{PATH}$. For this, choose the second option when the installer runs - `Add CMake to the system PATH for all users`. Use default settings for all other installation instructions.

2. Install $\text{GIT}$ for Windows

   1. $\text{GIT}$ is a version control system that we use to provide a consistent set of files including appropriate versions of libraries and appropriate versions of starter code for programming assignments. That is, $\text{GIT}$ allows your application programs to remain compatible with updated libraries and starter code.
   2. Begin by downloading the $64-$bit version of $\text{GIT}$ from [here](https://git-scm.com/download/win).
   3. Run the downloaded installer and follow the default installation instructions.

3. Install Microsoft's $\text{Visual Studio 2022}$ using the instructions listed [here](https://learn.microsoft.com/en-us/visualstudio/install/install-visual-studio?view=vs-2022). (If you already have skipped it)

4. Install $\text{Doxygen}$ for Windows

   1. $\text{Doxygen}$ is an automated documentation formatter and generator for source files implemented in C, C++, and a variety of other programming languages. It allows you to specially tag comments in your source files that will be used to generate nicely formatted HTML and Latex output. Use the handout provided in $\text{CSD1170/CSD1171}$ to re-introduce yourself to $\text{Doxygen}$.

5. Download and install the latest $64-$bit version of $\text{Doxygen}$ from [here](https://www.doxygen.nl/download.html).
--------
## 🛠️ Build Steps

You can build the project in both **Debug** and **Release** configurations using the steps below:

### 🔧 1. Open Command Prompt

Open a terminal window (cmd or PowerShell) and navigate to the root folder of the project.

### 📁 2. Create Build Directory

Check if the `build/` directory exists. If not, create it:

```bash
mkdir build
```

### ⚙️ 3. Configure with CMake

Inside the `build/` directory, run CMake to generate project files:

```bash
cd build
cmake ..
```

### 🧪 4. Build Debug Version

To build the project in Debug mode:

```bash
cmake --build . --config Debug
```

After building the project in **Debug mode**, the executable will be generated under the `Debug` subdirectory inside your `build` folder. To launch the executable from the command line:

```bash
Debug\project-1.exe
```

### 🚀 5. Build Release Version

To build the project in Release mode:

```bash
cmake --build . --config Release
```

The release executable will be generated under `Release` folder under `build` folder. From the `build` folder path in the command line run to launch the executable:

After building the project in **Release mode**, the executable will be generated under the `Release` subdirectory inside your `build` folder. To launch the executable from the command line:

```bash
Release\Project-1.exe
```

### 🧪 6. Use Visual Studio

The $\text{CMake}$ build command generate a $\text{Visual Studio 2022}$ solution within the $\color{blue}\text{build}$ directory. Go to this directory, it contains $\color{purple}\text{csd-3151.sln}$, double click solution file to open it in the $\text{Visual Studio 2022}$ IDE.

- Go to `Build` `->` `Build Solution`
- Next, in the Solution Explorer $\text{Right Click}$ `project-1` it will display a context menu, select option $\text{Set as Startup Project}$
- Click `Start Without Debugging` (`Ctrl+F5`) or `Start` (`F5`) to run it.

### 📂 7. Output

The compiled executables will be located inside the `build/` directory.

---

## 📝 Notes

- Repeat the `cmake ..` command if you make changes to the `CMakeLists.txt` file.
- These steps mirror what `run.bat` automates. You can optionally run `run.bat` to automate all of the above in one go.
- Similarly, before submitting your code run `clean.bat` in order to clean up the project.
