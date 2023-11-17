
// https://wiki.osdev.org/Calling_Global_Constructors

# .section .init
# 	// gcc will insert crtend.o's .init section here
# 	popl %ebp
# 	ret
# 
# .section .fini
# 	// gcc will insert crtend.o's .fini section here
# 	popl %ebp
# 	ret
