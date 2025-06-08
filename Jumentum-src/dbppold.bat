start "openocd" /d"K:\Program Files\OpenOCD\0.4.0\bin" openocd.exe -f "K:\Program Files\OpenOCD\0.4.0\bin\u.cfg"
start "insight" "K:\Program Files\CodeSourcery\Sourcery G++ Lite\bin\arm-none-eabi-gdb" --eval-command="file basic.elf" --eval-command="target remote :3333"
