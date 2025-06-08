start "K:\openocd\openocd-0.4.0\src\openocd.exe" "K:\openocd\openocd-0.4.0\src\openocd.exe" -f "K:\openocd\openocd-0.4.0\src\u.cfg"
start "insight" "K:\Program Files\CodeSourcery\Sourcery G++ Lite\bin\arm-none-eabi-gdb" --eval-command="file basic.elf" --eval-command="target remote :3333"  --eval-command="mon reset halt"
