for /f %%f in ('dir /b .\*.vert') do "C:/VulkanSDK/1.2.141.2/Bin32/glslangValidator.exe" -V %%f
for /f %%f in ('dir /b .\*.frag') do "C:/VulkanSDK/1.2.141.2/Bin32/glslangValidator.exe" -V %%f
pause