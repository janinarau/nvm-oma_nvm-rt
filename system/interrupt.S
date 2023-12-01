.global arm_vector_table_el1

.balign 0x800

arm_vector_table_el1:

	//synchronous exception using sp0
	curr_el_sp0_sync:
		b __arm_lowlevel_trap_handler_sync

	//irq exception using sp0
	.balign 0x80
	curr_el_sp0_irq:
		b __arm_lowlevel_trap_handler_irq

	//fiq exception using sp0
	.balign 0x80
	curr_el_sp0_fiq:
		b __arm_lowlevel_trap_handler_fiq

	//system error exception using sp0
	.balign 0x80
	curr_el_sp0_serror:
		b __arm_lowlevel_trap_handler_error


	//synchronous exception using sp
	.balign 0x80
	curr_el_sp_sync:
		b __arm_lowlevel_trap_handler_sync

	//irq exception using sp
	.balign 0x80
	curr_el_sp_irq:
		b __arm_lowlevel_trap_handler_irq

	//fiq exception using sp
	.balign 0x80
	curr_el_sp_fiq:
		b __arm_lowlevel_trap_handler_fiq

	//system error exception using sp
	.balign 0x80
	curr_el_sp_serror:
		b __arm_lowlevel_trap_handler_error


	//synchronous exception from lower el aarc64
	.balign 0x80
	lower_el_64_sync:
		b __arm_lowlevel_trap_handler_sync

	//irq exception from lower el aarc64
	.balign 0x80
	lower_el_64_irq:
		b __arm_lowlevel_trap_handler_irq

	//fiq exception from lower el aarc64
	.balign 0x80
	lower_el_64_fiq:
		b __arm_lowlevel_trap_handler_fiq

	//system error exception from lower el aarc64
	.balign 0x80
	lower_el_64_serror:
		b __arm_lowlevel_trap_handler_error


	//synchronous exception from lower el aarc32
	.balign 0x80
	lower_el_32_sync:
		b __arm_lowlevel_trap_handler_sync

	//irq exception from lower el aarc32
	.balign 0x80
	lower_el_32_irq:
		b __arm_lowlevel_trap_handler_irq

	//fiq exception from lower el aarc32
	.balign 0x80
	lower_el_32_fiq:
		b __arm_lowlevel_trap_handler_fiq

	//system error exception from lower el aarc32
	.balign 0x80
	lower_el_32_serror:
		b __arm_lowlevel_trap_handler_error


.balign 0x80
__arm_lowlevel_trap_handler_sync:

	//Store frame pointer
	str x29,[sp,#-16]!
	mov x29,sp
	//Store critical registers
	stp x0,x1,[sp,#-16]!
	stp x2,x3,[sp,#-16]!
	stp x4,x5,[sp,#-16]!
	stp x6,x7,[sp,#-16]!
	stp x8,x9,[sp,#-16]!
	stp x10,x11,[sp,#-16]!
	stp x12,x13,[sp,#-16]!
	stp x14,x15,[sp,#-16]!
	stp x16,x17,[sp,#-16]!
	stp x18,x19,[sp,#-16]!
	stp x20,x21,[sp,#-16]!
	stp x22,x23,[sp,#-16]!
	stp x24,x25,[sp,#-16]!
	stp x26,x27,[sp,#-16]!
	stp x28,x30,[sp,#-16]!

	//Call highlevel handler
	//Pass base to saved registers as argument
	mov x0,sp
	bl handle_irq_lowlevel_sync

	//restore critical registers
	ldp x28,x30,[sp],#16
	ldp x26,x27,[sp],#16
	ldp x24,x25,[sp],#16
	ldp x22,x23,[sp],#16
	ldp x20,x21,[sp],#16
	ldp x18,x19,[sp],#16
	ldp x16,x17,[sp],#16
	ldp x14,x15,[sp],#16
	ldp x12,x13,[sp],#16
	ldp x10,x11,[sp],#16
	ldp x8,x9,[sp],#16
	ldp x6,x7,[sp],#16
	ldp x4,x5,[sp],#16
	ldp x2,x3,[sp],#16
	ldp x0,x1,[sp],#16
	//Restore frame pointer
	ldr x29,[sp],#16

	//Finish the exception handling
	eret

	.balign 0x80
__arm_lowlevel_trap_handler_irq:
	//Store frame pointer
	str x29,[sp,#-16]!
	mov x29,sp
	//Store critical registers
	stp x0,x1,[sp,#-16]!
	stp x2,x3,[sp,#-16]!
	stp x4,x5,[sp,#-16]!
	stp x6,x7,[sp,#-16]!
	stp x8,x9,[sp,#-16]!
	stp x10,x11,[sp,#-16]!
	stp x12,x13,[sp,#-16]!
	stp x14,x15,[sp,#-16]!
	stp x16,x17,[sp,#-16]!
	stp x18,x19,[sp,#-16]!
	stp x20,x21,[sp,#-16]!
	stp x22,x23,[sp,#-16]!
	stp x24,x25,[sp,#-16]!
	stp x26,x27,[sp,#-16]!
	stp x28,x30,[sp,#-16]!

	//Call highlevel handler
	//Pass base to saved registers as argument
	mov x0,sp
	bl handle_irq_lowlevel_irq

	//restore critical registers
	ldp x28,x30,[sp],#16
	ldp x26,x27,[sp],#16
	ldp x24,x25,[sp],#16
	ldp x22,x23,[sp],#16
	ldp x20,x21,[sp],#16
	ldp x18,x19,[sp],#16
	ldp x16,x17,[sp],#16
	ldp x14,x15,[sp],#16
	ldp x12,x13,[sp],#16
	ldp x10,x11,[sp],#16
	ldp x8,x9,[sp],#16
	ldp x6,x7,[sp],#16
	ldp x4,x5,[sp],#16
	ldp x2,x3,[sp],#16
	ldp x0,x1,[sp],#16
	//Restore frame pointer
	ldr x29,[sp],#16

	//Finish the exception handling
	eret

	.balign 0x80
__arm_lowlevel_trap_handler_fiq:
	//Store frame pointer
	str x29,[sp,#-16]!
	mov x29,sp
	//Store critical registers
	stp x0,x1,[sp,#-16]!
	stp x2,x3,[sp,#-16]!
	stp x4,x5,[sp,#-16]!
	stp x6,x7,[sp,#-16]!
	stp x8,x9,[sp,#-16]!
	stp x10,x11,[sp,#-16]!
	stp x12,x13,[sp,#-16]!
	stp x14,x15,[sp,#-16]!
	stp x16,x17,[sp,#-16]!
	stp x18,x19,[sp,#-16]!
	stp x20,x21,[sp,#-16]!
	stp x22,x23,[sp,#-16]!
	stp x24,x25,[sp,#-16]!
	stp x26,x27,[sp,#-16]!
	stp x28,x30,[sp,#-16]!

	//Call highlevel handler
	//Pass base to saved registers as argument
	mov x0,sp
	bl handle_irq_lowlevel_fiq

	//restore critical registers
	ldp x28,x30,[sp],#16
	ldp x26,x27,[sp],#16
	ldp x24,x25,[sp],#16
	ldp x22,x23,[sp],#16
	ldp x20,x21,[sp],#16
	ldp x18,x19,[sp],#16
	ldp x16,x17,[sp],#16
	ldp x14,x15,[sp],#16
	ldp x12,x13,[sp],#16
	ldp x10,x11,[sp],#16
	ldp x8,x9,[sp],#16
	ldp x6,x7,[sp],#16
	ldp x4,x5,[sp],#16
	ldp x2,x3,[sp],#16
	ldp x0,x1,[sp],#16
	//Restore frame pointer
	ldr x29,[sp],#16

	//Finish the exception handling
	eret

	.balign 0x80
__arm_lowlevel_trap_handler_error:
	//Store frame pointer
	str x29,[sp,#-16]!
	mov x29,sp
	//Store critical registers
	stp x0,x1,[sp,#-16]!
	stp x2,x3,[sp,#-16]!
	stp x4,x5,[sp,#-16]!
	stp x6,x7,[sp,#-16]!
	stp x8,x9,[sp,#-16]!
	stp x10,x11,[sp,#-16]!
	stp x12,x13,[sp,#-16]!
	stp x14,x15,[sp,#-16]!
	stp x16,x17,[sp,#-16]!
	stp x18,x29,[sp,#-16]!
	str x30,[sp,#-16]!

	//Call highlevel handler
	bl handle_irq_lowlevel_error

	//restore critical registers
	ldr x30,[sp], #16
	ldp x18,x29,[sp],#16
	ldp x16,x17,[sp],#16
	ldp x14,x15,[sp],#16
	ldp x12,x13,[sp],#16
	ldp x10,x11,[sp],#16
	ldp x8,x9,[sp],#16
	ldp x6,x7,[sp],#16
	ldp x4,x5,[sp],#16
	ldp x2,x3,[sp],#16
	ldp x0,x1,[sp],#16
	//Restore frame pointer
	ldr x29,[sp],#16

	//Finish the exception handling
	eret