# AppCodeTemp
Template to create FreeRTOS tasks using the team's middleware

-------------------------------------------------------------------

## How to use this repo:

1. Clone it `git clone git@github.com:RobomasterOwltech/AppCodeTemp.git`
2. Go into the newly generated directory `cd AppCodeTemp`
1. Download the submodules `git submodule init && git submodule update`
1. In case any of the dependencies needs to be updated, simply do: `git submodule update --remote`

## Having problems with git?:

* Make sure that `postStartCommand` value is set at the `.devcontainer/devcontainer.json`.
* Make sure to have created and register ssh keys for Github, as done in [DeployTools](https://github.com/RobomasterOwltech/deployTools)

## How to change DevBoard

1. At the moment you clone the repo, modify the branch for the `robotConfig` repo.
        1. This will Change the defines on the `Makefile`
1. Change the OwlDefines variables at `.vscode/c_cpp_properties.json`

## Create the container

1. Download docker:  
        - For Linux: https://docs.docker.com/engine/install/  
        - For Windows: https://docs.docker.com/desktop/setup/install/windows-install/  
        - For MacOS: https://docs.docker.com/desktop/setup/install/mac-install/  
2. From the project root folder (AppCodeTemp) go to ".devcontainer": `cd .devcontainer`
3. Build the image: `docker build .`
4. List the images:  `docker images`
        - Identify the ID of the image just built
5. Go to the project root folder (AppCodeTemp): `cd ..`
6. Run the container in detach mode:  `docker run -d --mount type=bind,source=$(realpath ./),target=/home/myProjects <YOUR_IMAGE_ID>` # Source is the absoluthe path to AppCodeTemp

> All compilation steps need to be done form inside the container. To enter it:
> 1. List containers: `docker ps`
>         - Identify the ID of the container
> 2. Enter the container: `docker exec -it <CONTAINER_ID> bash`

## Compile:

1. From the project root folder (AppCodeTemp) call make 

        make all
    1. You might need to specify the compiler path, in which case use:
        
            make GCC_PATH=/path/2/compiler
    
    1. Alternatively, you can add it to your path, or set it as an environment variable. 
1. If your build failed, then you can clean the `build` folder
        
        make clean
1. If you are encountering problems, and would like to know what is being configured by the makefiles from other projects, try:

        make showRobotConfig showRtosConfig

### Add a file to be compiled 

1. Save your changes to the file.
1. Copy it's relative path.
1. If it is a C, add it in the `C_SOURCES` variable from the `Makefile` 
1. In case your file is referencing any library, make sure it is present in the `C_INCLUDES`

arm-none-eabi-gcc -c -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard -DUSE_HAL_DRIVER -DSTM32F407xx -IrobotConfig/inc -Ieigen/Eigen -IDrivers/STM32F4xx_HAL_Driver/Inc -IDrivers/STM32F4xx_HAL_Driver/Inc/Legacy -IDrivers/CMSIS/Device/ST/STM32F4xx/Include -IDrivers/CMSIS/Include -IMiddlewares/Third_Party/FreeRTOS/Source/include -IMiddlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -IMiddlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -IMiddlewares/ST/stm32_mw_usb_device/Class/CDC/Inc -IMiddlewares/ST/stm32_mw_usb_device/Core/Inc -IowltechMiddleware/inc -Og -Wall -fdata-sections -ffunction-sections -g -gdwarf-2 -MMD -MP -MF"build/main.d" -Wa,-a,-ad,-alms=build/main.lst robotConfig/src/main.cpp -o build/main.o

## NOTES:

### C/C++ standard
If you were to change the C/C++ standard used, remember to change it in the `IntelliSense`, `makefile`, and `c_cpp_properties.json`

For more [formatting options](https://clang.llvm.org/docs/ClangFormatStyleOptions.html) see the link.

## Resources

If you would like to learn more about the used tools and information, then check:

* [Quick intro to Makefiles](https://www.youtube.com/watch?v=aw9wHbFTnAQ) (Video)
* [GNU Make manual](https://www.gnu.org/savannah-checkouts/gnu/make/manual/make.html#Reading)
* The GNU Make [Book](https://learning.oreilly.com/library/view/the-gnu-make/9781457189883/). John Graham-Cumming. No Starch Press
* [Example on using Eigen](https://youtu.be/RgKreY1HK_8?si=aPSwUOyWYGdj3Nyl&t=284)
