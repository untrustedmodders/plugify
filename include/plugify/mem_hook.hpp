#pragma once

#include <plugify/mem_protector.hpp>

namespace plugify {
	/**
	 * @brief Hooks a method in a virtual table.
	 *
	 * This function replaces a method in a virtual table with a hook function and returns the original method pointer.
	 *
	 * @tparam F The type of the function pointer.
	 * @param vtp Pointer to the virtual table.
	 * @param func Pointer to the function to replace the original.
	 * @param offset The offset of the method in the virtual table.
	 * @return F The original method pointer.
	 */
	template<typename F> requires(std::is_pointer_v<F> && std::is_function_v<std::remove_pointer_t<F>>)
	F HookMethod(void* vtp, F func, int offset) {
		const uintptr_t vtable = *reinterpret_cast<uintptr_t*>(vtp);
		const uintptr_t entry = vtable + (static_cast<size_t>(offset) * sizeof(void*));
		const uintptr_t orig = *reinterpret_cast<uintptr_t*>(entry);
		MemProtector memProtector(entry, sizeof(entry), ProtFlag::RWX);
		*reinterpret_cast<uintptr_t*>(entry) = reinterpret_cast<uintptr_t>(func);
		return reinterpret_cast<F>(orig);
	}

	/**
	 * @brief Retrieves the index of a virtual function in a virtual table.
	 *
	 * This function determines the index of a virtual function pointer within a virtual table.
	 * The implementation is compiler-specific and may vary for GCC, Clang, and MSVC.
	 *
	 * @tparam F The type of the function pointer.
	 * @param func The virtual function pointer whose index is to be determined.
	 * @param flag The memory protection flag to use (default: ProtFlag::RWX).
	 * @return int The index of the virtual function in the virtual table, or -1 if the index could not be determined.
	 */
	template<typename F>
	int GetVirtualTableIndex(F func, ProtFlag flag = ProtFlag::RWX) {
		void* ptr = (void*&)func;

		constexpr size_t size = 12;

		MemProtector protector(ptr, size, flag);

#if defined(__GNUC__) || defined(__clang__)
		struct GCC_MemFunPtr {
			union {
				void* adrr;			// always even
				intptr_t vti_plus1; // vindex+1, always odd
			};
			intptr_t delta;
		};

		int vtindex;
		auto mfp_detail = (GCC_MemFunPtr*)&ptr;
		if (mfp_detail->vti_plus1 & 1) {
			vtindex = (mfp_detail->vti_plus1 - 1) / sizeof(void*);
		} else {
			vtindex = -1;
		}

		return vtindex;
#elif defined(_MSC_VER)
		// https://www.unknowncheats.me/forum/c-and-c-/102577-vtable-index-pure-virtual-function.html

		// Check whether it's a virtual function call on x86

		// They look like this:a
		//		0:  8b 01				   mov	eax,DWORD PTR [ecx]
		//		2:  ff 60 04				jmp	DWORD PTR [eax+0x4]
		// ==OR==
		//		0:  8b 01				   mov	eax,DWORD PTR [ecx]
		//		2:  ff a0 18 03 00 00	   jmp	DWORD PTR [eax+0x318]]

		// However, for vararg functions, they look like this:
		//		0:  8b 44 24 04			 mov	eax,DWORD PTR [esp+0x4]
		//		4:  8b 00				   mov	eax,DWORD PTR [eax]
		//		6:  ff 60 08				jmp	DWORD PTR [eax+0x8]
		// ==OR==
		//		0:  8b 44 24 04			 mov	eax,DWORD PTR [esp+0x4]
		//		4:  8b 00				   mov	eax,DWORD PTR [eax]
		//		6:  ff a0 18 03 00 00	   jmp	DWORD PTR [eax+0x318]
		// With varargs, the this pointer is passed as if it was the first argument

		// On x64
		//		0:  48 8b 01				mov	rax,QWORD PTR [rcx]
		//		3:  ff 60 04				jmp	QWORD PTR [rax+0x4]
		// ==OR==
		//		0:  48 8b 01				mov	rax,QWORD PTR [rcx]
		//		3:  ff a0 18 03 00 00	   jmp	QWORD PTR [rax+0x318]
		auto finder = [&](uint8_t* addr) {
			std::unique_ptr<MemProtector> protector;

			if (*addr == 0xE9) {
				// May or may not be!
				// Check where it'd jump
				addr += 5 /*size of the instruction*/ + *(uint32_t*)(addr + 1);

				protector = std::make_unique<MemProtector>(addr, size, flag);
			}

			bool ok = false;
#if INTPTR_MAX == INT64_MAX
			if (addr[0] == 0x48 && addr[1] == 0x8B && addr[2] == 0x01) {
				addr += 3;
				ok = true;
			} else
#endif
			if (addr[0] == 0x8B && addr[1] == 0x01) {
				addr += 2;
				ok = true;
			} else if (addr[0] == 0x8B && addr[1] == 0x44 && addr[2] == 0x24 && addr[3] == 0x04 && addr[4] == 0x8B && addr[5] == 0x00) {
				addr += 6;
				ok = true;
			}

			if (!ok)
				return -1;

			constexpr int PtrSize = static_cast<int>(sizeof(void*));

			if (*addr++ == 0xFF) {
				if (*addr == 0x60)
					return *++addr / PtrSize;
				else if (*addr == 0xA0)
					return int(*((uint32_t*)++addr)) / PtrSize;
				else if (*addr == 0x20)
					return 0;
				else
					return -1;
			}

			return -1;
		};

		return finder((uint8_t*)ptr);
#else
#error "Compiler not support"
#endif
	}
} // namespace plugify
