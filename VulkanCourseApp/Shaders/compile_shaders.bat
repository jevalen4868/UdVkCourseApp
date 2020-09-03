rem for /f %%f in ('dir /b .\*.vert') do "C:/VulkanSDK/1.2.141.2/Bin32/glslangValidator.exe" -V %%f
rem for /f %%f in ('dir /b .\*.frag') do "C:/VulkanSDK/1.2.141.2/Bin32/glslangValidator.exe" -V %%f

C:/VulkanSDK/1.2.141.2/Bin32/glslangValidator.exe -V shader.vert
C:/VulkanSDK/1.2.141.2/Bin32/glslangValidator.exe -V shader.frag
C:/VulkanSDK/1.2.141.2/Bin32/glslangValidator.exe -o second_vert.spv -V second.vert
C:/VulkanSDK/1.2.141.2/Bin32/glslangValidator.exe -o second_frag.spv -V second.frag

pause