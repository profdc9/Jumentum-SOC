start "K:\openocd\openocd-0.4.0\src\openocd.exe" "K:\openocd\openocd-0.4.0\src\openocd.exe" -f "K:\openocd\openocd-0.4.0\src\u21.cfg"
start "insight" "K:\codesourcery\bin\arm-none-eabi-gdb" --eval-command="file basic.elf" --eval-command="target remote :3333"  --eval-command="mon reset halt"
