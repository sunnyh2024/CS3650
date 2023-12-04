# Write the assembly code for main
.global main

# Start of the code section
.text

# int main(int argc, char argv[][])
main:
  # Function prologue
  enter $0, $0

  # Variable mappings:
  # op -> %r12
  # arg1 -> %r13
  # arg2 -> %r14
  movq 8(%rsi), %r12  # op = argv[1]
  movq 16(%rsi), %r13 # arg1 = argv[2]

  mov %r12, %rdi
  call atol
  mov %rax, %r12

  mov %r13, %rdi
  call atol
  mov %rax, %r13

  cmp %r12, %r13
  jl greater
  cmp %r12, %r13
  je equal
  cmp %r12, %r13
  jg less

less:
  mov $less_than, %rdi
  mov $0, %al
  call printf
  leave
  ret

equal:
  mov $equal_to, %rdi
  mov $0, %al
  call printf
  leave
  ret

greater:
  mov $greater_than, %rdi
  mov $0, %al
  call printf
  leave
  ret

.data

less_than:
  .asciz "less\n"
equal_to:
  .asciz "equal\n"
greater_than:
  .asciz "greater\n"
