.global relocate_stack
.global __current_stack_base_ptr
.global __virtual_stack_begin
.global __virtual_stack_end
.globl __debug_data

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
relocate_stack: //x9,x11
    //Collect region informatiom
    adr x15, __virtual_stack_begin
    ldr x10,[x15]
    //The current base_ptr
    adr x11, __current_stack_base_ptr
    ldr x12, [x11]

    //This is used to compare if a relocation is needed
    add x11,x10,#RELOCATION_STEP

    //Copy target
    mov x13,sp

    //Increase the stack pointer
    sub sp,sp, #RELOCATION_STEP

    //Set X9 to the real shadow base
    sub x9,x10,#REAL_STACK_SIZE

    //Start the copy of the stack
begin_loop:
    ldr x14,[x13]
#ifdef IN_MEMORY_POINTER_RELOCATION
    b relocate_ptr
#endif
loop_continue:
    str x14,[x13,#-RELOCATION_STEP]
    add x13,x13,#0x8
    cmp x13,x12
    blt begin_loop

    /**
    *   Maybe current unsaved registers (x0-x7 and x19-x29) have to be relocated, too
    */
#ifdef IN_MEMORY_POINTER_RELOCATION
    b relocate_registers
continue_registers:
#else
    sub x29,x29,#RELOCATION_STEP
#endif

    sub x12,x12,#RELOCATION_STEP


    //Check if the new base will reach the shadow area
    cmp x12,x10
    bge finish
    add x12,x12,#REAL_STACK_SIZE
    add sp,sp,#REAL_STACK_SIZE

    //cmp x29,x10
    //bge finish
    //add x29,x29,#REAL_STACK_SIZE

finish:
    adr x11, __current_stack_base_ptr
    str x12,[x11]
    ret

relocate_ptr:
    cmp x14,x12
    bge loop_continue
    cmp x14,x9
    ble loop_continue
    sub x14,x14,#RELOCATION_STEP

    //Check if the new base will reach the shadow area
    cmp x12,x11
    bge loop_continue
    //Perform a relocation if possivle
    cmp x14,x10 // TODO(maybe only relocate if really lesss)
    bge loop_continue
    add x14,x14,#REAL_STACK_SIZE
    b loop_continue

#ifdef IN_MEMORY_POINTER_RELOCATION

.macro relocate_register_code register, continue

    cmp \register,x12
    bge \continue
    cmp \register, x9
    ble \continue
    sub \register,\register,#RELOCATION_STEP

    //Check if the new base will reach the shadow area
    cmp x12,x11
    bge \continue
    //Perform a relocation if possivle
    cmp \register,x10 // TODO(maybe only relocate if really lesss)
    bge \continue
    add \register,\register,#REAL_STACK_SIZE

.endm

relocate_registers:
    relocate_register_code x0,c_x1
c_x1:
    relocate_register_code x1,c_x2
c_x2:
    relocate_register_code x2,c_x3
c_x3:
    relocate_register_code x3,c_x4
c_x4:
    relocate_register_code x4,c_x5
c_x5:
    relocate_register_code x5,c_x6
c_x6:
    relocate_register_code x6,c_x7
c_x7:
    relocate_register_code x7,c_x19
c_x19:
    relocate_register_code x19,c_x20
c_x20:
    relocate_register_code x20,c_x21
c_x21:
    relocate_register_code x21,c_x22
c_x22:
    relocate_register_code x22,c_x23
c_x23:
    relocate_register_code x23,c_x24
c_x24:
    relocate_register_code x24,c_x25
c_x25:
    relocate_register_code x25,c_x26
c_x26:
    relocate_register_code x26,c_x27
c_x27:
    relocate_register_code x27,c_x28
c_x28:
    relocate_register_code x28,c_x29
c_x29:
    relocate_register_code x29,continue_registers
b continue_registers

#endif