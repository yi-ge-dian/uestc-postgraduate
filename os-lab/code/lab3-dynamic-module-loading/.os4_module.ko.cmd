cmd_/home/wenlongdong/桌面/os-lab/lab3-dynamic-module-loading/os4_module.ko := ld -r -m elf_x86_64 -z max-page-size=0x200000 -T ./scripts/module-common.lds --build-id  -o /home/wenlongdong/桌面/os-lab/lab3-dynamic-module-loading/os4_module.ko /home/wenlongdong/桌面/os-lab/lab3-dynamic-module-loading/os4_module.o /home/wenlongdong/桌面/os-lab/lab3-dynamic-module-loading/os4_module.mod.o ;  true