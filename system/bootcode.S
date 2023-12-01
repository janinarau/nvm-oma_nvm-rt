.global system_entry

system_entry:
    ldr x0, =arm_vector_table_el1
	msr vbar_el1, x0

    ldr x0, =__bootstack_top
    mov sp, x0
    bl init_system_c