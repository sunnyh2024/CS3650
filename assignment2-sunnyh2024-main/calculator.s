#
# Usage: ./calculator <op> <arg1> <arg2>
#

# Make `main` accessible outside of this module
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
  movq 24(%rsi), %r14 # arg2 = argv[3]


  # Hint: Convert 1st operand to long int
  mov %r13, %rdi
  call atol
  mov %rax, %r13
  # Hint: Convert 2nd operand to long int
  mov %r14, %rdi
  call atol
  mov %rax, %r14

  # Hint: Copy the first char of op into an 8-bit register
  # i.e., op_char = op[0] - something like mov 0(%r12), ???
  mov %r12, %rdi 

  # if (op_char == '+') {
  #   ...
  # }
  # else if (op_char == '-') {
  #  ...
  # }
  # ...
  # else {
  #   // print error
  #   // return 1 from main
  # }
  mov $plus, %rsi
  call strcmp
  cmp $0, %rax
  je add

  mov $minus, %rsi
  call strcmp
  cmp $0, %rax
  je subtract

  mov $times, %rsi
  call strcmp
  cmp $0, %rax
  je multiply

  mov $divid, %rsi
  call strcmp
  cmp $0, %rax
  je divide

  jmp error

add:
  add %r13, %r14
  mov %r14, %rsi
  jmp print_result

subtract:
  sub %r14, %r13
  mov %r13, %rsi
  jmp print_result

multiply:
  imul %r13, %r14
  mov %r14, %rsi
  jmp print_result

divide:
  cmp $0, %r14
  je div_error
  mov $0, %rdx
  mov %r13, %rax
  cqo
  idiv %r14
  mov %rax, %rsi
  jmp print_result

error:
  mov $error_message, %rdi
  mov $0, %al
  call printf
  mov $1, %rax
  leave
  ret 

div_error:
  mov $div_error_message, %rdi
  mov $0, %al
  call printf
  mov $1, %rax
  leave
  ret

# assumes the output is in %rsi. Adds formatting and calls printf
print_result:
  mov $format, %rdi
  mov $0, %al
  call printf
  leave
  ret

# Start of the data section
.data

format: 
  .asciz "%ld\n"

error_message: 
  .asciz "Unknown operation\n"

div_error_message:
  .asciz "Cannot divide by 0\n"

plus:
  .asciz "+"
minus:
  .asciz "-"
times:
  .asciz "*"
divid:
  .asciz "/"
