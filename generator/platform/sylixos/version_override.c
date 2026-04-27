/*
 * The local SDK headers stamp modules as SylixOS 3.2.8, while the validation
 * boards run SylixOS 3.9.0. SylixOS accepts SO modules whose version string is
 * the default "0.0.0", so export that value instead of the stale SDK version.
 *
 * Keep the object in an executable/read-only section. Some 64-bit board loaders
 * in this lab read garbage for the version symbol when it is placed in the RW
 * data segment, while text-segment symbols are resolved consistently.
 */
__attribute__((used, section(".text.__sylixos_version")))
const char __sylixos_version[] = "0.0.0";
