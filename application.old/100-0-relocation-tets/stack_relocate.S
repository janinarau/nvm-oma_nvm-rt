//The stack pointer must be aligned on 16 bytes
#define RELOCATION_STEP 0x20

.global relocate_stack
.global __current_stack_base_ptr
.global __virtual_stack_begin
.global __virtual_stack_end

.data
__current_stack_base_ptr: 
    .long 0x0
    .long 0x0
__virtual_stack_begin: 
    .long 0x0
    .long 0x0
__virtual_stack_end: 
    .long 0x0
    .long 0x0

.text
/**
 *   Care about all registers:
 *   x0-x7 are passed arguments, does not apply here
 *   x9-x15 are caller-saved, just use them here
 *   x19-x29 are callee-saved, don't touch them
 *   x29 is the frame-pointer, relocate it by hand
 *   x30 is the link register, don't relocate it, cause it's code
 */
relocate_stack:
    //Collect region informatiom
    adr x15, __virtual_stack_end
    ldr x9,[x15]
    adr x15, __virtual_stack_begin
    ldr x10,[x15]
    //The current base_ptr
    adr x11, __current_stack_base_ptr
    ldr x12, [x11]

    //Copy target
    mov x13,sp

    //Increase the stack pointer
    sub sp,sp, #RELOCATION_STEP
    mov x15,sp

    //Start the copy of the stack
begin_loop:
    ldr x14,[x13]
    b relocate_ptr
loop_continue:
    str x14,[x13,#-RELOCATION_STEP]
    add x13,x13,#0x8
    cmp x13,x12
    blt begin_loop

    sub x29,x29,#RELOCATION_STEP
    sub x12,x12,#RELOCATION_STEP
    str x12,[x11]

    ret

relocate_ptr:
    cmp x14,x12
    bge loop_continue
    cmp x14,x15
    ble loop_continue
    sub x14,x14,#RELOCATION_STEP
    b loop_continue