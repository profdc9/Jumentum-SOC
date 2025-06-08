start "openocd" /d"K:\Program Files\Codesourcery" openocd-pp.exe -f "K:\Program Files\Codesourcery\u.cfg"
start "insight" "K:\Program Files\CodeSourcery\Sourcery G++ Lite\bin\arm-none-eabi-gdb" --eval-command="file basic.elf" --eval-command="target remote :3333"
