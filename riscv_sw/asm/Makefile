.RECIPEPREFIX = >

.PHONY: all clean

all:
> riscv64-unknown-elf-as -o main.o main.S
> riscv64-unknown-elf-ld -Map=main.map -T riscv.ld -o main.elf main.o
> riscv64-unknown-elf-objcopy -O binary ./main.elf ./main.bin
clean:
> rm *.o *.map main.elf main.bin
