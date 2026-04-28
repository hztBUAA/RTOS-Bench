/*
 * The local SDK headers stamp modules as SylixOS 3.2.8, while the validation
 * boards run SylixOS 3.9.0. SylixOS accepts modules whose version string is the
 * default "0.0.0", so export that value instead of the stale SDK version.
 *
 * Keep the symbol in a text section. Some 64-bit board loaders in this lab read
 * garbage for the version symbol when it is placed in the writable data segment.
 */
__attribute__((used, section(".text.__sylixos_version")))
const char __sylixos_version[] = "0.0.0";
