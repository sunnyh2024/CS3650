# Write the assembly code for array_max


.global array_max

# Start of the code section
.text

array_max:
  # Function prologue
  enter $0, $0

  # use displacement to get next value in array 
  mov $0, %r12 # store memory counter in r12
  mov (%rsi, %r12, 8), %r13    # store max in r13
  add $1, %r12

load_values:
  movq (%rsi, %r12, 8), %r14
  cmp %rdi, %r12
  je end

  add $1, %r12
  cmp %r13, %r14
  jg update_max
  jmp load_values

update_max:
  mov %r14, %r13
  jmp load_values

end:
  mov %r13, %rax
  leave
  ret

.data
format:
  .asciz "%ld\n"
